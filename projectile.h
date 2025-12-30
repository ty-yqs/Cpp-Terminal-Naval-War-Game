#pragma once
#include "entity.h"

enum class ProjectileType {
    SHELL,
    TORPEDO,
    MISSILE
};

class Projectile : public Entity {
public:
    Projectile(int row, int col, int dRow, int dCol, ProjectileType type);
    void update() override;
    int getDamage() const;
    ProjectileType getType() const { return type_; }
    
    // 用于导弹追踪
    void setTarget(int tRow, int tCol);

private:
    int dRow_;
    int dCol_;
    ProjectileType type_;
    int lifeTime_ = 0;
    
    // 导弹追踪逻辑
    bool tracking_ = false;
    int targetRow_ = 0;
    int targetCol_ = 0;
};
