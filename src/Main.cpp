#include <cstdio>
#include <algorithm>
#include <sstream>
#include <string>

#include "Render.h"
#include "Level.h"
#include "Arrow.h"
#include "FullscreenUI.h"
#include "ParticleParameter.h"
#include "glui/glui.h"

namespace TestMain {

namespace {
	int32 testIndex {};
	int32 testSelection {};
	int32 testCount {};
	TestEntry* entry;
	Level* level;
	Settings settings;
	int32 width {640};
	int32 height {640};
	int32 framePeriod {16};
	int32 mainWindow;
	float settingsHz {60};
	GLUI *glui = nullptr;
	float32 viewZoom {1};
	int tx, ty, tw, th;
	bool rMouseDown {};
	// State of the mouse on the previous call of Mouse().
	int32 previousMouseState {-1};
	b2Vec2 lastp;
	b2Vec2 extents;

	// Fullscreen UI object.
	FullscreenUI fullscreenUI;
	// Used to control the behavior of particle tests.
	ParticleParameter particleParameter;
}

// Set whether to restart the test on particle parameter changes.
// This parameter is re-enabled when the test changes.
void SetRestartOnParticleParameterChange(bool enable) {
	particleParameter.SetRestartOnChange(enable);
}

// Set the currently selected particle parameter value.  This value must
// match one of the values in TestMain::k_particleTypes or one of the values
// referenced by particleParameterDef passed to SetParticleParameters().
uint32 SetParticleParameterValue(uint32 value) {
	const int32 index {particleParameter.FindIndexByValue(value)};
	// If the particle type isn't found, so fallback to the first entry in the
	// parameter.
	particleParameter.Set(index >= 0 ? index : 0);
	return particleParameter.GetValue();
}

// Get the currently selected particle parameter value and enable particle
// parameter selection arrows on Android.
uint32 GetParticleParameterValue() {
	// Enable display of particle type selection arrows.
	fullscreenUI.SetParticleParameterSelectionEnabled(true);
	return particleParameter.GetValue();
}

// Override the default particle parameters for the test.
void SetParticleParameters(
	const ParticleParameter::Definition * const particleParameterDef,
	const uint32 particleParameterDefCount) {
	particleParameter.SetDefinition(particleParameterDef,
									particleParameterDefCount);
}

static void Resize(int32 w, int32 h) {
	width = w;
	height = h;

	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	glViewport(tx, ty, tw, th);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float32 ratio {th ? float32(tw) / float32(th) : 1};

	extents = ratio >= 1 ? b2Vec2(ratio * 25.0f, 25.0f) : b2Vec2(25.0f, 25.0f / ratio);
	extents *= viewZoom;

	b2Vec2 lower = settings.viewCenter - extents;
	b2Vec2 upper = settings.viewCenter + extents;

	// L/R/B/T
	LoadOrtho2DMatrix(lower.x, upper.x, lower.y, upper.y);

	if (fullscreenUI.GetEnabled()) {
		fullscreenUI.SetViewParameters(&settings.viewCenter, &extents);
	}
}

static b2Vec2 ConvertScreenToWorld(int32 x, int32 y) {
	float32 u = x / float32(tw);
	float32 v = (th - y) / float32(th);

	b2Vec2 lower = settings.viewCenter - extents;
	b2Vec2 upper = settings.viewCenter + extents;

	b2Vec2 p;
	p.x = (1.0f - u) * lower.x + u * upper.x;
	p.y = (1.0f - v) * lower.y + v * upper.y;
	return p;
}

// This is used to control the frame rate (60Hz).
static void Timer(int) {
	glutSetWindow(mainWindow);
	glutPostRedisplay();
	glutTimerFunc(framePeriod, Timer, 0);
}

static void SimulationLoop() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	settings.hz = settingsHz;

	// call this each frame, to function correctly with devices that may recreate
	// the GL Context without us asking for it
	Resize(width, height);

	level->Step(&settings);

	// Update the state of the particle parameter.
	bool restartTest;
	const bool changed = particleParameter.Changed(&restartTest);
	B2_NOT_USED(changed);


	if (fullscreenUI.GetEnabled()) {
		// Set framework settings options based on parameters
		const uint32 options {particleParameter.GetOptions()};

		settings.strictContacts 	= options &
			ParticleParameter::OptionStrictContacts;
		settings.drawContactPoints	= options &
			ParticleParameter::OptionDrawContactPoints;
		settings.drawContactNormals	= options &
			ParticleParameter::OptionDrawContactNormals;
		settings.drawContactImpulse	= options &
			ParticleParameter::OptionDrawContactImpulse;
		settings.drawFrictionImpulse = options &
			ParticleParameter::OptionDrawFrictionImpulse;
		settings.drawStats 			 = options &
			ParticleParameter::OptionDrawStats;
		settings.drawProfile		 = options &
			ParticleParameter::OptionDrawProfile;

		// The b2Draw based flags must be exactly 0 or 1 currently.
		settings.drawShapes 	= options &
			ParticleParameter::OptionDrawShapes ? 1 : 0;
		settings.drawParticles 	= options &
			ParticleParameter::OptionDrawParticles ? 1 : 0;
		settings.drawJoints		= options &
			ParticleParameter::OptionDrawJoints ? 1 : 0;
		settings.drawAABBs		= options &
			ParticleParameter::OptionDrawAABBs ? 1 : 0;
		settings.drawCOMs 		= options &
			ParticleParameter::OptionDrawCOMs ? 1 : 0;

		// Draw the full screen UI with
		// "test_name [: particle_parameter] / fps" at the bottom of the
		// screen.
		std::string msg {entry->name};
		if (fullscreenUI.GetParticleParameterSelectionEnabled()) {
			msg += " : ";
			msg += particleParameter.GetName();
		}

		std::stringstream ss;
		ss << int(1000.0f / ComputeFPS());
		msg += " / " + ss.str() + " fps";
		fullscreenUI.Draw(msg);
	}

	level->DrawTitle(entry->name);

	glutSwapBuffers();

	if (testSelection != testIndex || restartTest) {
		fullscreenUI.Reset();
		if (!restartTest) particleParameter.Reset();

		testIndex = testSelection;
		delete level;
		entry = g_testEntries + testIndex;
		level = entry->createFcn();
		viewZoom = level->GetDefaultViewZoom();
		settings.viewCenter.Set(0.0f, 20.0f * viewZoom);
		Resize(width, height);
	}

	// print world step time stats every 600 frames
	static int s_printCount {};
	static b2Stat st;
	st.Record(settings.stepTimeOut);

	const int STAT_PRINT_INTERVAL {600};
	if ( settings.printStepTimeStats && st.GetCount() == STAT_PRINT_INTERVAL ) {
		printf("World Step Time samples %i-%i: %fmin %fmax %favg (ms)\n",
			s_printCount*STAT_PRINT_INTERVAL,
			(s_printCount+1)*STAT_PRINT_INTERVAL-1,
			st.GetMin(), st.GetMax(), st.GetMean());
		st.Clear();
		s_printCount++;
	}
}

static void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 27: {
			// freeglut specific function
			glutLeaveMainLoop();
			exit(0);
			break;
		}

		// Press 'r' to reset.
		case 'r': {
			delete level;
			level = entry->createFcn();
			break;
		}

		case 'p': {
			settings.pause = !settings.pause;
			break;
		}

		// Press [ to prev test.
		case '[': {
			--testSelection;
			if (testSelection < 0) {
				testSelection = testCount - 1;
			}
			if (glui) {
				glui->sync_live();
			}
			break;
		}

		// Press ] to next test.
		case ']': {
			++testSelection;
			if (testSelection == testCount) {
				testSelection = 0;
			}
			if (glui) {
				glui->sync_live();
			}
			break;
		}

		// Press ~ to enable / disable the fullscreen UI.
		case '~': {
			fullscreenUI.SetEnabled(!fullscreenUI.GetEnabled());
			break;
		}

		default: {
			if (level) {
				level->Keyboard(key);
			}
		}
	}
}

static void KeyboardSpecial(int key, int x, int y) {
	if (level) {
		level->KeyboardSpecial(key);
	}
}

static void KeyboardUp(unsigned char key, int x, int y) {
	if (level) {
		level->KeyboardUp(key);
	}
}

static void Mouse(int32 button, int32 state, int32 x, int32 y) {
	// Use the mouse to move things around.
	if (button == GLUT_LEFT_BUTTON) {
		int mod {glutGetModifiers()};
		b2Vec2 p {ConvertScreenToWorld(x, y)};

		switch (fullscreenUI.Mouse(button, state, previousMouseState, p)) {
			case FullscreenUI::e_SelectionTestPrevious: {
				testSelection = std::max(0, testSelection - 1);
				break;
			}
			case FullscreenUI::e_SelectionTestNext: {
				if (g_testEntries[testSelection + 1].name) testSelection++;
				break;
			}
			case FullscreenUI::e_SelectionParameterPrevious: {
				particleParameter.Decrement();
				break;
			}
			case FullscreenUI::e_SelectionParameterNext: {
				particleParameter.Increment();
				break;
			}
			default: {
				break;
			}
		}

		if (state == GLUT_DOWN) {
			b2Vec2 p {ConvertScreenToWorld(x, y)};
			level->MouseDown(p);
		}

		if (state == GLUT_UP) {
			level->MouseUp(p);
		}
	} else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			lastp = ConvertScreenToWorld(x, y);
			rMouseDown = true;
		}

		if (state == GLUT_UP) {
			rMouseDown = false;
		}
	}
	previousMouseState = state;
}

static void MouseMotion(int32 x, int32 y) {
	b2Vec2 p = ConvertScreenToWorld(x, y);

	if (fullscreenUI.GetSelection() == FullscreenUI::e_SelectionNone) {
		level->MouseMove(p);
	}

	if (rMouseDown) {
		b2Vec2 diff {p - lastp};
		settings.viewCenter.x -= diff.x;
		settings.viewCenter.y -= diff.y;
		Resize(width, height);
		lastp = ConvertScreenToWorld(x, y);
	}
}

static void Restart(int) {
	delete level;
	entry = g_testEntries + testIndex;
	level = entry->createFcn();
	Resize(width, height);
}

static void Pause(int) {
	settings.pause = !settings.pause;
}

static void Exit(int code) {
	glutLeaveMainLoop();
	exit(code);
}

static void SingleStep(int) {
	settings.pause = 1;
	settings.singleStep = 1;
}

}  // namespace TestMain

int main(int argc, char** argv) {
	using namespace TestMain;

	testCount = 0;
	while (g_testEntries[testCount].createFcn != nullptr) {
		++testCount;
	}

	testIndex = b2Clamp(testIndex, 0, testCount-1);
	testSelection = testIndex;

	entry = g_testEntries + testIndex;
	if (entry && entry->createFcn) {
		level = entry->createFcn();
		testSelection = testIndex;
		testIndex = -1;
	}

	glutInit(&argc, argv);
	glutInitContextVersion(2, 0);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	mainWindow = glutCreateWindow("Gravitas");

	glutDisplayFunc(SimulationLoop);

	GLUI_Master.set_glutReshapeFunc(Resize);
	GLUI_Master.set_glutKeyboardFunc(Keyboard);
	GLUI_Master.set_glutSpecialFunc(KeyboardSpecial);
	GLUI_Master.set_glutMouseFunc(Mouse);

	glutMotionFunc(MouseMotion);

	glutKeyboardUpFunc(KeyboardUp);

	// Configure the fullscreen UI's viewport parameters.
	fullscreenUI.SetViewParameters(&settings.viewCenter, &extents);

	// Use a timer to control the frame rate.
	glutTimerFunc(framePeriod, Timer, 0);

	glutMainLoop();
}