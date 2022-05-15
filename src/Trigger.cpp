#include "Trigger.h"

Trigger::Trigger(int threshold): threshold {threshold} {}

void Trigger::trigger() {
    if (!triggered && count++ > threshold) {
        triggered = true;
    }
}

bool Trigger::isTriggered() const {
    return triggered;
}

b2Color Trigger::getColor() const {
    if (triggered) {
        // Green
        return {0, 1, 0};
    } else {
        // Red
        return {1, 0, 0};
    }
}