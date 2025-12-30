#include "projectile.h"
#include <cstdlib> // abs

Projectile::Projectile(int row, int col, int dRow, int dCol, ProjectileType type)
    : Entity(row, col, " "), dRow_(dRow), dCol_(dCol), type_(type) {
    
    setColor(6); // White
    switch (type) {
        case ProjectileType::SHELL:
            glyph_ = (dCol == 0) ? "|" : "-";
            break;
        case ProjectileType::TORPEDO:
            glyph_ = "=";
            break;
        case ProjectileType::MISSILE:
            glyph_ = "*";
            break;
    }
}

void Projectile::setTarget(int tRow, int tCol) {
    tracking_ = true;
    targetRow_ = tRow;
    targetCol_ = tCol;
}

void Projectile::update() {
    lifeTime_++;
    
    if (type_ == ProjectileType::MISSILE && tracking_) {
        // 简单的追踪逻辑：每步向目标靠近
        if (row_ < targetRow_) dRow_ = 1;
        else if (row_ > targetRow_) dRow_ = -1;
        else dRow_ = 0;

        if (col_ < targetCol_) dCol_ = 1;
        else if (col_ > targetCol_) dCol_ = -1;
        else dCol_ = 0;
        
        // 导弹寿命限制，防止无限追踪
        if (lifeTime_ > 30) kill();
    }

    row_ += dRow_;
    col_ += dCol_;
    
    // 简单的边界检查可以在World中做，或者这里做
    // 这里只负责移动
}

int Projectile::getDamage() const {
    switch (type_) {
        case ProjectileType::SHELL: return 1;
        case ProjectileType::TORPEDO: return 10;
        case ProjectileType::MISSILE: return 5;
        default: return 0;
    }
}
