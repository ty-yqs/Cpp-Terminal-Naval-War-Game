#include "ship.h"

Ship::Ship(int row, int col, std::string glyph, int hp)
    : Entity(row, col, glyph), hp_(hp), maxHp_(hp) {}

void Ship::takeDamage(int dmg) {
    hp_ -= dmg;
    if (hp_ <= 0) {
        kill();
    }
}

std::vector<std::unique_ptr<Projectile>> Ship::collectNewProjectiles() {
    std::vector<std::unique_ptr<Projectile>> temp;
    temp.swap(newProjectiles_);
    return temp;
}

void Ship::spawnProjectile(std::unique_ptr<Projectile> p) {
    newProjectiles_.push_back(std::move(p));
}
