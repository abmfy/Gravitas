#ifndef Levels_h
#define Levels_h

#include <array>
#include <memory>

#include "LevelFactory.h"
#include "Level1Factory.h"
#include "Level2Factory.h"
#include "Level3Factory.h"
#include "Level4Factory.h"

const std::unique_ptr<LevelFactory> levelFactories[] {
    std::make_unique<Level1Factory>(),
    std::make_unique<Level2Factory>(),
    std::make_unique<Level3Factory>(),
    std::make_unique<Level4Factory>()
};

const int levelCount {std::size(levelFactories)};

#endif