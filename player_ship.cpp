#include "player_ship.h"

PlayerShip::PlayerShip(int row, int col)
    : Ship(row, col, " ^\n \"\n V", 1000) {
    setColor(1); // Green
    // 初始符号可能根据方向变，这里简单用^
}

void PlayerShip::update() {
    // 玩家更新逻辑，主要是状态维护，移动在handleInput处理
    // 自动回复弹药逻辑
    static int tick = 0;
    tick++;
    if (tick % 10 == 0) shells_++;
    if (tick % 1000 == 0) missiles_++;
}

void PlayerShip::handleInput(const InputState& input, const World& world) {
    // 移动
    int nextRow = row_ + input.dRow;
    int nextCol = col_ + input.dCol;
    
    // 记录方向
    if (input.dRow != 0 || input.dCol != 0) {
        lastDirRow_ = input.dRow;
        lastDirCol_ = input.dCol;
        
        // 更新符号
        if (input.dRow != 0) glyph_ = " ^\n \"\n V";
        else glyph_ = "<=>";
    }

    // 边界检查
    if (world.inBounds(nextRow, nextCol)) {
        if (!world.isBlocked(nextRow, nextCol)) {
            row_ = nextRow;
            col_ = nextCol;
        }
    }

    // 攻击
    if (input.fireShell && shells_ > 0) {
        shells_--;
        // 发射方向为移动方向，如果静止则用最后方向
        int dr = (input.dRow != 0 || input.dCol != 0) ? input.dRow : lastDirRow_;
        int dc = (input.dRow != 0 || input.dCol != 0) ? input.dCol : lastDirCol_;
        if (dr == 0 && dc == 0) dr = -1; // 默认向上

        spawnProjectile(std::make_unique<Projectile>(row_ + dr, col_ + dc, dr, dc, ProjectileType::SHELL));
    }
    
    if (input.fireSpreadLeft && shells_ >= 3) {
        shells_ -= 3;
        // 左侧散射: 左上, 左, 左下? 或者是向左发射三发?
        // 题目: "Three bullets by hitting 'a', direction left"
        // 简单实现：向左发射
        spawnProjectile(std::make_unique<Projectile>(row_, col_ - 1, 0, -1, ProjectileType::SHELL));
        spawnProjectile(std::make_unique<Projectile>(row_ - 1, col_ - 1, -1, -1, ProjectileType::SHELL));
        spawnProjectile(std::make_unique<Projectile>(row_ + 1, col_ - 1, 1, -1, ProjectileType::SHELL));
    }

    if (input.fireSpreadRight && shells_ >= 3) {
        shells_ -= 3;
        // 向右散射
        spawnProjectile(std::make_unique<Projectile>(row_, col_ + 1, 0, 1, ProjectileType::SHELL));
        spawnProjectile(std::make_unique<Projectile>(row_ - 1, col_ + 1, -1, 1, ProjectileType::SHELL));
        spawnProjectile(std::make_unique<Projectile>(row_ + 1, col_ + 1, 1, 1, ProjectileType::SHELL));
    }

    if (input.fireMissile && missiles_ > 0) {
        missiles_--;
        // 导弹，初始方向向上，之后会自动追踪
        auto m = std::make_unique<Projectile>(row_ - 1, col_, -1, 0, ProjectileType::MISSILE);
        // 需要在Game层设置目标，这里先生成
        spawnProjectile(std::move(m));
    }
}

void PlayerShip::addAmmo(int shells, int missiles) {
    shells_ += shells;
    missiles_ += missiles;
}

void PlayerShip::heal(int amount) {
    hp_ += amount;
    if (hp_ > maxHp_) hp_ = maxHp_;
}
