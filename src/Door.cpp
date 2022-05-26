#include "Door.h"

bool Door::isOpen() const {
    return opened;
}

void Door::open() {
    opened = true;
}

b2Color Door::getColor() const {
    if (opened) {
        // Deep gray
        return {0.1, 0.1, 0.1};
    } else {
        // Red
        return {1, 0, 0};
    }
}