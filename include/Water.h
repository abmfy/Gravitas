#ifndef Water_h
#define Water_h

#include "Box2D/Box2D.h"

// Manage a particle group representing water
class Water {
    b2ParticleGroup *group;

public:
    enum class Type {
        water, poison
    };

    static b2ParticleColor getColor(Type);
    // Create some water in system
    Water(b2ParticleSystem&, const b2Shape&, Type = Type::water, 
        const unsigned = b2_staticPressureParticle | b2_tensileParticle | b2_fixtureContactListenerParticle | b2_destructionListenerParticle | b2_colorMixingParticle | b2_particleContactListenerParticle);
    int getParticleCount() const;
};

#endif