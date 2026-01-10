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

enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    WIN
};

class Game {
public:
    explicit Game(std::string mapFilePath = "");
    void runLoop();

private:
    void handleInput(const InputState &input);
    void handleMenuInput(const InputState &input);
    void update();
    void render();
    void renderMenu();
    
    void spawnEnemies();
    void spawnPickups();
    void checkCollisions();
    void collectProjectiles();

    void startLevel(int newLevel);

    World world_;
    Renderer renderer_;
    InputManager input_;
    bool running_ = false;
    GameState state_ = GameState::MENU;
    int menuSelection_ = 0;
    
    std::unique_ptr<PlayerShip> player_;
    std::vector<std::unique_ptr<EnemyShip>> enemies_;
    std::vector<std::unique_ptr<Projectile>> projectiles_;
    std::vector<std::unique_ptr<Pickup>> pickups_;
    
    int spawnTimer_ = 0;

    std::string mapFilePath_;

    int level_ = 1;
    static constexpr int kLevel1WinCoins = 100;
    static constexpr int kLevel2WinCoins = 200;
};
