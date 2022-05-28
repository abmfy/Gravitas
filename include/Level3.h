#ifndef Level3_h
#define Level3_h

#include "Level.h"

class Level3: public Level {
public:
    Level3();
    float GetDefaultViewZoom() const override;
};

#endif