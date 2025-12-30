#pragma once
#include <string>

class Entity {
public:
    Entity(int row, int col, std::string glyph);
    virtual ~Entity() = default;

    virtual void update() = 0; // 纯虚函数，子类必须实现

    int getRow() const { return row_; }
    int getCol() const { return col_; }
    std::string getGlyph() const { return glyph_; }
    void setPos(int r, int c) { row_ = r; col_ = c; }
    
    int getColor() const { return color_; }
    void setColor(int c) { color_ = c; }

    bool isDead() const { return dead_; }
    void kill() { dead_ = true; }

protected:
    int row_;
    int col_;
    std::string glyph_;
    int color_ = 0; // 0 is default/white
    bool dead_ = false;
};
