#include "Level1.h"

#include "Water.h"

Level1::Level1() {
    id = 1;

    // Ground body
    b2BodyDef bodyDef;
    b2Body &ground {*world.CreateBody(&bodyDef)};

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

        Water water {*particleSystem, square};

        // The threshold for passing the level
        levelManager.setThreshold(water.getParticleCount() * 0.9);
    }
}