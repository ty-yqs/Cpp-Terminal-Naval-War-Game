#include "renderer.h"
#include <sstream>
#include <vector>

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
    std::string glyph = entity.getGlyph();
    int r = entity.getRow() + 2;
    int c = entity.getCol() + 1;
    
    std::stringstream ss(glyph);
    std::string line;
    int maxWidth = 0;
    std::vector<std::string> lines;
    while (std::getline(ss, line)) {
        if ((int)line.size() > maxWidth) maxWidth = (int)line.size();
        lines.push_back(line);
    }

    // Clear the draw area for this entity first (prevents artifacts from previous frame/overdraw).
    const int screenTop = 2;
    const int screenBottom = rows_ + 1;
    const int screenLeft = 1;
    const int screenRight = cols_;

    int h = (int)lines.size();
    if (h == 0) h = 1;
    if (maxWidth <= 0) maxWidth = 1;

    for (int dr = 0; dr < h; ++dr) {
        int rr = r + dr;
        if (rr < screenTop || rr > screenBottom) continue;
        for (int dc = 0; dc < maxWidth; ++dc) {
            int cc = c + dc;
            if (cc < screenLeft || cc > screenRight) continue;
            mvaddch(rr, cc, ' ');
        }
    }

    if (entity.getColor() > 0) attron(COLOR_PAIR(entity.getColor()));

    // Draw glyph
    for (int i = 0; i < (int)lines.size(); ++i) {
        int rr = r + i;
        if (rr < screenTop || rr > screenBottom) continue;
        const std::string& ln = lines[i];
        for (int j = 0; j < (int)ln.size(); ++j) {
            int cc = c + j;
            if (cc < screenLeft || cc > screenRight) continue;
            mvaddch(rr, cc, ln[j]);
        }
    }
    
    if (entity.getColor() > 0) attroff(COLOR_PAIR(entity.getColor()));
}

void Renderer::printAt(int row, int col, const std::string &text) {
    mvaddnstr(row + 2, col + 1, text.c_str(), cols_ - col);
}

void Renderer::present() {
    refresh();
}
