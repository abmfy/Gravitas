#include "Level2.h"

#include "Water.h"

Level2::Level2(): trigger1 {30000}, trigger2 {5000} {
    id = 2;

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

    // The walls
    {
        b2EdgeShape edge;
        edge.Set({-15, 0}, {-15, 20});
        ground.CreateFixture(&edge, 0);
        edge.Set({-5, 0}, {-5, 20});
        ground.CreateFixture(&edge, 0);
        edge.Set({5, 0}, {5, 20});
        ground.CreateFixture(&edge, 0);

        b2PolygonShape square;
        const b2Vec2 vertices[] {{15, 0}, {15, 20}, {20, 0}, {20, 20}};
        square.Set(vertices, 4);
        ground.CreateFixture(&square, 0);
    }

    // The trigger
    {
        {
            b2PolygonShape square;
            const b2Vec2 vertices[] {{-13, 0}, {-13, 2}, {-7, 0}, {-7, 2}};
            square.Set(vertices, 4);
            trigger1.shape = square;

            b2BodyDef bodyDef;
            bodyDef.userData = &trigger1;
            world.CreateBody(&bodyDef)->CreateFixture(&square, 0);
        }

        {
            b2PolygonShape square;
            const b2Vec2 vertices[] {{7, 0}, {7, 2}, {13, 0}, {13, 2}};
            square.Set(vertices, 4);
            trigger2.shape = square;

            b2BodyDef bodyDef;
            bodyDef.userData = &trigger2;
            world.CreateBody(&bodyDef)->CreateFixture(&square, 0);
        }
    }

    // The door
    {
        b2EdgeShape shape;
        shape.Set({20, 20}, {20, 25});
        door.shape = shape;

        b2BodyDef bodyDef;
        b2Body *body {world.CreateBody(&bodyDef)};
        door.body = body;
        body->CreateFixture(&shape, 0);
    }

    // The water
    {
        b2PolygonShape square;
        const b2Vec2 vertices[] {{-15, 7}, {-15, 12}, {-5, 7}, {-5, 12}};
        square.Set(vertices, 4);

        Water water {*particleSystem, square};

        // The threshold for passing the level
        levelManager.setThreshold(water.getParticleCount() * 0.9);
    }
}

void Level2::drawTrigger(const Trigger &trigger) {
    const b2PolygonShape &shape {trigger.shape};
    debugDraw.DrawSolidPolygon(shape.m_vertices, shape.m_count, trigger.getColor());
}

void Level2::drawTriggers() {
    drawTrigger(trigger1);
    drawTrigger(trigger2);
}

void Level2::drawDoor() {
    debugDraw.DrawSegment(door.shape.m_vertex1, door.shape.m_vertex2, door.getColor());
}

void Level2::BeginContact(b2ParticleSystem*, b2ParticleBodyContact *contact) {
    if (void *data {contact->body->GetUserData()}; data != nullptr) {
        static_cast<Trigger*>(data)->trigger();
    }
}

void Level2::Step(int paused) {
    Level::Step(paused);
    if (trigger1.isTriggered() && trigger2.isTriggered()) {
        door.open();
        if (door.body) {
            world.DestroyBody(door.body);
            door.body = nullptr;
        }
    }
    drawTriggers();
    drawDoor();
}