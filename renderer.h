#pragma once
#include <string>
#include <ncurses.h>
#include "entity.h"
#include "world.h"

class Renderer {
public:
    Renderer(int rows, int cols);
    ~Renderer();

    void clear();
    void drawBorders();
    void drawWorld(const World& world);
    void drawHud(const std::string &status);
    void drawEntity(const Entity& entity);
    void printAt(int row, int col, const std::string &text);
    void present();

private:
    int rows_;
    int cols_;
};
