#pragma once
#include "entity.h"
#include "projectile.h"
#include <vector>
#include <memory>

class Ship : public Entity {
public:
    Ship(int row, int col, std::string glyph, int hp);
    virtual ~Ship() = default;

    void takeDamage(int dmg);
    int getHp() const { return hp_; }
    int getMaxHp() const { return maxHp_; }
    
    // 收集本帧产生的新投射物，移交给Game管理
    std::vector<std::unique_ptr<Projectile>> collectNewProjectiles();

protected:
    void spawnProjectile(std::unique_ptr<Projectile> p);
    
    int hp_;
    int maxHp_;
    std::vector<std::unique_ptr<Projectile>> newProjectiles_;
};
