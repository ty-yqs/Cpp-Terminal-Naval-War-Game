#include "world.h"
#include <cstdlib>

World::World() {
    // Initialize obstacles
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            obstacles_[r][c] = false;
        }
    }

    // Random obstacles (Islands) - 10% chance?
    // Avoid spawn area (bottom center)
    for (int r = 5; r < kRows - 5; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (std::rand() % 20 == 0) { // 5% chance
                obstacles_[r][c] = true;
            }
        }
    }
}

bool World::inBounds(int row, int col) const {
    return row >= 0 && row < kRows && col >= 0 && col < kCols;
}

bool World::isBlocked(int row, int col) const {
    if (!inBounds(row, col)) return true;
    return obstacles_[row][col];
}
