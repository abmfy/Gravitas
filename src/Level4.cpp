#include "Level4.h"

#include "Water.h"

Level4::Level4() {
    id = 4;

    // Ground body
    b2BodyDef bodyDef;
    b2Body &ground {*m_world->CreateBody(&bodyDef)};

    // The border
    {
        b2ChainShape chain;
        const b2Vec2 vertices[] {{-5, 0}, {20, 0}, {20, 40}, {-20, 40}, {-20, 0}, {-10, 0}};
        chain.CreateChain(vertices, 6);
        ground.CreateFixture(&chain, 0);
    }

    // The Wall
    {
        b2ChainShape chain;
        const b2Vec2 vertices[] {{-5, 0}, {10, 0}, {10, 35}, {-10, 35}, {-10, 30}, {5, 30}, 
            {5, 20}, {-10, 20}, {-10, 15}, {5, 15}, {5, 5}, {-10, 5}, {-10, 0}};
        chain.CreateChain(vertices, B2_ARRAY_SIZE(vertices));
        ground.CreateFixture(&chain, 0);
    }

    // The water
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{-10, 34}, {-10, 31}, {-5, 34}, {-5, 31}};
        square.Set(vertices, 4);

        Water water {*m_particleSystem, square};

        // The threshold for passing the level
        levelManager.setThreshold(water.getParticleCount() * 0.9);
    }

    // The magic body
    {
        b2BodyDef def;
        def.type = b2_dynamicBody;
        def.fixedRotation = true;
        b2Body &body1 = *m_world->CreateBody(&def);
        b2Body &body2 = *m_world->CreateBody(&def);
        
        b2PolygonShape square;

        const b2Vec2 square1[] {{-0.1, 30}, {-0.1, 34.9}, {-5, 34.9}, {-5, 30}};
        square.Set(square1, 4);
        body1.CreateFixture(&square, 0.1);

        const b2Vec2 square2[] {{-0.1, 15}, {-0.1, 19.9}, {-5, 19.9}, {-5, 15}};
        square.Set(square2, 4);
        body2.CreateFixture(&square, 0.11);

        b2DistanceJointDef jointDef;
        jointDef.Initialize(&body1, &body2, {-7.5, 32.5}, {-7.5, 17.5});
        jointDef.collideConnected = true;
        jointDef.length = 15;

        m_world->CreateJoint(&jointDef);
    }
}

Level *Level4::create() {
    return new Level4;
}