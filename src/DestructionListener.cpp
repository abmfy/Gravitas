#include "DestructionListener.h"

#include "LevelManager.h"

DestructionListener::DestructionListener(LevelManager &manager): manager {manager} {}

void DestructionListener::SayGoodbye(b2ParticleSystem *system, int id) {
    manager.leaking(*system, id);
}