#ifndef Level3_h
#define Level3_h

#include "Level.h"

class Level3: public Level {
    Level3();
public:
    float GetDefaultViewZoom() const override;
    static Level *create();
};

#endif