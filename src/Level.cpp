#include <cstdio>

#include "Level.h"
#include "Main.h"

#include "glui/glui.h"


void DestructionListener::SayGoodbye(b2Joint *joint) {
	if (level->m_mouseJoint == joint) {
		level->m_mouseJoint = nullptr;
	} else {
		level->JointDestroyed(joint);
	}
}

void DestructionListener::SayGoodbye(b2ParticleGroup* group) {
	level->ParticleGroupDestroyed(group);
}

void DestructionListener::SayGoodbye(b2ParticleSystem *system, int32 index) {
    level->score++;
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
	const b2ParticleSystemDef particleSystemDef;
	b2Vec2 gravity {0, -static_cast<float32>(gravityIntensity)};
	gravity.Set(0, -gravityIntensity);
	m_world = new b2World(gravity);
	m_particleSystem = m_world->CreateParticleSystem(&particleSystemDef);
	m_bomb = nullptr;
	m_textLine = 30;
	m_mouseJoint = nullptr;
	m_pointCount = 0;

	m_destructionListener.level = this;
	m_world->SetDestructionListener(&m_destructionListener);
	m_world->SetContactListener(this);
	m_world->SetDebugDraw(&m_debugDraw);

	m_particleSystem->SetGravityScale(0.4f);
	m_particleSystem->SetDensity(1.2f);
	m_particleSystem->SetRadius(particleRadius);

	m_bombSpawning = false;

	m_stepCount = 0;

	m_mouseWorld = b2Vec2_zero;
	m_mouseTracing = false;
	m_mouseTracerPosition = b2Vec2_zero;
	m_mouseTracerVelocity = b2Vec2_zero;

	b2BodyDef bodyDef;
	m_groundBody = m_world->CreateBody(&bodyDef);

	memset(&m_maxProfile, 0, sizeof(b2Profile));
	memset(&m_totalProfile, 0, sizeof(b2Profile));

	m_particleParameters = nullptr;
}

Level::~Level()
{
	// By deleting the world, we delete the bomb, mouse joint, etc.
	delete m_world;
	m_world = nullptr;
	RestoreParticleParameters();
}

void Level::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
	const b2Manifold* manifold = contact->GetManifold();

	if (manifold->pointCount == 0) {
		return;
	}

	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
	b2GetPointStates(state1, state2, oldManifold, manifold);

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);

	for (int32 i = 0; i < manifold->pointCount && m_pointCount < k_maxContactPoints; ++i)
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
		++m_pointCount;
	}
}

void Level::DrawTitle(const char *string) {
    m_debugDraw.DrawString(5, DRAW_STRING_NEW_LINE, string);
    m_textLine = 2 * DRAW_STRING_NEW_LINE;
}

class QueryCallback : public b2QueryCallback {
public:
	QueryCallback(const b2Vec2& point) {
		m_point = point;
		m_fixture = nullptr;
	}

	bool ReportFixture(b2Fixture* fixture) {
		b2Body* body = fixture->GetBody();
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
	b2Fixture* m_fixture;
};

class QueryCallback2 : public b2QueryCallback {
public:
	QueryCallback2(b2ParticleSystem* particleSystem,
				   const b2Shape* shape, const b2Vec2& velocity) {
		m_particleSystem = particleSystem;
		m_shape = shape;
		m_velocity = velocity;
	}

	bool ReportFixture(b2Fixture* fixture) {
		B2_NOT_USED(fixture);
		return false;
	}

	bool ReportParticle(const b2ParticleSystem* particleSystem, int32 index) {
		if (particleSystem != m_particleSystem)
			return false;

		b2Transform xf;
		xf.SetIdentity();
		b2Vec2 p = m_particleSystem->GetPositionBuffer()[index];
		if (m_shape->TestPoint(xf, p)) {
			b2Vec2& v = m_particleSystem->GetVelocityBuffer()[index];
			v = m_velocity;
		}
		return true;
	}

	b2ParticleSystem* m_particleSystem;
	const b2Shape* m_shape;
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

void Level::MouseDown(const b2Vec2& p) {
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
	d.Set(0.001f, 0.001f);
	aabb.lowerBound = p - d;
	aabb.upperBound = p + d;

	// Query the world for overlapping shapes.
	QueryCallback callback(p);
	m_world->QueryAABB(&callback, aabb);

	if (callback.m_fixture) {
		b2Body* body = callback.m_fixture->GetBody();
		b2MouseJointDef md;
		md.bodyA = m_groundBody;
		md.bodyB = body;
		md.target = p;
		md.maxForce = 1000.0f * body->GetMass();
		m_mouseJoint = (b2MouseJoint*)m_world->CreateJoint(&md);
		body->SetAwake(true);
	}
}

void Level::MouseUp(const b2Vec2& p)
{
	m_mouseTracing = false;

	if (m_mouseJoint)
	{
		m_world->DestroyJoint(m_mouseJoint);
		m_mouseJoint = nullptr;
	}
}

void Level::MouseMove(const b2Vec2& p)
{
	m_mouseWorld = p;

	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(p);
	}
}

void Level::Step(Settings* settings) {
	float32 timeStep = settings->hz > 0.0f ? 1.0f / settings->hz : float32(0.0f);

	if (settings->pause) {
		if (settings->singleStep) {
			settings->singleStep = 0;
		} else {
			timeStep = 0.0f;
		}

		m_debugDraw.DrawString(5, m_textLine, "****PAUSED****");
		m_textLine += DRAW_STRING_NEW_LINE;
	}

	uint32 flags {};
	flags += settings->drawShapes			* b2Draw::e_shapeBit;
	flags += settings->drawParticles		* b2Draw::e_particleBit;
	flags += settings->drawJoints			* b2Draw::e_jointBit;
	flags += settings->drawAABBs			* b2Draw::e_aabbBit;
	flags += settings->drawCOMs				* b2Draw::e_centerOfMassBit;
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

	if (timeStep > 0.0f) {
		++m_stepCount;
	}

	if (settings->drawStats) {
		int32 bodyCount = m_world->GetBodyCount();
		int32 contactCount = m_world->GetContactCount();
		int32 jointCount = m_world->GetJointCount();
		m_debugDraw.DrawString(5, m_textLine, "bodies/contacts/joints = %d/%d/%d", bodyCount, contactCount, jointCount);
		m_textLine += DRAW_STRING_NEW_LINE;

		int32 particleCount = m_particleSystem->GetParticleCount();
		int32 groupCount = m_particleSystem->GetParticleGroupCount();
		int32 pairCount = m_particleSystem->GetPairCount();
		int32 triadCount = m_particleSystem->GetTriadCount();
		m_debugDraw.DrawString(5, m_textLine, "particles/groups/pairs/triads = %d/%d/%d/%d", particleCount, groupCount, pairCount, triadCount);
		m_textLine += DRAW_STRING_NEW_LINE;

		int32 proxyCount = m_world->GetProxyCount();
		int32 height = m_world->GetTreeHeight();
		int32 balance = m_world->GetTreeBalance();
		float32 quality = m_world->GetTreeQuality();
		m_debugDraw.DrawString(5, m_textLine, "proxies/height/balance/quality = %d/%d/%d/%g", proxyCount, height, balance, quality);
		m_textLine += DRAW_STRING_NEW_LINE;
	}

	// Track maximum profile times
	{
		const b2Profile& p = m_world->GetProfile();
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

	if (settings->drawProfile) {
		const b2Profile& p = m_world->GetProfile();

		b2Profile aveProfile;
		memset(&aveProfile, 0, sizeof(b2Profile));
		if (m_stepCount > 0) {
			float32 scale {1.0f / m_stepCount};
			aveProfile.step = scale * m_totalProfile.step;
			aveProfile.collide = scale * m_totalProfile.collide;
			aveProfile.solve = scale * m_totalProfile.solve;
			aveProfile.solveInit = scale * m_totalProfile.solveInit;
			aveProfile.solveVelocity = scale * m_totalProfile.solveVelocity;
			aveProfile.solvePosition = scale * m_totalProfile.solvePosition;
			aveProfile.solveTOI = scale * m_totalProfile.solveTOI;
			aveProfile.broadphase = scale * m_totalProfile.broadphase;
		}

		m_debugDraw.DrawString(5, m_textLine, "step [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.step, aveProfile.step, m_maxProfile.step);
		m_textLine += DRAW_STRING_NEW_LINE;
		m_debugDraw.DrawString(5, m_textLine, "collide [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.collide, aveProfile.collide, m_maxProfile.collide);
		m_textLine += DRAW_STRING_NEW_LINE;
		m_debugDraw.DrawString(5, m_textLine, "solve [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solve, aveProfile.solve, m_maxProfile.solve);
		m_textLine += DRAW_STRING_NEW_LINE;
		m_debugDraw.DrawString(5, m_textLine, "solve init [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveInit, aveProfile.solveInit, m_maxProfile.solveInit);
		m_textLine += DRAW_STRING_NEW_LINE;
		m_debugDraw.DrawString(5, m_textLine, "solve velocity [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveVelocity, aveProfile.solveVelocity, m_maxProfile.solveVelocity);
		m_textLine += DRAW_STRING_NEW_LINE;
		m_debugDraw.DrawString(5, m_textLine, "solve position [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solvePosition, aveProfile.solvePosition, m_maxProfile.solvePosition);
		m_textLine += DRAW_STRING_NEW_LINE;
		m_debugDraw.DrawString(5, m_textLine, "solveTOI [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveTOI, aveProfile.solveTOI, m_maxProfile.solveTOI);
		m_textLine += DRAW_STRING_NEW_LINE;
		m_debugDraw.DrawString(5, m_textLine, "broad-phase [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.broadphase, aveProfile.broadphase, m_maxProfile.broadphase);
		m_textLine += DRAW_STRING_NEW_LINE;
	}

	if (m_mouseTracing && !m_mouseJoint) {
		float32 delay = {0.1};
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
		c.Set(0.0f, 1.0f, 0.0f);
		m_debugDraw.DrawPoint(p1, 4.0f, c);
		m_debugDraw.DrawPoint(p2, 4.0f, c);

		c.Set(0.8f, 0.8f, 0.8f);
		m_debugDraw.DrawSegment(p1, p2, c);
	}

	if (m_bombSpawning) {
		b2Color c;
		c.Set(0.0f, 0.0f, 1.0f);
		m_debugDraw.DrawPoint(m_bombSpawnPoint, 4.0f, c);

		c.Set(0.8f, 0.8f, 0.8f);
		m_debugDraw.DrawSegment(m_mouseWorld, m_bombSpawnPoint, c);
	}

	if (settings->drawContactPoints) {
		const float32 k_impulseScale = 0.1f;
		const float32 k_axisScale = 0.3f;

		for (int32 i = 0; i < m_pointCount; ++i)
		{
			ContactPoint* point = m_points + i;

			if (point->state == b2_addState)
			{
				// Add
				m_debugDraw.DrawPoint(point->position, 10.0f, b2Color(0.3f, 0.95f, 0.3f));
			}
			else if (point->state == b2_persistState)
			{
				// Persist
				m_debugDraw.DrawPoint(point->position, 5.0f, b2Color(0.3f, 0.3f, 0.95f));
			}

			if (settings->drawContactNormals)
			{
				b2Vec2 p1 = point->position;
				b2Vec2 p2 = p1 + k_axisScale * point->normal;
				m_debugDraw.DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.9f));
			}
			else if (settings->drawContactImpulse)
			{
				b2Vec2 p1 = point->position;
				b2Vec2 p2 = p1 + k_impulseScale * point->normalImpulse * point->normal;
				m_debugDraw.DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
			}

			if (settings->drawFrictionImpulse)
			{
				b2Vec2 tangent = b2Cross(point->normal, 1.0f);
				b2Vec2 p1 = point->position;
				b2Vec2 p2 = p1 + k_impulseScale * point->tangentImpulse * tangent;
				m_debugDraw.DrawSegment(p1, p2, b2Color(0.9f, 0.9f, 0.3f));
			}
		}
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
	{
		char str[64];
		sprintf(str, "Score: %d", score);
		m_debugDraw.DrawString(5, m_textLine, str);
		m_textLine += DRAW_STRING_NEW_LINE;
		if (score > threshold) {
			m_debugDraw.DrawString(5, m_textLine, "You win!");
			m_textLine += DRAW_STRING_NEW_LINE;
		}
	}
}

void Level::ShiftOrigin(const b2Vec2& newOrigin)
{
	m_world->ShiftOrigin(newOrigin);
}

float32 Level::GetDefaultViewZoom() const {
	return 1;
}

// Apply a preset range of colors to a particle group.
// A different color out of k_ParticleColors is applied to each
// particlesPerColor particles in the specified group.
// If particlesPerColor is 0, the particles in the group are divided into
// k_ParticleColorsCount equal sets of colored particles.
void Level::ColorParticleGroup(b2ParticleGroup * const group,
							  uint32 particlesPerColor)
{
	b2Assert(group);
	b2ParticleColor * const colorBuffer = m_particleSystem->GetColorBuffer();
	const int32 particleCount = group->GetParticleCount();
	const int32 groupStart = group->GetBufferIndex();
	const int32 groupEnd = particleCount + groupStart;
	const int32 colorCount = (int32)k_ParticleColorsCount;
	if (!particlesPerColor)
	{
		particlesPerColor = particleCount / colorCount;
		if (!particlesPerColor)
		{
			particlesPerColor = 1;
		}
	}
	for (int32 i = groupStart; i < groupEnd; i++)
	{
		colorBuffer[i] = k_ParticleColors[i / particlesPerColor];
	}
}


// Remove particle parameters matching "filterMask" from the set of
// particle parameters available for this test.
void Level::InitializeParticleParameters(const uint32 filterMask)
{
	const uint32 defaultNumValues =
		ParticleParameter::k_defaultDefinition[0].numValues;
	const ParticleParameter::Value * const defaultValues =
		ParticleParameter::k_defaultDefinition[0].values;
	m_particleParameters = new ParticleParameter::Value[defaultNumValues];

	// Disable selection of wall and barrier particle types.
	uint32 numValues = 0;
	for (uint32 i = 0; i < defaultNumValues; i++)
	{
		if (defaultValues[i].value & filterMask)
		{
			continue;
		}
		memcpy(&m_particleParameters[numValues], &defaultValues[i],
			   sizeof(defaultValues[0]));
		numValues++;
	}
	m_particleParameterDef.values = m_particleParameters;
	m_particleParameterDef.numValues = numValues;
	// TestMain::SetParticleParameters(&m_particleParameterDef, 1);
}

// Restore default particle parameters.
void Level::RestoreParticleParameters()
{
	if (m_particleParameters)
	{
		// TestMain::SetParticleParameters(
			// ParticleParameter::k_defaultDefinition, 1);
		delete [] m_particleParameters;
		m_particleParameters = nullptr;
	}
}
