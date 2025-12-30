#pragma once
#include "ship.h"

enum class EnemyType {
    GUNBOAT,
    DESTROYER,
    CRUISER,
    BOMBER
};

class EnemyShip : public Ship {
public:
    EnemyShip(int row, int col, EnemyType type);
    void update() override;
    
    // AI Logic needs player position
    void aiUpdate(int playerRow, int playerCol, int maxRows, int maxCols);
    
    EnemyType getEnemyType() const { return type_; }
    int getScoreValue() const;

private:
    EnemyType type_;
    int moveTimer_ = 0;
    int fireTimer_ = 0;
    int moveInterval_ = 0;
    int fireInterval_ = 0;
    
    // Bomber direction
    int bomberDir_ = 1; // 1 right, -1 left
    
    // Ammo counts
    int shells_ = 0;
    int torpedoes_ = 0;
    int missiles_ = 0;
};
