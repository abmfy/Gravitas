#ifndef Level1Factory_h
#define Level1Factory_h

#include <memory>

#include "Level.h"
#include "Level1.h"
#include "LevelFactory.h"

class Level1Factory: public LevelFactory {
    virtual std::unique_ptr<Level> create() override;
};

#endif