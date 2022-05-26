#ifndef Level_h
#define Level_h

#include "Box2D/Box2D.h"
#include "Render.h"

#include "LevelManager.h"

class Level;
struct Settings;

using TestCreateFcn = Level*();

const int RAND_LIMIT {32767};
const int DRAW_STRING_NEW_LINE {25};

/// Test settings. Some can be controlled in the GUI.
struct Settings {
	Settings() {
		viewCenter.Set(0, 20);
		hz = 60;
		velocityIterations = 8;
		positionIterations = 3;
		// Particle iterations are needed for numerical stability in particle
		// simulations with small particles and relatively high gravity.
		// b2CalculateParticleIterations helps to determine the number.
		particleIterations = b2CalculateParticleIterations(10, 0.04, 1 / hz);
		drawShapes = 1;
		drawParticles = 1;
		enableWarmStarting = 1;
		enableContinuous = 1;
		enableSubStepping = 0;
		enableSleep = 1;
		pause = 0;
		singleStep = 0;
		stepTimeOut = 0;
		strictContacts = 0;
	}

	b2Vec2 viewCenter;
	float hz;
	int velocityIterations;
	int positionIterations;
	int particleIterations;
	int drawShapes;
	int drawParticles;
	int enableWarmStarting;
	int enableContinuous;
	int enableSubStepping;
	int enableSleep;
	int pause;
	int singleStep;
	int strictContacts;

	/// Measures how long did the world step took, in ms
	float stepTimeOut;
};

struct TestEntry {
	const char *name;
	TestCreateFcn *createFcn;
};

extern TestEntry g_testEntries[];
// This is called when a joint in the world is implicitly destroyed
// because an attached body is destroyed. This gives us a chance to
// nullify the mouse joint.
class DestructionListener: public b2DestructionListener {
public:
	void SayGoodbye(b2Fixture*) override {}
	void SayGoodbye(b2Joint*) override;
	void SayGoodbye(b2ParticleGroup*) override;

	void SayGoodbye(b2ParticleSystem*, int) override;

	Level *level;
};

const int k_maxContactPoints {2048};

struct ContactPoint {
	b2Fixture *fixtureA;
	b2Fixture *fixtureB;
	b2Vec2 normal;
	b2Vec2 position;
	b2PointState state;
	float normalImpulse;
	float tangentImpulse;
	float separation;
};

class Level: public b2ContactListener {
public:
	const int gravityIntensity {100};
	const double particleRadius {0.1};

	Level();
	virtual ~Level();

	int id;

    void DrawTitle(const char*);
	virtual void Step(Settings*);
	virtual void Keyboard(unsigned char) {}
	virtual void KeyboardSpecial(int);
	virtual void KeyboardUp(unsigned char) {}
	virtual void MouseDown(const b2Vec2&);
	virtual void MouseUp(const b2Vec2&);
	virtual void MouseMove(const b2Vec2&);

	// Let derived tests know that a joint was destroyed.
	virtual void JointDestroyed(b2Joint*) {}

	// Let derived tests know that a particle group was destroyed.
	virtual void ParticleGroupDestroyed(b2ParticleGroup*) {}

	// Callbacks for derived classes.
	virtual void BeginContact(b2Contact*) override {}
	virtual void EndContact(b2Contact*) override {}
	virtual void PreSolve(b2Contact*, const b2Manifold*) override;
	virtual void PostSolve(b2Contact*, const b2ContactImpulse*) override {}

	void ShiftOrigin(const b2Vec2&);
	virtual float GetDefaultViewZoom() const;

	// Apply a preset range of colors to a particle group.
	// A different color out of k_ParticleColors is applied to each
	// particlesPerColor particles in the specified group.
	// If particlesPerColor is 0, the particles in the group are divided into
	// k_ParticleColorsCount equal sets of colored particles.
	void ColorParticleGroup(b2ParticleGroup *const, uint32);

protected:
	friend class DestructionListener;
	friend class BoundaryListener;
	friend class ContactListener;

	LevelManager levelManager;

	b2Body *m_groundBody;
	b2AABB m_worldAABB;
	ContactPoint m_points[k_maxContactPoints];
	int m_pointCount;
	DestructionListener m_destructionListener;
	DebugDraw m_debugDraw;
	int m_textLine;
	b2World *m_world;
	b2ParticleSystem *m_particleSystem;
	b2MouseJoint *m_mouseJoint;
	b2Vec2 m_mouseWorld;
	bool m_mouseTracing;
	b2Vec2 m_mouseTracerPosition;
	b2Vec2 m_mouseTracerVelocity;

	static const b2ParticleColor k_ParticleColors[];
	static const uint32 k_ParticleColorsCount;
};

#endif