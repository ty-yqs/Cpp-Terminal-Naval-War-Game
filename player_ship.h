#pragma once
#include "ship.h"
#include "input_manager.h"
#include "world.h"

class PlayerShip : public Ship {
public:
    PlayerShip(int row, int col);
    void update() override;
    void handleInput(const InputState& input, const World& world);
    
    void addCoins(int amount) { coins_ += amount; }
    int getCoins() const { return coins_; }
    
    void addAmmo(int shells, int missiles);
    void heal(int amount);
    
    int getShells() const { return shells_; }
    int getMissiles() const { return missiles_; }

private:
    int coins_ = 0;
    int shells_ = 100;
    int missiles_ = 5;
    
    // 记录最后移动方向，用于发射子弹
    int lastDirRow_ = -1; // 默认向上
    int lastDirCol_ = 0;
};
