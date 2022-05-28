#include <cstdio>
#include <algorithm>
#include <sstream>
#include <string>

#include "Render.h"
#include "Level.h"
#include "glui/glui.h"

#include "SaveManager.h"

namespace TestMain {

namespace {
    int testIndex;
    int testSelection;
    int testCount;
    TestEntry *entry;
    Level *level;
    int paused;
    b2Vec2 viewCenter {0, 20};
    int width {640};
    int height {640};
    int framePeriod {16};
    int mainWindow;
    GLUI *glui;
    float viewZoom {1};
    int tx, ty, tw, th;
    b2Vec2 extents;
}

static void Resize(int w, int h) {
    width = w;
    height = h;

    GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
    glViewport(tx, ty, tw, th);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio {th ? static_cast<float>(tw) / static_cast<float>(th) : 1};

    extents = ratio >= 1 ? b2Vec2(ratio * 25, 25) : b2Vec2(25, 25 / ratio);
    extents *= viewZoom;

    b2Vec2 lower {viewCenter - extents};
    b2Vec2 upper {viewCenter + extents};

    // L/R/B/T
    LoadOrtho2DMatrix(lower.x, upper.x, lower.y, upper.y);
}

// This is used to control the frame rate (60Hz).
static void Timer(int) {
    glutSetWindow(mainWindow);
    glutPostRedisplay();
    glutTimerFunc(framePeriod, Timer, 0);
}

static void SimulationLoop() {
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // call this each frame, to function correctly with devices that may recreate
    // the GL Context without us asking for it
    Resize(width, height);

    level->Step(paused);

    // level->DrawTitle(entry->name);

    glutSwapBuffers();

    if (testSelection != testIndex) {
        testIndex = testSelection;
        delete level;
        entry = g_testEntries + testIndex;
        level = entry->createFcn();
        viewZoom = level->GetDefaultViewZoom();
        viewCenter.Set(0, 20 * viewZoom);
        Resize(width, height);
    }
}

static void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: {
            paused = !paused;
            break;
        }

        case 'q':
        case 'Q': {
            if (paused) {
                glutLeaveMainLoop();
                exit(0);
            }
            break;
        }

        // Press 'r' to reset.
        case 'r':
        case 'R': {
            delete level;
            level = entry->createFcn();
            break;
        }

        case 'p':
        case 'P': {
            if (testSelection) {
                testSelection--;
            }
            if (glui) {
                glui->sync_live();
            }
            break;
        }

        case 'n':
        case 'N': {
            if (testSelection < SaveManager::getInstance().getLevel() - 1) {
                testSelection++;
            }
            if (glui) {
                glui->sync_live();
            }
            break;
        }
    }
}

static void KeyboardSpecial(int key, int x, int y) {
    if (level) {
        level->KeyboardSpecial(key);
    }
}

}  // namespace TestMain

int main(int argc, char **argv) {
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

    // Use a timer to control the frame rate.
    glutTimerFunc(framePeriod, Timer, 0);

    glutMainLoop();
}