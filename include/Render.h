#ifndef Render_h
#define Render_h

#include "Box2D/Box2D.h"

#if defined(__APPLE__)
#define USE_GL_KIT 1
#include <GLKit/GLKMath.h>
#else
#define USE_GL_KIT 0
#endif // defined(__APPLE__)

struct b2AABB;

// This class implements debug drawing callbacks that are invoked
// inside b2World::Step.
class DebugDraw: public b2Draw {
public:
	void DrawPolygon(const b2Vec2 *vertices, int vertexCount, const b2Color &color);

	void DrawFlatPolygon(const b2Vec2 *vertices, int vertexCount, const b2Color &color);

	void DrawSolidPolygon(const b2Vec2 *vertices, int vertexCount, const b2Color &color);

	void DrawCircle(const b2Vec2 &center, float radius, const b2Color &color);

	void DrawSolidCircle(const b2Vec2 &center, float radius, const b2Vec2 &axis, const b2Color &color);

	void DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color);

	void DrawParticles(const b2Vec2 *centers, float radius, const b2ParticleColor *colors, int count);

	void DrawTransform(const b2Transform &xf);

	void DrawPoint(const b2Vec2 &p, float size, const b2Color &color);

	void DrawString(int x, int y, const char *string, ...);

	void DrawString(const b2Vec2 &p, const char *string, ...);

	void DrawAABB(b2AABB *aabb, const b2Color &color);
};

void LoadOrtho2DMatrix(double left, double right, double bottom, double top);

#endif
