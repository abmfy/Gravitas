#ifndef Level_h
#define Level_h

#include "Box2D/Box2D.h"
#include "Render.h"

#include "DestructionListener.h"
#include "LevelManager.h"

class Level;

using TestCreateFcn = Level*();

const int DRAW_STRING_NEW_LINE {25};

struct TestEntry {
	const char *name;
	TestCreateFcn *createFcn;
};

extern TestEntry g_testEntries[];

class Level: public b2ContactListener {
public:
	static const int gravityIntensity {100};
	static constexpr double particleRadius {0.1};

	static constexpr float hz {60};
	static const int velocityIterations {8};
	static const int positionIterations {3};
	const int particleIterations {b2CalculateParticleIterations(10, 0.04, 1 / hz)};

	int id;

	Level();
	virtual ~Level();

    void DrawTitle(const char*);
	virtual void Step(int);
	virtual void KeyboardSpecial(int);

	virtual float GetDefaultViewZoom() const;

protected:
	LevelManager levelManager;
	DestructionListener destructionListener {levelManager};

	b2Body *m_groundBody;
	DebugDraw m_debugDraw;
	int m_textLine;
	b2World *m_world;
	b2ParticleSystem *m_particleSystem;
};

#endif