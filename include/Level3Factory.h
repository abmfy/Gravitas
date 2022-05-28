#ifndef Level3Factory_h
#define Level3Factory_h

#include <memory>

#include "Level.h"
#include "Level3.h"
#include "LevelFactory.h"

class Level3Factory: public LevelFactory {
    virtual std::unique_ptr<Level> create() override;
};

#endif