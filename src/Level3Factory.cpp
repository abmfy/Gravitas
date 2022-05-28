#include <memory>
using std::make_unique, std::unique_ptr;

#include "Level.h"
#include "Level3.h"
#include "Level3Factory.h"

unique_ptr<Level> Level3Factory::create() {
    return make_unique<Level3>();
}