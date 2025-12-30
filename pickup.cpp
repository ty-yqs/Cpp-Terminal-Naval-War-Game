#include "pickup.h"

Pickup::Pickup(int row, int col, PickupType type)
    : Entity(row, col, " "), type_(type) {
    setColor(7); // Blue
    if (type == PickupType::WEAPON) glyph_ = "W";
    else glyph_ = "+";
}

void Pickup::update() {}

PickupType Pickup::getType() const { return type_; }
