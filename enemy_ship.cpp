#include "enemy_ship.h"
#include <cstdlib>

EnemyShip::EnemyShip(int row, int col, EnemyType type, int bomberDir)
    : Ship(row, col, "?", 1), type_(type) {
    
    switch (type) {
        case EnemyType::GUNBOAT:
            glyph_ = "o";
            hp_ = 1;
            maxHp_ = 1;
            moveInterval_ = 8;
            fireInterval_ = 20;
            shells_ = 10;
            setColor(2); // Yellow
            break;
        case EnemyType::DESTROYER:
            glyph_ = "O";
            hp_ = 10;
            maxHp_ = 10;
            moveInterval_ = 12;
            fireInterval_ = 30;
            shells_ = 10;
            torpedoes_ = 2;
            setColor(3); // Red
            break;
        case EnemyType::CRUISER:
            glyph_ = "<>"; // <> is two chars, use C for now or special handling in renderer
            hp_ = 100;
            maxHp_ = 100;
            moveInterval_ = 15;
            fireInterval_ = 40;
            shells_ = 30;
            missiles_ = 2;
            setColor(4); // Magenta
            break;
        case EnemyType::BOMBER:
            bomberDir_ = (bomberDir >= 0) ? 1 : -1;
            glyph_ = (bomberDir_ > 0) ? "\\\n==\n/" : "/\n==\n\\"; // Facing based on direction
            hp_ = 9999; // Cannot be damaged
            maxHp_ = 9999;
            moveInterval_ = 3;
            fireInterval_ = 25;
            shells_ = 3; // "has 3 bullets"
            torpedoes_ = 999; // "Or drop a torpedo"
            setColor(5); // Cyan
            break;
    }
}

void EnemyShip::update() {
    // Basic update if needed
}

int EnemyShip::getScoreValue() const {
    switch (type_) {
        case EnemyType::DESTROYER: return 1;
        case EnemyType::CRUISER: return 10;
        default: return 0;
    }
}

void EnemyShip::aiUpdate(int playerRow, int playerCol, int maxRows, int maxCols) {
    if (isDead()) return;
    moveTimer_++;
    fireTimer_++;
    
    // Movement
    if (moveTimer_ >= moveInterval_) {
        moveTimer_ = 0;
        
        if (type_ == EnemyType::BOMBER) {
            // Fly horizontally
            int nextCol = col_ + bomberDir_;
            // Only disappear when attempting to leave bounds (allow edge positions).
            if (nextCol < 0 || nextCol > maxCols - 2) {
                kill();
                return;
            }

            col_ = nextCol;

            // Update glyph
            if (bomberDir_ > 0) glyph_ = "\\\n==\n/";
            else glyph_ = "/\n==\n\\";

        } else if (type_ == EnemyType::CRUISER) {
            // Only move horizontally
            int dCol = (rand() % 3) - 1;
            int nextCol = col_ + dCol;
            if (nextCol >= 0 && nextCol < maxCols) col_ = nextCol;
        } else {
            // Move 4 directions randomly or towards player
            int dRow = (rand() % 3) - 1;
            int dCol = (rand() % 3) - 1;
            
            // Simple AI: 50% chance to move towards player
            if (rand() % 2 == 0) {
                if (row_ < playerRow) dRow = 1;
                else if (row_ > playerRow) dRow = -1;
                
                if (col_ < playerCol) dCol = 1;
                else if (col_ > playerCol) dCol = -1;
            }
            
            int nextRow = row_ + dRow;
            int nextCol = col_ + dCol;
            if (nextRow >= 0 && nextRow < maxRows && nextCol >= 0 && nextCol < maxCols) {
                row_ = nextRow;
                col_ = nextCol;
            }
        }
    }
    
    // Firing
    if (fireTimer_ >= fireInterval_) {
        fireTimer_ = 0;
        
        // Calculate direction to player
        int dr = 0, dc = 0;
        if (row_ < playerRow) dr = 1; else if (row_ > playerRow) dr = -1;
        if (col_ < playerCol) dc = 1; else if (col_ > playerCol) dc = -1;
        
        if (type_ == EnemyType::GUNBOAT) {
            if (shells_ > 0) {
                spawnProjectile(std::make_unique<Projectile>(row_ + dr, col_ + dc, dr, dc, ProjectileType::SHELL));
                shells_--;
            }
        } else if (type_ == EnemyType::DESTROYER) {
            if (shells_ > 0) {
                spawnProjectile(std::make_unique<Projectile>(row_ + dr, col_ + dc, dr, dc, ProjectileType::SHELL));
                shells_--;
            }
            if (torpedoes_ > 0) {
                // Torpedo left/right
                spawnProjectile(std::make_unique<Projectile>(row_, col_ - 1, 0, -1, ProjectileType::TORPEDO));
                spawnProjectile(std::make_unique<Projectile>(row_, col_ + 1, 0, 1, ProjectileType::TORPEDO));
                torpedoes_--;
            }
        } else if (type_ == EnemyType::CRUISER) {
            if (shells_ >= 4) {
                // Shells 4 directions
                spawnProjectile(std::make_unique<Projectile>(row_ - 1, col_, -1, 0, ProjectileType::SHELL));
                spawnProjectile(std::make_unique<Projectile>(row_ + 1, col_, 1, 0, ProjectileType::SHELL));
                spawnProjectile(std::make_unique<Projectile>(row_, col_ - 1, 0, -1, ProjectileType::SHELL));
                spawnProjectile(std::make_unique<Projectile>(row_, col_ + 1, 0, 1, ProjectileType::SHELL));
                shells_ -= 4;
            }
            
            if (missiles_ > 0) {
                // Missile
                auto m = std::make_unique<Projectile>(row_ + 1, col_, 1, 0, ProjectileType::MISSILE);
                m->setTarget(playerRow, playerCol);
                spawnProjectile(std::move(m));
                missiles_--;
            }
        } else if (type_ == EnemyType::BOMBER) {
            // Drop torpedo or 3 bullets
            if (rand() % 2 == 0) {
                spawnProjectile(std::make_unique<Projectile>(row_ + 1, col_, 1, 0, ProjectileType::TORPEDO));
            } else {
                if (shells_ >= 3) {
                    spawnProjectile(std::make_unique<Projectile>(row_ + 1, col_, 1, 0, ProjectileType::SHELL));
                    spawnProjectile(std::make_unique<Projectile>(row_ + 1, col_ - 1, 1, -1, ProjectileType::SHELL));
                    spawnProjectile(std::make_unique<Projectile>(row_ + 1, col_ + 1, 1, 1, ProjectileType::SHELL));
                    shells_ -= 3;
                }
            }
        }
    }
}
