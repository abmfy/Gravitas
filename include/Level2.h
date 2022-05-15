#ifndef LEVEL2_H
#define LEVEL2_H

#include "Level.h"
#include "Trigger.h"
#include "Door.h"

class Level2: public Level {
    Trigger trigger1, trigger2;
    Door door;

    void drawTrigger(const Trigger&);
    void drawTriggers();
    void drawDoor();
public:
    Level2();

    // Detect the contact between the water and the trigger
    void BeginContact(b2ParticleSystem*, b2ParticleBodyContact*) override;
    void Step(Settings*) override;

    static Level *create();
};

#endif