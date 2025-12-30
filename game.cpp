#include "game.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <ctime>

Game::Game() : renderer_(World::kRows, World::kCols) {
    std::srand(std::time(nullptr));
    player_ = std::make_unique<PlayerShip>(World::kRows - 2, World::kCols / 2);
    startLevel(1);
}

void Game::startLevel(int newLevel) {
    level_ = newLevel;
    spawnTimer_ = 0;

    // Reset the arena state while keeping player's progress (HP/coins/ammo).
    enemies_.clear();
    projectiles_.clear();
    pickups_.clear();
    world_ = World();

    // Reposition player to a safe, familiar spawn point.
    player_->setPos(World::kRows - 2, World::kCols / 2);
}

void Game::runLoop() {
    running_ = true;
    while (running_) {
        InputState inputState = input_.poll();
        handleInput(inputState);
        update();
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 FPS
    }
}

void Game::handleInput(const InputState &input) {
    if (input.quit) {
        running_ = false;
        return;
    }
    
    player_->handleInput(input, world_);
}

void Game::update() {
    spawnTimer_++;
    spawnEnemies();
    spawnPickups();
    
    player_->update();
    
    for (auto& e : enemies_) {
        e->aiUpdate(player_->getRow(), player_->getCol(), World::kRows, World::kCols);
        e->update();
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
        running_ = false;
        return;
    }

    // Level progression: reach the coin target to advance/win.
    if (level_ == 1 && player_->getCoins() >= kLevel1WinCoins) {
        startLevel(2);
    } else if (level_ == 2 && player_->getCoins() >= kLevel2WinCoins) {
        running_ = false;
        // Should show win screen
    }
}

void Game::spawnEnemies() {
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
                enemies_.push_back(std::make_unique<EnemyShip>(0, 0, EnemyType::BOMBER));
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
                enemies_.push_back(std::make_unique<EnemyShip>(0, col, EnemyType::BOMBER));
            }
        }
    }
}

void Game::spawnPickups() {
    const int pickupChance = (level_ == 1) ? 200 : 350;
    if (rand() % pickupChance == 0) {
        int r = rand() % World::kRows;
        int c = rand() % World::kCols;
        if (rand() % 2 == 0)
            pickups_.push_back(std::make_unique<Pickup>(r, c, PickupType::WEAPON));
        else
            pickups_.push_back(std::make_unique<Pickup>(r, c, PickupType::MEDICAL));
    }
}

void Game::collectProjectiles() {
    auto newP = player_->collectNewProjectiles();
    for (auto& p : newP) projectiles_.push_back(std::move(p));
    
    for (auto& e : enemies_) {
        auto newE = e->collectNewProjectiles();
        for (auto& p : newE) projectiles_.push_back(std::move(p));
    }
}

void Game::checkCollisions() {
    // Projectile vs Ships
    for (auto& p : projectiles_) {
        if (p->isDead()) continue;
        
        // Vs Player
        if (p->getRow() == player_->getRow() && p->getCol() == player_->getCol()) {
            player_->takeDamage(p->getDamage());
            p->kill();
            continue;
        }
        
        // Vs Enemies
        for (auto& e : enemies_) {
            if (e->isDead()) continue;
            if (p->getRow() == e->getRow() && p->getCol() == e->getCol()) {
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
