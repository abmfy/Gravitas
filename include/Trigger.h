#ifndef TRIGGER_H
#define TRIGGER_H

#include "Box2D/Box2D.h"

// A pressure trigger.
// Once triggered it will remain triggered
class Trigger {
    bool triggered {};
    // Trigger after some time
    int threshold;
    int count {};
public:
    b2PolygonShape shape;

    Trigger(int);

    void trigger();
    bool isTriggered() const;
    // Render different colors to indicate
    // whether a trigger is triggered
    b2Color getColor() const;
};

#endif