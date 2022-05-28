#ifndef DestructionListener_h
#define DestructionListener_h

#include "Box2D/Box2D.h"

#include "LevelManager.h"

class DestructionListener: public b2DestructionListener {
    LevelManager &manager;
public:
    DestructionListener(LevelManager&);
    // This LiquidFun API makes these functions as pure virtual,
    // which violates the Interface Segregation Principle.
    // However we really do not need them.
    void SayGoodbye(b2Fixture*) override {}
	void SayGoodbye(b2Joint*) override {};
	
    void SayGoodbye(b2ParticleSystem*, int) override;
};

#endif