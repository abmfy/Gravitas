#ifndef Level_h
#define Level_h

#include "Box2D/Box2D.h"
#include "Render.h"

#include "DestructionListener.h"
#include "LevelManager.h"

class Level;

using TestCreateFcn = Level*();

const int DRAW_STRING_NEW_LINE {25};

class Level: public b2ContactListener {
public:
	static const int gravityIntensity {100};
	static constexpr double particleRadius {0.1};

	static constexpr float hz {60};
	static const int velocityIterations {8};
	static const int positionIterations {3};
	static const int particleIterations;

	static const b2ParticleSystemDef defaultParticleSystemDef;
	static const b2BodyDef defaultBodyDef;

	Level();
	virtual ~Level();

	void drawTitle();

	virtual void Step(int);
	virtual void KeyboardSpecial(int);

	virtual float GetDefaultViewZoom() const;

protected:
	int id;

	LevelManager levelManager;
	DestructionListener destructionListener {levelManager};

	b2World world {{0, -static_cast<float>(gravityIntensity)}};
	b2ParticleSystem *particleSystem {world.CreateParticleSystem(&defaultParticleSystemDef)};
	b2Body *groundBody {world.CreateBody(&defaultBodyDef)};

	DebugDraw debugDraw;
	int textLine {30};
};

#endif