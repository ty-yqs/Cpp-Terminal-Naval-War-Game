#pragma once
#include <vector>

class World {
public:
    static constexpr int kRows = 30;
    static constexpr int kCols = 80;

    World(); // Constructor to init obstacles

    bool inBounds(int row, int col) const;
    bool isBlocked(int row, int col) const;

private:
    bool obstacles_[kRows][kCols];
};
