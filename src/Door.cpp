#include "Door.h"

bool Door::isOpen() const {
    return opened;
}

void Door::open() {
    opened = true;
}

b2Color Door::getColor() const {
    if (opened) {
        // Black
        return {};
    } else {
        // Red
        return {1, 0, 0};
    }
}