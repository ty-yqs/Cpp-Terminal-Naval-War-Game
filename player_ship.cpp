#include "player_ship.h"
#include <sstream>
#include <vector>

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
    auto canPlaceGlyphAt = [&](int baseRow, int baseCol, const std::string& glyph) -> bool {
        std::stringstream ss(glyph);
        std::string line;
        int dr = 0;
        while (std::getline(ss, line)) {
            for (int dc = 0; dc < (int)line.size(); ++dc) {
                if (line[dc] == ' ') continue;
                const int rr = baseRow + dr;
                const int cc = baseCol + dc;
                if (!world.inBounds(rr, cc)) return false;
                if (world.isBlocked(rr, cc)) return false;
            }
            ++dr;
        }
        return true;
    };

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

    // 边界/障碍检查：按实际 glyph 占用的格子判断（避免船体出界导致“少一行”）
    if (input.dRow != 0 || input.dCol != 0) {
        if (canPlaceGlyphAt(nextRow, nextCol, glyph_)) {
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

        // 从船体在该方向的“最前沿占格”外侧一格生成，避免出生在自身占格里造成自伤
        int muzzleRow = row_;
        int muzzleCol = col_;
        {
            std::stringstream ss(glyph_);
            std::string line;
            int bestScore = -1e9;
            int glyphDr = 0;
            while (std::getline(ss, line)) {
                for (int glyphDc = 0; glyphDc < (int)line.size(); ++glyphDc) {
                    if (line[glyphDc] == ' ') continue;
                    const int score = glyphDr * dr + glyphDc * dc;
                    if (score > bestScore) {
                        bestScore = score;
                        muzzleRow = row_ + glyphDr;
                        muzzleCol = col_ + glyphDc;
                    }
                }
                ++glyphDr;
            }
        }

        spawnProjectile(std::make_unique<Projectile>(muzzleRow + dr, muzzleCol + dc, dr, dc, ProjectileType::SHELL));
    }
    
    auto fireSideParallel = [&](bool fireLeftOfForward) {
        if (shells_ < 3) return;
        shells_ -= 3;

        // 当前移动方向：优先用本帧输入，否则用最后一次移动方向
        int forwardRow = (input.dRow != 0 || input.dCol != 0) ? input.dRow : lastDirRow_;
        int forwardCol = (input.dRow != 0 || input.dCol != 0) ? input.dCol : lastDirCol_;
        if (forwardRow == 0 && forwardCol == 0) {
            forwardRow = -1;
            forwardCol = 0;
        }

        // 相对 forward 的左/右：
        // left  = (-forwardCol, forwardRow)
        // right = ( forwardCol,-forwardRow)
        const int fireDirRow = fireLeftOfForward ? (-forwardCol) : (forwardCol);
        const int fireDirCol = fireLeftOfForward ? ( forwardRow) : (-forwardRow);

        // 从当前 glyph 的“非空格占格”取前三格作为炮口，并向侧边外移一格生成
        std::vector<std::pair<int, int>> occupied;
        occupied.reserve(3);
        {
            std::stringstream ss(glyph_);
            std::string line;
            int dr = 0;
            while (std::getline(ss, line)) {
                for (int dc = 0; dc < (int)line.size(); ++dc) {
                    if (line[dc] == ' ') continue;
                    occupied.push_back({dr, dc});
                    if ((int)occupied.size() == 3) break;
                }
                if ((int)occupied.size() == 3) break;
                ++dr;
            }
        }

        for (const auto& cell : occupied) {
            const int shipRow = row_ + cell.first;
            const int shipCol = col_ + cell.second;
            spawnProjectile(std::make_unique<Projectile>(
                shipRow + fireDirRow,
                shipCol + fireDirCol,
                fireDirRow,
                fireDirCol,
                ProjectileType::SHELL));
        }
    };

    if (input.fireSpreadLeft) {
        fireSideParallel(true);
    }
    if (input.fireSpreadRight) {
        fireSideParallel(false);
    }

    if (input.fireMissile && missiles_ > 0) {
        missiles_--;
        // 导弹按当前运动方向发射；若静止则沿最后运动方向；再否则默认向上
        int dr = (input.dRow != 0 || input.dCol != 0) ? input.dRow : lastDirRow_;
        int dc = (input.dRow != 0 || input.dCol != 0) ? input.dCol : lastDirCol_;
        if (dr == 0 && dc == 0) {
            dr = -1;
            dc = 0;
        }

        // 从船体在该方向的“最前沿占格”外侧一格生成，避免出生在自身占格里
        int muzzleRow = row_;
        int muzzleCol = col_;
        {
            std::stringstream ss(glyph_);
            std::string line;
            int bestScore = -1e9;
            int glyphDr = 0;
            while (std::getline(ss, line)) {
                for (int glyphDc = 0; glyphDc < (int)line.size(); ++glyphDc) {
                    if (line[glyphDc] == ' ') continue;
                    const int score = glyphDr * dr + glyphDc * dc;
                    if (score > bestScore) {
                        bestScore = score;
                        muzzleRow = row_ + glyphDr;
                        muzzleCol = col_ + glyphDc;
                    }
                }
                ++glyphDr;
            }
        }

        auto m = std::make_unique<Projectile>(muzzleRow + dr, muzzleCol + dc, dr, dc, ProjectileType::MISSILE);
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
