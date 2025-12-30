#include "renderer.h"
#include <sstream>

Renderer::Renderer(int rows, int cols) : rows_(rows), cols_(cols) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    start_color();
    
    init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Player
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Gunboat
    init_pair(3, COLOR_RED, COLOR_BLACK);     // Destroyer
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK); // Cruiser
    init_pair(5, COLOR_CYAN, COLOR_BLACK);    // Bomber
    init_pair(6, COLOR_WHITE, COLOR_BLACK);   // Projectile
    init_pair(7, COLOR_BLUE, COLOR_BLACK);    // Pickup
}

Renderer::~Renderer() {
    endwin();
}

void Renderer::clear() {
    ::clear();
}

void Renderer::drawBorders() {
    for (int c = 0; c <= cols_ + 1; ++c) {
        mvaddch(1, c, '-');
        mvaddch(rows_ + 2, c, '-');
    }
    for (int r = 1; r <= rows_ + 2; ++r) {
        mvaddch(r, 0, '|');
        mvaddch(r, cols_ + 1, '|');
    }
}

void Renderer::drawWorld(const World& world) {
    for (int r = 0; r < World::kRows; ++r) {
        for (int c = 0; c < World::kCols; ++c) {
            if (world.isBlocked(r, c)) {
                mvaddch(r + 2, c + 1, '#');
            }
        }
    }
}

void Renderer::drawHud(const std::string &status) {
    mvaddnstr(0, 0, status.c_str(), cols_);
}

void Renderer::drawEntity(const Entity& entity) {
    // Offset by 1 row (HUD is 0, Border is 1, Game starts at 2?)
    // Let's say HUD is row 0. Top border is row 1. Game rows 0..29 map to screen rows 2..31.
    if (entity.getColor() > 0) attron(COLOR_PAIR(entity.getColor()));
    
    std::string glyph = entity.getGlyph();
    int r = entity.getRow() + 2;
    int c = entity.getCol() + 1;
    
    std::stringstream ss(glyph);
    std::string line;
    int lineIdx = 0;
    while (std::getline(ss, line)) {
        mvaddstr(r + lineIdx, c, line.c_str());
        lineIdx++;
    }
    
    if (entity.getColor() > 0) attroff(COLOR_PAIR(entity.getColor()));
}

void Renderer::printAt(int row, int col, const std::string &text) {
    mvaddnstr(row + 2, col + 1, text.c_str(), cols_ - col);
}

void Renderer::present() {
    refresh();
}
