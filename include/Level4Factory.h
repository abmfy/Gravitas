#ifndef Level4Factory_h
#define Level4Factory_h

#include <memory>

#include "Level.h"
#include "Level4.h"
#include "LevelFactory.h"

class Level4Factory: public LevelFactory {
    virtual std::unique_ptr<Level> create() override;
};

#endif