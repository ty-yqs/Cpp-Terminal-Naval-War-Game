#pragma once
#include "entity.h"

enum class PickupType {
    WEAPON,
    MEDICAL
};

class Pickup : public Entity {
public:
    Pickup(int row, int col, PickupType type);
    void update() override;
    PickupType getType() const;

private:
    PickupType type_;
};
