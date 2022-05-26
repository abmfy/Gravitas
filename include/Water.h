#ifndef Water_h
#define Water_h

#include "Box2D/Box2D.h"

#include "WaterType.h"

// Manage a particle group representing water
class Water {
    b2ParticleGroup *group;

public:
    static b2ParticleColor getColor(WaterType);
    // Create some water in system
    Water(b2ParticleSystem&, const b2Shape&, WaterType = WaterType::water, 
        const unsigned = b2_tensileParticle | b2_fixtureContactListenerParticle | b2_destructionListenerParticle | b2_colorMixingParticle | b2_particleContactListenerParticle);
    int getParticleCount() const;
};

#endif