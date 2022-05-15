#ifndef DOOR_H
#define DOOR_H

#include "Box2D/Box2D.h"

class Door {
    bool opened {};
public:
    b2EdgeShape shape;
    b2Body *body;

    bool isOpen() const;
    void open();
    b2Color getColor() const;
};

#endif