#include <cstdio>

#include "Level.h"

#include "glui/glui.h"

#include "SaveManager.h"

Level::Level() {
    b2ParticleSystemDef particleSystemDef;
    b2Vec2 gravity {0, -static_cast<float>(gravityIntensity)};
    m_world = new b2World {gravity};
    m_particleSystem = m_world->CreateParticleSystem(&particleSystemDef);
    m_textLine = 30;

    m_world->SetDestructionListener(&destructionListener);
    m_world->SetContactListener(this);
    m_world->SetDebugDraw(&m_debugDraw);
    m_world->SetAllowSleeping(true);
    m_world->SetWarmStarting(true);
    m_world->SetContinuousPhysics(true);

    m_particleSystem->SetGravityScale(0.4);
    m_particleSystem->SetDensity(1.2);
    m_particleSystem->SetRadius(particleRadius);

    m_debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_particleBit);

    b2BodyDef bodyDef;
    m_groundBody = m_world->CreateBody(&bodyDef);
}

Level::~Level() {
    // By deleting the world, we delete the bomb, mouse joint, etc.
    delete m_world;
}

void Level::DrawTitle(const char *string) {
    m_debugDraw.DrawString(5, DRAW_STRING_NEW_LINE, string);
    m_textLine = 2 * DRAW_STRING_NEW_LINE;

    m_debugDraw.DrawString(5, 605, "<ESC> Pause    [R] Restart    [P] Previous level    [N] Next level");
    m_debugDraw.DrawString(5, 630, "<CURSOR> Control gravity");
}

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

void Level::Step(int paused) {
    if (paused) {
        m_debugDraw.DrawString(5, m_textLine, "****PAUSED**** <ESC> Resume    [Q] Quit game");
        m_textLine += DRAW_STRING_NEW_LINE;
        m_world->DrawDebugData();
        return;
    }

    m_world->Step(1 / hz,
        velocityIterations,
        positionIterations,
        particleIterations);

    m_world->DrawDebugData();

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

    // Save progress
    if (levelManager.didWin()) {
        SaveManager::getInstance().updateLevel(id + 1);
    }
}

float Level::GetDefaultViewZoom() const {
    return 1;
}