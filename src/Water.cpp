#include "Box2D/Box2D.h"

#include "Water.h"
#include "WaterType.h"
using WaterType::poison, WaterType::water;

b2ParticleColor Water::getColor(WaterType type) {
    switch (type) {
        case water: {
            // Blue
            return {0, 0, 255, 255};
        }
        case poison: {
            // Green
            return {0, 255, 0, 255};
        }
        default: {
            assert(false);
        }
    }
}

Water::Water(b2ParticleSystem &system, const b2Shape &shape, WaterType type, const unsigned flags) {
    b2ParticleGroupDef def;
    def.shape = &shape;
    def.color = getColor(type);
    def.flags = flags;
    if (type == poison) {
        def.flags |= b2_elasticParticle;
        def.groupFlags = b2_solidParticleGroup;
    }
    def.userData = new WaterType {type};
    group = system.CreateParticleGroup(def);
}

int Water::getParticleCount() const {
    return group->GetParticleCount();
}