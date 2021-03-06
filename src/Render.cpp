#include <cstdio>
#include <cstdarg>

#include "Render.h"

#include "GL/freeglut.h"

float currentscale {1};	// amount of pixels that corresponds to one world unit, needed to use glPointSize correctly

void LoadOrtho2DMatrix(double left, double right, double bottom, double top) {
	int h {glutGet(GLUT_WINDOW_HEIGHT)};
	currentscale = float(h / (top - bottom));

#if USE_GL_KIT
	const GLKMatrix4 matrix = GLKMatrix4MakeOrtho(left, right, bottom, top,
													-1.0, 1.0);
	glLoadMatrixf((const GLfloat*)&matrix);
#else
	// L/R/B/T
	gluOrtho2D(left, right, bottom, top);
#endif // USE_GL_KIT
}

void DebugDraw::DrawPolygon(const b2Vec2 *vertices, int vertexCount, const b2Color &color) {
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINE_LOOP);
	for (int i {}; i < vertexCount; i++) {
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();
}

void DebugDraw::DrawFlatPolygon(const b2Vec2 *vertices, int vertexCount, const b2Color &color) {
	glColor4f(color.r, color.g, color.b, 1);
	glBegin(GL_TRIANGLE_FAN);
	for (int i {}; i < vertexCount; i++) {
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();
}

void DebugDraw::DrawSolidPolygon(const b2Vec2 *vertices, int vertexCount, const b2Color &color) {
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.5 * color.r, 0.5 * color.g, 0.5 * color.b, 0.5);
	glBegin(GL_TRIANGLE_FAN);
	for (int i {}; i < vertexCount; i++) {
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();
	glDisable(GL_BLEND);

	glColor4f(color.r, color.g, color.b, 1);
	glBegin(GL_LINE_LOOP);
	for (int i {}; i < vertexCount; i++) {
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();
}

void DebugDraw::DrawCircle(const b2Vec2 &center, float radius, const b2Color &color) {
	const float k_segments {16};
	const float k_increment {2 * b2_pi / k_segments};
	float theta {};
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINE_LOOP);
	for (int i {}; i < k_segments; i++) {
		b2Vec2 v {center + radius * b2Vec2(cosf(theta), sinf(theta))};
		glVertex2f(v.x, v.y);
		theta += k_increment;
	}
	glEnd();
}

float smoothstep(float x) {
	return x * x * (3 - 2 * x);
}

void DebugDraw::DrawParticles(const b2Vec2 *centers, float radius, const b2ParticleColor *colors, int count) {
	static unsigned int particle_texture {};

	if (!particle_texture ||
			!glIsTexture(particle_texture)) // Android deletes textures upon sleep etc.
	{
		// generate a "gaussian blob" texture procedurally
		glGenTextures(1, &particle_texture);
		b2Assert(particle_texture);
		const int TSIZE {64};
		unsigned char tex[TSIZE][TSIZE][4];
		for (int y {}; y < TSIZE; y++)
		{
			for (int x {}; x < TSIZE; x++)
			{
				float fx {(x + 0.5f) / TSIZE * 2 - 1};
				float fy {(y + 0.5f) / TSIZE * 2 - 1};
				float dist {sqrtf(fx * fx + fy * fy)};
				unsigned char intensity = static_cast<unsigned char>(dist <= 1 ? smoothstep(1 - dist) * 255 : 0);
				tex[y][x][0] = tex[y][x][1] = tex[y][x][2] = 128;
				tex[y][x][3] = intensity;
			}
		}
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, particle_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TSIZE, TSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
		glDisable(GL_TEXTURE_2D);

		glEnable(GL_POINT_SMOOTH);
	}

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, particle_texture);

	glEnable(GL_POINT_SPRITE);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

	const float particle_size_multiplier {3};  // because of falloff
	glPointSize(radius * currentscale * particle_size_multiplier);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, &centers[0].x);
	if (colors) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0].r);
	} else {
		glColor4f(1, 1, 1, 1);
	}

	glDrawArrays(GL_POINTS, 0, count);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (colors) {
		glDisableClientState(GL_COLOR_ARRAY);
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void DebugDraw::DrawSolidCircle(const b2Vec2 &center, float radius, const b2Vec2 &axis, const b2Color &color) {
	const float k_segments {16};
	const float k_increment {2.0f * b2_pi / k_segments};
	float theta {};
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.5 * color.r, 0.5 * color.g, 0.5 * color.b, 0.5);
	glBegin(GL_TRIANGLE_FAN);
	for (int i {}; i < k_segments; i++)
	{
		b2Vec2 v {center + radius * b2Vec2(cosf(theta), sinf(theta))};
		glVertex2f(v.x, v.y);
		theta += k_increment;
	}
	glEnd();
	glDisable(GL_BLEND);

	theta = 0;
	glColor4f(color.r, color.g, color.b, 1);
	glBegin(GL_LINE_LOOP);
	for (int i {}; i < k_segments; i++)
	{
		b2Vec2 v {center + radius * b2Vec2(cosf(theta), sinf(theta))};
		glVertex2f(v.x, v.y);
		theta += k_increment;
	}
	glEnd();

	b2Vec2 p {center + radius * axis};
	glBegin(GL_LINES);
	glVertex2f(center.x, center.y);
	glVertex2f(p.x, p.y);
	glEnd();
}

void DebugDraw::DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color) {
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINES);
	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p2.y);
	glEnd();
}

void DebugDraw::DrawTransform(const b2Transform &xf) {
	b2Vec2 p1 {xf.p}, p2;
	const float k_axisScale {0.4};
	glBegin(GL_LINES);

	glColor3f(1, 0, 0);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	glVertex2f(p2.x, p2.y);

	glColor3f(0, 1, 0);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	glVertex2f(p2.x, p2.y);

	glEnd();
}

void DebugDraw::DrawPoint(const b2Vec2 &p, float size, const b2Color &color) {
	glPointSize(size);
	glBegin(GL_POINTS);
	glColor3f(color.r, color.g, color.b);
	glVertex2f(p.x, p.y);
	glEnd();
	glPointSize(1.0f);
}

void DebugDraw::DrawString(int x, int y, const char *string, ...) {
	char buffer[128];

	va_list arg;
	va_start(arg, string);
	vsprintf(buffer, string, arg);
	va_end(arg);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	LoadOrtho2DMatrix(0, w, h, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.9, 0.6, 0.6);
	glRasterPos2i(x, y);
	int length {static_cast<int>(strlen(buffer))};
	for (int i {}; i < length; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void DebugDraw::DrawString(const b2Vec2& p, const char *string, ...) {
	char buffer[128];

	va_list arg;
	va_start(arg, string);
	vsprintf(buffer, string, arg);
	va_end(arg);

	glColor3f(0.5, 0.9, 0.5);
	glRasterPos2f(p.x, p.y);

	int length {static_cast<int>(strlen(buffer))};
	for (int i {}; i < length; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, buffer[i]);
	}

	glPopMatrix();
}

void DebugDraw::DrawAABB(b2AABB *aabb, const b2Color &c) {
	glColor3f(c.r, c.g, c.b);
	glBegin(GL_LINE_LOOP);
	glVertex2f(aabb->lowerBound.x, aabb->lowerBound.y);
	glVertex2f(aabb->upperBound.x, aabb->lowerBound.y);
	glVertex2f(aabb->upperBound.x, aabb->upperBound.y);
	glVertex2f(aabb->lowerBound.x, aabb->upperBound.y);
	glEnd();
}