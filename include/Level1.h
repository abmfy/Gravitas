#ifndef LEVEL1_H
#define LEVEL1_H

#include "Level.h"

class Level1: public Level {
public:
    Level1();

    static Level *create();
};

#endif