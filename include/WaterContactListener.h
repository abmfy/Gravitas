#ifndef WaterContactListener_h
#define WaterContactListener_h

#include "Box2D/Box2D.h"

class WaterContactListener {
public:
    static void contact(b2ParticleSystem&, int, int);
};

#endif