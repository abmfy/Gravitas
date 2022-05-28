#include <cstdio>

#include "Level.h"

#include "glui/glui.h"

#include "SaveManager.h"

const int Level::particleIterations {b2CalculateParticleIterations(10, 0.04, 1 / hz)};

const b2ParticleSystemDef Level::defaultParticleSystemDef {};
const b2BodyDef Level::defaultBodyDef {};

Level::Level() {
    

    textLine = 30;

    world.SetDestructionListener(&destructionListener);
    world.SetContactListener(this);
    world.SetDebugDraw(&debugDraw);
    world.SetAllowSleeping(true);
    world.SetWarmStarting(true);
    world.SetContinuousPhysics(true);

    particleSystem->SetGravityScale(0.4);
    particleSystem->SetDensity(1.2);
    particleSystem->SetRadius(particleRadius);

    debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_particleBit);
}

Level::~Level() {}

void Level::drawTitle() {
    char buf[16];
    sprintf(buf, "Level %d", id);
    debugDraw.DrawString(5, DRAW_STRING_NEW_LINE, buf);
    textLine = 2 * DRAW_STRING_NEW_LINE;

    debugDraw.DrawString(5, 605, "<ESC> Pause    [R] Restart    [P] Previous level    [N] Next level");
    debugDraw.DrawString(5, 630, "<CURSOR> Control gravity");
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
    world.SetGravity(gravity);
}

void Level::Step(int paused) {
    if (paused) {
        debugDraw.DrawString(5, textLine, "****PAUSED**** <ESC> Resume    [Q] Quit game");
        textLine += DRAW_STRING_NEW_LINE;
        world.DrawDebugData();
        drawTitle();
        return;
    }

    world.Step(1 / hz,
        velocityIterations,
        positionIterations,
        particleIterations);

    world.DrawDebugData();
    drawTitle();

    // Remove the particles that went out of the screen
    {
        b2PolygonShape shape;
        shape.SetAsBox(25, 25);

        b2Transform killLocation;

        killLocation.Set({50, 25}, 0);
        particleSystem->DestroyParticlesInShape(shape, killLocation);
        killLocation.Set({-50, 25}, 0);
        particleSystem->DestroyParticlesInShape(shape, killLocation);
        killLocation.Set({0, 75}, 0);
        particleSystem->DestroyParticlesInShape(shape, killLocation);
        killLocation.Set({0, -25}, 0);
        particleSystem->DestroyParticlesInShape(shape, killLocation);
    }

    // Show score
    levelManager.showScore(debugDraw, textLine);

    // Save progress
    if (levelManager.didWin()) {
        SaveManager::getInstance().updateLevel(id + 1);
    }
}

float Level::GetDefaultViewZoom() const {
    return 1;
}