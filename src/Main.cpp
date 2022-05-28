#include <cstdio>
#include <memory>
using std::unique_ptr;

#include "Level.h"
#include "Levels.h"
#include "Render.h"
#include "SaveManager.h"

#include "glui/glui.h"

int nowLevel;
unique_ptr<Level> level;
int paused;
b2Vec2 viewCenter {0, 20};
int width {640};
int height {640};
int framePeriod {16};
int mainWindow;
float viewZoom {1};
int tx, ty, tw, th;
b2Vec2 extents;

void Resize(int w, int h) {
    width = w;
    height = h;

    GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
    glViewport(tx, ty, tw, th);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ratio {th ? static_cast<float>(tw) / th : 1};

    extents = ratio >= 1 ? b2Vec2(ratio * 25, 25) : b2Vec2(25, 25 / ratio);
    extents *= viewZoom;

    b2Vec2 lower {viewCenter - extents};
    b2Vec2 upper {viewCenter + extents};

    // L/R/B/T
    LoadOrtho2DMatrix(lower.x, upper.x, lower.y, upper.y);
}

// This is used to control the frame rate (60Hz).
void Timer(int) {
    glutSetWindow(mainWindow);
    glutPostRedisplay();
    glutTimerFunc(framePeriod, Timer, 0);
}

void selectLevel(int id) {
    level = levelFactories[id]->create();
    viewZoom = level->GetDefaultViewZoom();
    viewCenter.Set(0, 20 * viewZoom);
    Resize(width, height);
}

void SimulationLoop() {
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // call this each frame, to function correctly with devices that may recreate
    // the GL Context without us asking for it
    Resize(width, height);

    level->Step(paused);

    glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y) {
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
            selectLevel(nowLevel);
            break;
        }

        case 'p':
        case 'P': {
            if (nowLevel) {
                selectLevel(--nowLevel);
            }
            break;
        }

        case 'n':
        case 'N': {
            if (nowLevel < SaveManager::getInstance().getLevel() - 1) {
                selectLevel(++nowLevel);
            }
            break;
        }
    }
}

void KeyboardSpecial(int key, int x, int y) {
    if (level) {
        level->KeyboardSpecial(key);
    }
}

int main(int argc, char **argv) {
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

    selectLevel(0);

    glutMainLoop();
}