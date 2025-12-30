#include "world.h"
#include <cstdlib>
#include <fstream>
#include <string>

World::World() {
    clearAll();

    // No map specified: generate random obstacles.
    generateRandomObstacles();

    // Always keep a safe spawn area.
    clearSpawnArea();
}

World::World(const std::string& mapFilePath) {
    clearAll();

    // Map specified: try loading it. If it fails, fall back to random.
    if (!loadFromFile(mapFilePath)) {
        generateRandomObstacles();
    }

    clearSpawnArea();
}

void World::clearAll() {
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            obstacles_[r][c] = false;
        }
    }
}

void World::generateRandomObstacles() {
    // Random obstacles (Islands)
    // Keep top/bottom areas clearer to reduce unavoidable collisions.
    for (int r = 5; r < kRows - 5; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (std::rand() % 20 == 0) { // 5% chance
                obstacles_[r][c] = true;
            }
        }
    }
}

void World::clearSpawnArea() {
    // Player spawns near bottom center; clear a small rectangle.
    const int centerRow = kRows - 2;
    const int centerCol = kCols / 2;

    for (int r = centerRow - 3; r <= centerRow + 1; ++r) {
        for (int c = centerCol - 6; c <= centerCol + 6; ++c) {
            if (inBounds(r, c)) {
                obstacles_[r][c] = false;
            }
        }
    }
}

bool World::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    clearAll();

    std::string line;
    int row = 0;
    bool sawAnyCell = false;
    while (row < kRows && std::getline(in, line)) {
        // Skip completely empty lines.
        bool hasNonSpace = false;
        for (char ch : line) {
            if (ch != ' ' && ch != '\t' && ch != '\r') {
                hasNonSpace = true;
                break;
            }
        }
        if (!hasNonSpace) continue;

        int col = 0;
        for (char ch : line) {
            if (ch == ' ' || ch == '\t' || ch == '\r') continue;
            if (col >= kCols) break;

            // '.' or '0' => empty; '#', '1', 'X' => blocked.
            if (ch == '#' || ch == '1' || ch == 'X' || ch == 'x') {
                obstacles_[row][col] = true;
                sawAnyCell = true;
            } else if (ch == '.' || ch == '0') {
                obstacles_[row][col] = false;
                sawAnyCell = true;
            } else {
                // Unknown character: treat as empty but still count as a cell.
                obstacles_[row][col] = false;
                sawAnyCell = true;
            }
            col++;
        }
        // Missing columns stay empty.
        row++;
    }

    // If the file had no usable content, treat it as invalid.
    return sawAnyCell;
}

bool World::inBounds(int row, int col) const {
    return row >= 0 && row < kRows && col >= 0 && col < kCols;
}

bool World::isBlocked(int row, int col) const {
    if (!inBounds(row, col)) return true;
    return obstacles_[row][col];
}
