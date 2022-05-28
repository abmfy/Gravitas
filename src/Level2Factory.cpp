#include <memory>
using std::make_unique, std::unique_ptr;

#include "Level.h"
#include "Level2.h"
#include "Level2Factory.h"

unique_ptr<Level> Level2Factory::create() {
    return make_unique<Level2>();
}