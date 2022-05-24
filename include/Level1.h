#ifndef Level1_h
#define Level1_h

#include "Level.h"

class Level1: public Level {
    Level1();
public:
    static Level *create();
};

#endif