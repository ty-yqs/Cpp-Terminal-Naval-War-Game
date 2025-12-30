#pragma once
#include <string>

class World {
public:
    static constexpr int kRows = 30;
    static constexpr int kCols = 80;

    World(); // Random obstacles
    explicit World(const std::string& mapFilePath); // Load map from file (fallback to random)

    bool inBounds(int row, int col) const;
    bool isBlocked(int row, int col) const;

private:
    void clearAll();
    void clearSpawnArea();
    void generateRandomObstacles();
    bool loadFromFile(const std::string& path);

    bool obstacles_[kRows][kCols];
};
