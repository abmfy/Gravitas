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
    int testIndex {};
    int testSelection {};
    int testCount {};
    TestEntry* entry;
    Level* level;
    Settings settings;
    int width {640};
    int height {640};
    int framePeriod {16};
    int mainWindow;
    float settingsHz {60};
    GLUI *glui = nullptr;
    float viewZoom {1};
    int tx, ty, tw, th;
    bool rMouseDown {};
    // State of the mouse on the previous call of Mouse().
    int previousMouseState {-1};
    b2Vec2 lastp;
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

    b2Vec2 lower = settings.viewCenter - extents;
    b2Vec2 upper = settings.viewCenter + extents;

    // L/R/B/T
    LoadOrtho2DMatrix(lower.x, upper.x, lower.y, upper.y);
}

static b2Vec2 ConvertScreenToWorld(int x, int y) {
    float u {x / static_cast<float>(tw)};
    float v {(th - y) / static_cast<float>(th)};

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
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    settings.hz = settingsHz;

    // call this each frame, to function correctly with devices that may recreate
    // the GL Context without us asking for it
    Resize(width, height);

    level->Step(&settings);

    level->DrawTitle(entry->name);

    glutSwapBuffers();

    if (testSelection != testIndex) {
        testIndex = testSelection;
        delete level;
        entry = g_testEntries + testIndex;
        level = entry->createFcn();
        viewZoom = level->GetDefaultViewZoom();
        settings.viewCenter.Set(0, 20 * viewZoom);
        Resize(width, height);
    }
}

static void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: {
            settings.pause = !settings.pause;
            break;
        }

        case 'q':
        case 'Q': {
            if (settings.pause) {
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

        // Press A to prev test.
        case 'a':
        case 'A': {
            if (testSelection) {
                testSelection--;
            }
            if (glui) {
                glui->sync_live();
            }
            break;
        }

        // Press D to next test.
        case 'd':
        case 'D': {
            if (testSelection < SaveManager::getInstance().getLevel() - 1) {
                testSelection++;
            }
            if (glui) {
                glui->sync_live();
            }
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

static void Mouse(int button, int state, int x, int y) {
    // Use the mouse to move things around.
    if (button == GLUT_LEFT_BUTTON) {
        int mod {glutGetModifiers()};
        b2Vec2 p {ConvertScreenToWorld(x, y)};

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

static void MouseMotion(int x, int y) {
    b2Vec2 p = ConvertScreenToWorld(x, y);

    level->MouseMove(p);

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

    // Use a timer to control the frame rate.
    glutTimerFunc(framePeriod, Timer, 0);

    glutMainLoop();
}