#include "Level3.h"

#include "Water.h"
#include "WaterType.h"

Level3::Level3() {
    id = 3;

    // Ground body
    b2BodyDef bodyDef;
    b2Body &ground {*m_world->CreateBody(&bodyDef)};

    // The border
    {
        b2ChainShape chain;
        const b2Vec2 vertices[] {{-2.5, 0}, {30, 0}, {30, 60}, {-30, 60}, {-30, 0}, {-7.5, 0}};
        chain.CreateChain(vertices, B2_ARRAY_SIZE(vertices));
        ground.CreateFixture(&chain, 0);
    }
    {
        b2ChainShape chain;
        const b2Vec2 vertices[] {{-2.5, 15}, {-2.5, 20}, {2.5, 20}, {2.5, 15}};
        chain.CreateLoop(vertices, 4);
        ground.CreateFixture(&chain, 0);
    }
    {
        b2ChainShape chain;
        const b2Vec2 vertices[] {{-2.5, 45}, {-2.5, 40}, {2.5, 40}, {2.5, 45}};
        chain.CreateChain(vertices, 4);
        ground.CreateFixture(&chain, 0);
    }

    // The wall
    {
        b2ChainShape chain;
        const b2Vec2 vertices[] {
            {-7.5, 0}, {-7.5, 25}, {2.5, 25}, {2.5, 35}, {-2.5, 35},
            {-2.5, 30}, {-7.5, 30}, {-7.5, 50}, {7.5, 50}, {7.5, 15}, {12.5, 15},
            {12.5, 5}, {7.5, 5}, {7.5, 10}, {-2.5, 10}, {-2.5, 0}
        };
        chain.CreateChain(vertices, B2_ARRAY_SIZE(vertices));
        ground.CreateFixture(&chain, 0);
    }

    // The water
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{-2.5, 43}, {-2.5, 40}, {2.5, 40}, {2.5, 43}};
        square.Set(vertices, 4);

        Water water {*m_particleSystem, square};

        // The threshold for passing the level
        levelManager.setThreshold(water.getParticleCount() * 0.7);
    }

    // The poison
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{-2.6, 30}, {-2.6, 33}, {-7.4, 30}, {-7.4, 33}};
        square.Set(vertices, 4);

        Water poison {*m_particleSystem, square, WaterType::poison};
    }

    // The poison
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{7.6, 5}, {7.6, 8}, {12.4, 5}, {12.4, 8}};
        square.Set(vertices, 4);

        Water poison {*m_particleSystem, square, WaterType::poison};
    }
}

float Level3::GetDefaultViewZoom() const {
    return 1.5;
}

Level *Level3::create() {
    return new Level3;
}