#pragma once
#include <vector>
#include <memory>
#include <string>
#include "world.h"
#include "renderer.h"
#include "input_manager.h"
#include "player_ship.h"
#include "enemy_ship.h"
#include "projectile.h"
#include "pickup.h"

class Game {
public:
    explicit Game(std::string mapFilePath = "");
    void runLoop();

private:
    void handleInput(const InputState &input);
    void update();
    void render();
    
    void spawnEnemies();
    void spawnPickups();
    void checkCollisions();
    void collectProjectiles();

    void startLevel(int newLevel);

    World world_;
    Renderer renderer_;
    InputManager input_;
    bool running_ = false;
    
    std::unique_ptr<PlayerShip> player_;
    std::vector<std::unique_ptr<EnemyShip>> enemies_;
    std::vector<std::unique_ptr<Projectile>> projectiles_;
    std::vector<std::unique_ptr<Pickup>> pickups_;
    
    int spawnTimer_ = 0;

    std::string mapFilePath_;

    int level_ = 1;
    static constexpr int kLevel1WinCoins = 1000;
    static constexpr int kLevel2WinCoins = 2000;
};
