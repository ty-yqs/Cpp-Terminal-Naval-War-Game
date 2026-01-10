#include "game.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sstream>

Game::Game(std::string mapFilePath)
    : renderer_(World::kRows, World::kCols), mapFilePath_(std::move(mapFilePath)) {
    std::srand(std::time(nullptr));
    // Player ship glyph is 3 rows tall; keep it fully in-bounds.
    player_ = std::make_unique<PlayerShip>(World::kRows - 3, World::kCols / 2);
    world_ = mapFilePath_.empty() ? World() : World(mapFilePath_);
    startLevel(1);
}

void Game::startLevel(int newLevel) {
    level_ = newLevel;
    spawnTimer_ = 0;

    // After clearing Level 1, reset coins for Level 2.
    if (level_ == 2) {
        player_->resetCoins();
    }

    // Reset the arena state while keeping player's progress (HP/coins/ammo).
    enemies_.clear();
    projectiles_.clear();
    pickups_.clear();
    if (mapFilePath_.empty()) {
        // Random map: regenerate per level.
        world_ = World();
    } else {
        // File map: keep the same map across levels.
        // (It is already loaded during Game construction / level 1 start.)
        if (level_ == 1) {
            world_ = World(mapFilePath_);
        }
    }

    // Reposition player to a safe, familiar spawn point.
    player_->setPos(World::kRows - 3, World::kCols / 2);

    // Level 1 starts with 4 Bombers placed randomly at distinct free positions.
    if (level_ == 1) {
        constexpr int kInitialBombers = 4;

        std::vector<std::vector<bool>> occupied(World::kRows, std::vector<bool>(World::kCols, false));
        auto canPlaceBomber = [&](int r, int c) {
            // Bomber glyph is 3 rows tall and ~2 cols wide.
            for (int dr = 0; dr < 3; ++dr) {
                for (int dc = 0; dc < 2; ++dc) {
                    int rr = r + dr;
                    int cc = c + dc;
                    if (!world_.inBounds(rr, cc)) return false;
                    if (world_.isBlocked(rr, cc)) return false;
                    if (occupied[rr][cc]) return false;
                }
            }
            return true;
        };
        auto markBomber = [&](int r, int c) {
            for (int dr = 0; dr < 3; ++dr) {
                for (int dc = 0; dc < 2; ++dc) {
                    occupied[r + dr][c + dc] = true;
                }
            }
        };

        int placed = 0;
        const int maxAttempts = 5000;
        for (int attempt = 0; attempt < maxAttempts && placed < kInitialBombers; ++attempt) {
            int r = std::rand() % (World::kRows - 2); // 0..kRows-3
            int c = 1 + (std::rand() % (World::kCols - 3)); // 1..kCols-3 (avoid instant wall hit)

            if (!canPlaceBomber(r, c)) continue;

            enemies_.push_back(std::make_unique<EnemyShip>(r, c, EnemyType::BOMBER));
            markBomber(r, c);
            placed++;
        }
    }
}

void Game::runLoop() {
    running_ = true;
    while (running_) {
        InputState inputState = input_.poll();
        
        if (inputState.quit) {
            running_ = false;
            continue;
        }
        
        if (state_ == GameState::MENU) {
            handleMenuInput(inputState);
            renderMenu();
        } else if (state_ == GameState::PLAYING) {
            handleInput(inputState);
            update();
            render();
        } else if (state_ == GameState::GAME_OVER || state_ == GameState::WIN) {
            renderer_.clear();
            if (state_ == GameState::GAME_OVER) {
                renderer_.printAt(World::kRows / 2, World::kCols / 2 - 10, "GAME OVER");
            } else {
                renderer_.printAt(World::kRows / 2, World::kCols / 2 - 10, "YOU WIN!");
            }
            renderer_.printAt(World::kRows / 2 + 2, World::kCols / 2 - 15, "Press Q to quit");
            renderer_.present();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
    }
}

void Game::handleInput(const InputState &input) {
    player_->handleInput(input, world_);
}

void Game::handleMenuInput(const InputState &input) {
    if (input.up && menuSelection_ > 0) {
        menuSelection_--;
    }
    if (input.down && menuSelection_ < 1) {
        menuSelection_++;
    }
    if (input.fireShell) {  // 使用空格键确认
        if (menuSelection_ == 0) {
            // 开始游戏
            state_ = GameState::PLAYING;
            startLevel(1);
        } else if (menuSelection_ == 1) {
            // 退出游戏
            running_ = false;
        }
    }
}

void Game::renderMenu() {
    renderer_.clear();
    
    // Draw title
    int titleRow = World::kRows / 4;
    renderer_.printAt(titleRow, World::kCols / 2 - 10, "===================");
    renderer_.printAt(titleRow + 1, World::kCols / 2 - 10, "   SPACE SHOOTER   ");
    renderer_.printAt(titleRow + 2, World::kCols / 2 - 10, "===================");
    
    // Draw menu options
    int menuRow = World::kRows / 2;
    std::string startText = menuSelection_ == 0 ? "> START GAME <" : "  START GAME  ";
    std::string quitText = menuSelection_ == 1 ? ">  QUIT GAME <" : "   QUIT GAME  ";
    
    renderer_.printAt(menuRow, World::kCols / 2 - 7, startText);
    renderer_.printAt(menuRow + 2, World::kCols / 2 - 7, quitText);
    
    // Draw instructions
    renderer_.printAt(World::kRows - 3, World::kCols / 2 - 20, "Use Arrow Keys to select, SPACE to confirm");
    
    renderer_.present();
}

void Game::update() {
    auto canPlaceGlyphAt = [&](int baseRow, int baseCol, const std::string& glyph) -> bool {
        std::stringstream ss(glyph);
        std::string line;
        int dr = 0;
        while (std::getline(ss, line)) {
            for (int dc = 0; dc < (int)line.size(); ++dc) {
                if (line[dc] == ' ') continue;
                const int rr = baseRow + dr;
                const int cc = baseCol + dc;
                if (!world_.inBounds(rr, cc)) return false;
                if (world_.isBlocked(rr, cc)) return false;
            }
            ++dr;
        }
        return true;
    };

    spawnTimer_++;
    spawnEnemies();
    spawnPickups();
    
    player_->update();
    
    for (auto& e : enemies_) {
        const int prevRow = e->getRow();
        const int prevCol = e->getCol();

        e->aiUpdate(player_->getRow(), player_->getCol(), World::kRows, World::kCols);
        e->update();

        // Entities (except Bomber) cannot move through obstacles.
        if (e->getEnemyType() != EnemyType::BOMBER) {
            if (!canPlaceGlyphAt(e->getRow(), e->getCol(), e->getGlyph())) {
                e->setPos(prevRow, prevCol);
            }
        }
    }
    
    for (auto& p : projectiles_) {
        p->update();
        if (!world_.inBounds(p->getRow(), p->getCol()) || world_.isBlocked(p->getRow(), p->getCol())) {
            p->kill();
        }
    }
    
    collectProjectiles();
    checkCollisions();
    
    // Cleanup dead entities
    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), 
        [](const auto& e) { return e->isDead(); }), enemies_.end());
        
    projectiles_.erase(std::remove_if(projectiles_.begin(), projectiles_.end(), 
        [](const auto& p) { return p->isDead(); }), projectiles_.end());
        
    pickups_.erase(std::remove_if(pickups_.begin(), pickups_.end(), 
        [](const auto& p) { return p->isDead(); }), pickups_.end());

    if (player_->getHp() <= 0) {
        state_ = GameState::GAME_OVER;
        return;
    }

    // Level progression: reach the coin target to advance/win.
    if (level_ == 1 && player_->getCoins() >= kLevel1WinCoins) {
        startLevel(2);
    } else if (level_ == 2 && player_->getCoins() >= kLevel2WinCoins) {
        state_ = GameState::WIN;
        // Should show win screen
    }
}

void Game::spawnEnemies() {
    auto trySpawnBomberAtEdge = [&]() {
        std::vector<std::vector<bool>> occupied(World::kRows, std::vector<bool>(World::kCols, false));

        auto markGlyphArea = [&](int baseRow, int baseCol, const std::string& glyph) {
            std::stringstream ss(glyph);
            std::string line;
            std::vector<std::string> lines;
            int maxWidth = 0;
            while (std::getline(ss, line)) {
                if ((int)line.size() > maxWidth) maxWidth = (int)line.size();
                lines.push_back(line);
            }

            int h = (int)lines.size();
            if (h <= 0) h = 1;
            if (maxWidth <= 0) maxWidth = 1;

            for (int dr = 0; dr < h; ++dr) {
                for (int dc = 0; dc < maxWidth; ++dc) {
                    int rr = baseRow + dr;
                    int cc = baseCol + dc;
                    if (!world_.inBounds(rr, cc)) continue;
                    occupied[rr][cc] = true;
                }
            }
        };

        // Mark existing entities so we don't spawn Bomber overlapping them.
        markGlyphArea(player_->getRow(), player_->getCol(), player_->getGlyph());
        for (const auto& e : enemies_) {
            if (e->isDead()) continue;
            markGlyphArea(e->getRow(), e->getCol(), e->getGlyph());
        }

        // Bomber is 3 rows tall and 2 cols wide.
        auto canPlaceBomber = [&](int r, int c) {
            for (int dr = 0; dr < 3; ++dr) {
                for (int dc = 0; dc < 2; ++dc) {
                    int rr = r + dr;
                    int cc = c + dc;
                    if (!world_.inBounds(rr, cc)) return false;
                    if (world_.isBlocked(rr, cc)) return false;
                    if (occupied[rr][cc]) return false;
                }
            }
            return true;
        };

        const int maxAttempts = 200;
        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            int r = std::rand() % (World::kRows - 2); // 0..kRows-3
            bool left = (std::rand() % 2 == 0);
            int c = left ? 0 : (World::kCols - 2);
            int dir = left ? 1 : -1; // Fly into the arena

            if (!canPlaceBomber(r, c)) continue;
            enemies_.push_back(std::make_unique<EnemyShip>(r, c, EnemyType::BOMBER, dir));
            return;
        }
    };

    // Extra Bomber spawns (post-start): higher frequency than before.
    const int bomberInterval = (level_ == 1) ? 120 : 80;
    if (spawnTimer_ % bomberInterval == 0) {
        trySpawnBomberAtEdge();
    }

    const int spawnInterval = (level_ == 1) ? 50 : 30;
    if (spawnTimer_ % spawnInterval != 0) return;

    const int spawnCount = (level_ == 1) ? 1 : 2;
    for (int i = 0; i < spawnCount; ++i) {
        int r = rand() % 100;
        int col = rand() % World::kCols;

        if (level_ == 1) {
            if (r < 50) {
                enemies_.push_back(std::make_unique<EnemyShip>(0, col, EnemyType::GUNBOAT));
            } else if (r < 80) {
                enemies_.push_back(std::make_unique<EnemyShip>(0, col, EnemyType::DESTROYER));
            } else if (r < 95) {
                enemies_.push_back(std::make_unique<EnemyShip>(rand() % World::kRows, 0, EnemyType::CRUISER));
            } else {
                trySpawnBomberAtEdge();
            }
        } else {
            // Level 2 is harder: more enemies, more tough types.
            if (r < 20) {
                enemies_.push_back(std::make_unique<EnemyShip>(0, col, EnemyType::GUNBOAT));
            } else if (r < 65) {
                enemies_.push_back(std::make_unique<EnemyShip>(0, col, EnemyType::DESTROYER));
            } else if (r < 95) {
                int side = (rand() % 2 == 0) ? 0 : (World::kCols - 1);
                enemies_.push_back(std::make_unique<EnemyShip>(rand() % World::kRows, side, EnemyType::CRUISER));
            } else {
                trySpawnBomberAtEdge();
            }
        }
    }
}

void Game::spawnPickups() {
    const int pickupChance = (level_ == 1) ? 200 : 350;
    if (rand() % pickupChance == 0) {
        int r = rand() % World::kRows;
        int c = rand() % World::kCols;
        if (world_.isBlocked(r, c)) return;
        if (rand() % 2 == 0)
            pickups_.push_back(std::make_unique<Pickup>(r, c, PickupType::WEAPON));
        else
            pickups_.push_back(std::make_unique<Pickup>(r, c, PickupType::MEDICAL));
    }
}

void Game::collectProjectiles() {
    auto newP = player_->collectNewProjectiles();
    for (auto& p : newP) {
        if (world_.isBlocked(p->getRow(), p->getCol())) p->kill();
        projectiles_.push_back(std::move(p));
    }
    
    for (auto& e : enemies_) {
        auto newE = e->collectNewProjectiles();
        for (auto& p : newE) {
            if (world_.isBlocked(p->getRow(), p->getCol())) p->kill();
            projectiles_.push_back(std::move(p));
        }
    }
}

void Game::checkCollisions() {
    auto hitsEntityGlyph = [&](const Entity& entity, int projRow, int projCol) -> bool {
        const int baseRow = entity.getRow();
        const int baseCol = entity.getCol();
        const std::string& glyph = entity.getGlyph();

        std::stringstream ss(glyph);
        std::string line;
        int dr = 0;
        while (std::getline(ss, line)) {
            for (int dc = 0; dc < (int)line.size(); ++dc) {
                if (line[dc] == ' ') continue;
                if (baseRow + dr == projRow && baseCol + dc == projCol) return true;
            }
            ++dr;
        }
        return false;
    };

    // Projectile vs Ships
    for (auto& p : projectiles_) {
        if (p->isDead()) continue;
        
        // Vs Player
        if (hitsEntityGlyph(*player_, p->getRow(), p->getCol())) {
            player_->takeDamage(p->getDamage());
            p->kill();
            continue;
        }
        
        // Vs Enemies
        for (auto& e : enemies_) {
            if (e->isDead()) continue;
            if (hitsEntityGlyph(*e, p->getRow(), p->getCol())) {
                if (e->getEnemyType() != EnemyType::BOMBER) { // Bomber invincible
                    e->takeDamage(p->getDamage());
                    if (e->isDead()) {
                        player_->addCoins(e->getScoreValue());
                    }
                }
                p->kill();
                break; 
            }
        }
    }
    
    // Player vs Pickups (Range +/- 1)
    for (auto& pu : pickups_) {
        if (pu->isDead()) continue;
        if (abs(player_->getRow() - pu->getRow()) <= 1 && abs(player_->getCol() - pu->getCol()) <= 1) {
            if (pu->getType() == PickupType::WEAPON) {
                player_->addAmmo(20, 1);
            } else {
                player_->heal(100);
            }
            pu->kill();
        }
    }
}

void Game::render() {
    renderer_.clear();
    renderer_.drawBorders();
    renderer_.drawWorld(world_);
    
    renderer_.drawEntity(*player_);
    for (const auto& e : enemies_) renderer_.drawEntity(*e);
    for (const auto& p : projectiles_) renderer_.drawEntity(*p);
    for (const auto& pu : pickups_) renderer_.drawEntity(*pu);
    
    std::string hud = "Lv:" + std::to_string(level_) +
                      " HP:" + std::to_string(player_->getHp()) + 
                      " Coins:" + std::to_string(player_->getCoins()) + 
                      " Shells:" + std::to_string(player_->getShells()) +
                      " Missiles:" + std::to_string(player_->getMissiles());
    renderer_.drawHud(hud);
    
    renderer_.present();
}
