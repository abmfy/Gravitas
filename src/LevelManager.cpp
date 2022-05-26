#include <cstdio>

#include "Box2D/Box2D.h"

#include "Render.h"

#include "LevelManager.h"

#include "Water.h"
using Water::Type::poison, Water::Type::water;

// Pixels of a line
const int newLine {25};

void LevelManager::incrementScore() {
    score++;
}

void LevelManager::decrementScore() {
    score--;
}

void LevelManager::setThreshold(int newThreshold) {
    threshold = newThreshold;
}

int LevelManager::getScore() const {
    return score;
}

void LevelManager::showScore(DebugDraw &draw, int &line) const {
    char str[64];
    sprintf(str, "Score: %d %s", getScore(), didWin() ? "You win!" : didLose() ? "You lose!" : "");
    draw.DrawString(5, line, str);
    line += newLine;
}

bool LevelManager::didWin() const {
    return score > threshold;
}

bool LevelManager::didLose() const {
    return score < 0;
}

void LevelManager::leaking(b2ParticleSystem &system, int index) {
    void *data {system.GetUserDataBuffer()[index]};
    if (data) {
        Water::Type &type {*static_cast<Water::Type*>(data)};
        switch (type) {
            case water: {
                incrementScore();
                break;
            }
            case poison: {
                decrementScore();
                break;
            }
        }
    }
}