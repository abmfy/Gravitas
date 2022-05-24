#include <cstdio>

#include "Level.h"

#include "glui/glui.h"

#include "WaterContactListener.h"


void DestructionListener::SayGoodbye(b2Joint *joint) {
    if (level->m_mouseJoint == joint) {
        level->m_mouseJoint = nullptr;
    } else {
        level->JointDestroyed(joint);
    }
}

void DestructionListener::SayGoodbye(b2ParticleGroup *group) {
    level->ParticleGroupDestroyed(group);
}

void DestructionListener::SayGoodbye(b2ParticleSystem *system, int index) {
    level->levelManager.leaking(*system, index);
}

const b2ParticleColor Level::k_ParticleColors[] = {
    b2ParticleColor(0xff, 0x00, 0x00, 0xff), // red
    b2ParticleColor(0x00, 0xff, 0x00, 0xff), // green
    b2ParticleColor(0x00, 0x00, 0xff, 0xff), // blue
    b2ParticleColor(0xff, 0x8c, 0x00, 0xff), // orange
    b2ParticleColor(0x00, 0xce, 0xd1, 0xff), // turquoise
    b2ParticleColor(0xff, 0x00, 0xff, 0xff), // magenta
    b2ParticleColor(0xff, 0xd7, 0x00, 0xff), // gold
    b2ParticleColor(0x00, 0xff, 0xff, 0xff), // cyan
};
const uint32 Level::k_ParticleColorsCount =
    B2_ARRAY_SIZE(Level::k_ParticleColors);

Level::Level() {
    b2ParticleSystemDef particleSystemDef;
    b2Vec2 gravity {0, -static_cast<float>(gravityIntensity)};
    gravity.Set(0, -gravityIntensity);
    m_world = new b2World(gravity);
    m_particleSystem = m_world->CreateParticleSystem(&particleSystemDef);
    m_textLine = 30;
    m_mouseJoint = nullptr;
    m_pointCount = 0;

    m_destructionListener.level = this;
    m_world->SetDestructionListener(&m_destructionListener);
    m_world->SetContactListener(this);
    m_world->SetDebugDraw(&m_debugDraw);

    m_particleSystem->SetGravityScale(0.4);
    m_particleSystem->SetDensity(1.2);
    m_particleSystem->SetRadius(particleRadius);

    m_stepCount = 0;

    m_mouseWorld = b2Vec2_zero;
    m_mouseTracing = false;
    m_mouseTracerPosition = b2Vec2_zero;
    m_mouseTracerVelocity = b2Vec2_zero;

    b2BodyDef bodyDef;
    m_groundBody = m_world->CreateBody(&bodyDef);

    memset(&m_maxProfile, 0, sizeof(b2Profile));
    memset(&m_totalProfile, 0, sizeof(b2Profile));
}

Level::~Level() {
    // By deleting the world, we delete the bomb, mouse joint, etc.
    delete m_world;
    m_world = nullptr;
}

void Level::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
    const b2Manifold *manifold {contact->GetManifold()};

    if (manifold->pointCount == 0) {
        return;
    }

    b2Fixture *fixtureA = contact->GetFixtureA();
    b2Fixture *fixtureB = contact->GetFixtureB();

    b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
    b2GetPointStates(state1, state2, oldManifold, manifold);

    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);

    for (int i = 0; i < manifold->pointCount && m_pointCount < k_maxContactPoints; i++)
    {
        ContactPoint* cp = m_points + m_pointCount;
        cp->fixtureA = fixtureA;
        cp->fixtureB = fixtureB;
        cp->position = worldManifold.points[i];
        cp->normal = worldManifold.normal;
        cp->state = state2[i];
        cp->normalImpulse = manifold->points[i].normalImpulse;
        cp->tangentImpulse = manifold->points[i].tangentImpulse;
        cp->separation = worldManifold.separations[i];
        m_pointCount++;
    }
}

void Level::BeginContact(b2ParticleSystem *system, b2ParticleContact *contact) {
    WaterContactListener::contact(*system, contact->GetIndexA(), contact->GetIndexB());
}

void Level::DrawTitle(const char *string) {
    m_debugDraw.DrawString(5, DRAW_STRING_NEW_LINE, string);
    m_textLine = 2 * DRAW_STRING_NEW_LINE;
}

class QueryCallback: public b2QueryCallback {
public:
    QueryCallback(const b2Vec2 &point) {
        m_point = point;
        m_fixture = nullptr;
    }

    bool ReportFixture(b2Fixture *fixture) {
        b2Body *body = fixture->GetBody();
        if (body->GetType() == b2_dynamicBody) {
            bool inside = fixture->TestPoint(m_point);
            if (inside) {
                m_fixture = fixture;

                // We are done, terminate the query.
                return false;
            }
        }

        // Continue the query.
        return true;
    }

    b2Vec2 m_point;
    b2Fixture *m_fixture;
};

class QueryCallback2: public b2QueryCallback {
public:
    QueryCallback2(b2ParticleSystem *particleSystem,
                   const b2Shape *shape, const b2Vec2 &velocity) {
        m_particleSystem = particleSystem;
        m_shape = shape;
        m_velocity = velocity;
    }

    bool ReportFixture(b2Fixture *fixture) {
        return false;
    }

    bool ReportParticle(const b2ParticleSystem *particleSystem, int index) {
        if (particleSystem != m_particleSystem) {
            return false;
        }

        b2Transform xf;
        xf.SetIdentity();
        b2Vec2 p {m_particleSystem->GetPositionBuffer()[index]};
        if (m_shape->TestPoint(xf, p)) {
            b2Vec2 &v {m_particleSystem->GetVelocityBuffer()[index]};
            v = m_velocity;
        }
        return true;
    }

    b2ParticleSystem *m_particleSystem;
    const b2Shape *m_shape;
    b2Vec2 m_velocity;
};

// Set gravity when cursor key pressed
void Level::KeyboardSpecial(int key) {
    b2Vec2 gravity;
    switch (key) {
        case GLUT_KEY_LEFT: {
            gravity = {-1, 0};
            break;
        }
        case GLUT_KEY_RIGHT: {
            gravity = {1, 0};
            break;
        }
        case GLUT_KEY_UP: {
            gravity = {0, 1};
            break;
        }
        case GLUT_KEY_DOWN: {
            gravity = {0, -1};
            break;
        }
    }
    gravity *= gravityIntensity;
    m_world->SetGravity(gravity);
}

void Level::MouseDown(const b2Vec2 &p) {
    m_mouseWorld = p;
    m_mouseTracing = true;
    m_mouseTracerPosition = p;
    m_mouseTracerVelocity = b2Vec2_zero;

    if (m_mouseJoint != nullptr) {
        return;
    }

    // Make a small box.
    b2AABB aabb;
    b2Vec2 d;
    d.Set(0.001, 0.001);
    aabb.lowerBound = p - d;
    aabb.upperBound = p + d;

    // Query the world for overlapping shapes.
    QueryCallback callback(p);
    m_world->QueryAABB(&callback, aabb);

    if (callback.m_fixture) {
        b2Body *body {callback.m_fixture->GetBody()};
        b2MouseJointDef md;
        md.bodyA = m_groundBody;
        md.bodyB = body;
        md.target = p;
        md.maxForce = 1000.0 * body->GetMass();
        m_mouseJoint = static_cast<b2MouseJoint*>(m_world->CreateJoint(&md));
        body->SetAwake(true);
    }
}

void Level::MouseUp(const b2Vec2 &p) {
    m_mouseTracing = false;

    if (m_mouseJoint) {
        m_world->DestroyJoint(m_mouseJoint);
        m_mouseJoint = nullptr;
    }
}

void Level::MouseMove(const b2Vec2& p) {
    m_mouseWorld = p;

    if (m_mouseJoint) {
        m_mouseJoint->SetTarget(p);
    }
}

void Level::Step(Settings *settings) {
    float timeStep {settings->hz > 0 ? 1 / settings->hz : 0};

    if (settings->pause) {
        if (settings->singleStep) {
            settings->singleStep = 0;
        } else {
            timeStep = 0;
        }

        m_debugDraw.DrawString(5, m_textLine, "****PAUSED****");
        m_textLine += DRAW_STRING_NEW_LINE;
    }

    uint32 flags {};
    flags += settings->drawShapes			* b2Draw::e_shapeBit;
    flags += settings->drawParticles		* b2Draw::e_particleBit;
    m_debugDraw.SetFlags(flags);

    m_world->SetAllowSleeping(settings->enableSleep > 0);
    m_world->SetWarmStarting(settings->enableWarmStarting > 0);
    m_world->SetContinuousPhysics(settings->enableContinuous > 0);
    m_world->SetSubStepping(settings->enableSubStepping > 0);
    m_particleSystem->SetStrictContactCheck(settings->strictContacts > 0);

    m_pointCount = 0;

    b2Timer timer;
    m_world->Step(timeStep,
        settings->velocityIterations,
        settings->positionIterations,
        settings->particleIterations);
    settings->stepTimeOut = timer.GetMilliseconds();

    m_world->DrawDebugData();

    if (timeStep > 0) {
        m_stepCount++;
    }

    // Track maximum profile times
    {
        const b2Profile &p {m_world->GetProfile()};
        m_maxProfile.step = b2Max(m_maxProfile.step, p.step);
        m_maxProfile.collide = b2Max(m_maxProfile.collide, p.collide);
        m_maxProfile.solve = b2Max(m_maxProfile.solve, p.solve);
        m_maxProfile.solveInit = b2Max(m_maxProfile.solveInit, p.solveInit);
        m_maxProfile.solveVelocity = b2Max(m_maxProfile.solveVelocity, p.solveVelocity);
        m_maxProfile.solvePosition = b2Max(m_maxProfile.solvePosition, p.solvePosition);
        m_maxProfile.solveTOI = b2Max(m_maxProfile.solveTOI, p.solveTOI);
        m_maxProfile.broadphase = b2Max(m_maxProfile.broadphase, p.broadphase);

        m_totalProfile.step += p.step;
        m_totalProfile.collide += p.collide;
        m_totalProfile.solve += p.solve;
        m_totalProfile.solveInit += p.solveInit;
        m_totalProfile.solveVelocity += p.solveVelocity;
        m_totalProfile.solvePosition += p.solvePosition;
        m_totalProfile.solveTOI += p.solveTOI;
        m_totalProfile.broadphase += p.broadphase;
    }

    if (m_mouseTracing && !m_mouseJoint) {
        float delay = {0.1};
        b2Vec2 acceleration = 2 / delay * (1 / delay * (m_mouseWorld - m_mouseTracerPosition) - m_mouseTracerVelocity);
        m_mouseTracerVelocity += timeStep * acceleration;
        m_mouseTracerPosition += timeStep * m_mouseTracerVelocity;
        b2CircleShape shape;
        shape.m_p = m_mouseTracerPosition;
        shape.m_radius = 2 * GetDefaultViewZoom();
        QueryCallback2 callback(m_particleSystem, &shape, m_mouseTracerVelocity);
        b2AABB aabb;
        b2Transform xf;
        xf.SetIdentity();
        shape.ComputeAABB(&aabb, xf, 0);
        m_world->QueryAABB(&callback, aabb);
    }

    if (m_mouseJoint) {
        b2Vec2 p1 {m_mouseJoint->GetAnchorB()};
        b2Vec2 p2 {m_mouseJoint->GetTarget()};

        b2Color c;
        c.Set(0, 1, 0);
        m_debugDraw.DrawPoint(p1, 4, c);
        m_debugDraw.DrawPoint(p2, 4, c);

        c.Set(0.8, 0.8, 0.8);
        m_debugDraw.DrawSegment(p1, p2, c);
    }

    // Remove the particles that went out of the screen
    {
        b2PolygonShape shape;
        shape.SetAsBox(25, 25);

        b2Transform killLocation;

        killLocation.Set({50, 25}, 0);
        m_particleSystem->DestroyParticlesInShape(shape, killLocation);
        killLocation.Set({-50, 25}, 0);
        m_particleSystem->DestroyParticlesInShape(shape, killLocation);
        killLocation.Set({0, 75}, 0);
        m_particleSystem->DestroyParticlesInShape(shape, killLocation);
        killLocation.Set({0, -25}, 0);
        m_particleSystem->DestroyParticlesInShape(shape, killLocation);
    }

    // Show score
    levelManager.showScore(m_debugDraw, m_textLine);
}

void Level::ShiftOrigin(const b2Vec2 &newOrigin) {
    m_world->ShiftOrigin(newOrigin);
}

float Level::GetDefaultViewZoom() const {
    return 1;
}

// Apply a preset range of colors to a particle group.
// A different color out of k_ParticleColors is applied to each
// particlesPerColor particles in the specified group.
// If particlesPerColor is 0, the particles in the group are divided into
// k_ParticleColorsCount equal sets of colored particles.
void Level::ColorParticleGroup(b2ParticleGroup *const group,
                              uint32 particlesPerColor) {
    b2Assert(group);
    b2ParticleColor *const colorBuffer {m_particleSystem->GetColorBuffer()};
    const int particleCount {group->GetParticleCount()};
    const int groupStart {group->GetBufferIndex()};
    const int groupEnd {particleCount + groupStart};
    const int colorCount {static_cast<int>(k_ParticleColorsCount)};
    if (!particlesPerColor) {
        particlesPerColor = particleCount / colorCount;
        if (!particlesPerColor) {
            particlesPerColor = 1;
        }
    }
    for (int i = groupStart; i < groupEnd; i++) {
        colorBuffer[i] = k_ParticleColors[i / particlesPerColor];
    }
}