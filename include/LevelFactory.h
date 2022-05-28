#ifndef LevelFactory_h
#define LevelFactory_h

#include <memory>

#include "Level.h"

class LevelFactory {
public:
    virtual ~LevelFactory();
    virtual std::unique_ptr<Level> create() = 0;
};

#endif