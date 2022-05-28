#include <memory>
using std::make_unique, std::unique_ptr;

#include "Level.h"
#include "Level4.h"
#include "Level4Factory.h"

unique_ptr<Level> Level4Factory::create() {
    return make_unique<Level4>();
}