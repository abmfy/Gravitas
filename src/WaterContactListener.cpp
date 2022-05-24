#include "Box2D/Box2D.h"

#include "Water.h"
#include "WaterContactListener.h"
#include "WaterType.h"
using WaterType::poison, WaterType::water;

void WaterContactListener::contact(b2ParticleSystem &system, int a, int b) {
    void *const data1 {system.GetUserDataBuffer()[a]};
    void *const data2 {system.GetUserDataBuffer()[b]};
    if (data1 && data2) {
        WaterType &type1 {*static_cast<WaterType*>(data1)};
        WaterType &type2 {*static_cast<WaterType*>(data2)};
        if (type1 == poison || type2 == poison) {
            type1 = type2 = poison;
            system.GetColorBuffer()[a] = Water::getColor(poison);
            system.GetColorBuffer()[b] = Water::getColor(poison);
        }
    }
}