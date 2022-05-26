#ifndef Level2_h
#define Level2_h

#include "Level.h"
#include "Trigger.h"
#include "Door.h"

class Level2: public Level {
    Trigger trigger1, trigger2;
    Door door;

    void drawTrigger(const Trigger&);
    void drawTriggers();
    void drawDoor();
    Level2();
public:
    // Detect the contact between the water and the trigger
    void BeginContact(b2ParticleSystem*, b2ParticleBodyContact*) override;
    void Step(int) override;

    static Level *create();
};

#endif