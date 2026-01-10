#include "input_manager.h"

InputState InputManager::poll() {
    InputState state;
    int ch = getch();
    switch (ch) {
    case KEY_UP:
        state.dRow = -1;
        state.up = true;
        break;
    case KEY_DOWN:
        state.dRow = 1;
        state.down = true;
        break;
    case KEY_LEFT:
        state.dCol = -1;
        break;
    case KEY_RIGHT:
        state.dCol = 1;
        break;
    case ' ':
        state.fireShell = true;
        break;
    case 'a':
    case 'A':
        state.fireSpreadLeft = true;
        break;
    case 'd':
    case 'D':
        state.fireSpreadRight = true;
        break;
    case 'u':
    case 'U':
        state.fireMissile = true;
        break;
    case 'q':
    case 'Q':
        state.quit = true;
        break;
    default:
        break;
    }
    return state;
}
