#include <memory>
using std::make_unique, std::unique_ptr;

#include "Level.h"
#include "Level1.h"
#include "Level1Factory.h"

unique_ptr<Level> Level1Factory::create() {
    return make_unique<Level1>();
}