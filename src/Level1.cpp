#include "Level1.h"

Level1::Level1() {
    // Ground body
    b2BodyDef bodyDef;
    b2Body &ground {*m_world->CreateBody(&bodyDef)};

    // The border
    {
        b2ChainShape chain;
        const b2Vec2 vertices[] {{20, 20}, {20, 0}, {-20, 0}, {-20, 40}, {20, 40}, {20, 25}};
        chain.CreateChain(vertices, 6);
        ground.CreateFixture(&chain, 0);
    }

    // The higher square ground
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{0, 0}, {20, 0}, {20, 20}, {0, 20}};
        square.Set(vertices, 4);
        ground.CreateFixture(&square, 0);
    }

    // The lower square ground
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{0, 0}, {-20, 0}, {-20, 10}, {0, 10}};
        square.Set(vertices, 4);
        ground.CreateFixture(&square, 0);
    }

    // The water
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{0, 10.05}, {-20, 10.05}, {-20, 15}, {0, 15}};
        square.Set(vertices, 4);
        
        b2ParticleGroupDef particleGroupDef;
        particleGroupDef.shape = &square;
        particleGroupDef.color = {0, 0, 127, 255};
        particleGroupDef.flags = b2_tensileParticle | b2_destructionListenerParticle;
        b2ParticleGroup &group {*m_particleSystem->CreateParticleGroup(particleGroupDef)};

        // The threshold for passing the level
        threshold = group.GetParticleCount() * 0.9;
    }
}

Level *Level1::create() {
    return new Level1;
}