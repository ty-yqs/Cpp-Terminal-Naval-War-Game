#pragma once
#include <ncurses.h>

struct InputState {
    int dRow = 0;
    int dCol = 0;
    bool fireShell = false;
    bool fireSpreadLeft = false;
    bool fireSpreadRight = false;
    bool fireMissile = false;
    bool quit = false;
};

class InputManager {
public:
    InputState poll();
};
