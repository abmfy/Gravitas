#ifndef LevelManager_H
#define LevelManager_H

#include "Box2D/Box2D.h"

#include "Render.h"

// Manage a level's status
class LevelManager {
    int score {};
    int threshold {};
public:
    void incrementScore();
    void decrementScore();
    void setThreshold(int);
    int getScore() const;
    void showScore(DebugDraw&, int&) const;
    bool didWin() const;
    bool didLose() const;

    void leaking(b2ParticleSystem&, int);
};

#endif