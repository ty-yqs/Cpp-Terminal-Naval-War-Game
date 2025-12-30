#include "game.h"
#include <string>

int main(int argc, char** argv) {
    std::string mapPath;
    if (argc >= 2) {
        mapPath = argv[1];
    }

    Game game(mapPath);
    game.runLoop();
    return 0;
}
