#ifndef Level2Factory_h
#define Level2Factory_h

#include <memory>

#include "Level.h"
#include "Level2.h"
#include "LevelFactory.h"

class Level2Factory: public LevelFactory {
    virtual std::unique_ptr<Level> create() override;
};

#endif