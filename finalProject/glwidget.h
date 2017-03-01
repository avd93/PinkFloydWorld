#ifndef __GLWIDGET__INCLUDE__
#define __GLWIDGET__INCLUDE__

#include <QGLWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <QTimer>
#include <glm/glm.hpp>

#define GLM_FORCE_RADIANS

using glm::mat3;
using glm::mat4;
using glm::vec3;
using namespace std;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core { 
    Q_OBJECT

    public:
        GLWidget(QWidget *parent=0);
        ~GLWidget();

        GLuint loadShaders(const char* vertf, const char* fragf);
    protected:
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();

        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);

    // Part 2 - add an animate slot
    public slots:
        void animate();

    private:
        void initializeCube();
        void renderFunWall(mat4 transform);
        void renderWall(mat4 transform);
        void renderRow(mat4 transform);
        void renderBrick(mat4 transform);
        void renderCube(mat4 transform);

        void renderFunWall(mat4 transform,GLint textureID);
        void renderWall(mat4 transform,GLint textureID);
        void renderRow(mat4 transform,GLint textureID);
        void renderBrick(mat4 transform,GLint textureID);
        void renderCube(mat4 transform,GLint textureID);

        void renderClassroom(mat4 transform);
        void renderTeacherDesk(mat4 transform);
        void renderPretendDrawer(mat4 transform);
        void renderStudentDesk(mat4 transform);
        void renderLeg(mat4 transform,GLuint textureID);

        void renderMeatGrinder(mat4 transform);
        void renderFunnel(mat4 transform);

        void renderHammer(mat4 transform);
        void renderHead(mat4 transform, int h);
        void renderFace(mat4 transform);

        GLuint cubeProg;
        GLuint cubeVao;
        GLint cubeProjMatrixLoc;
        GLint cubeViewMatrixLoc;
        GLint cubeModelMatrixLoc;

        void initializeTexture();
        void renderTexture(mat4 transform,GLuint textureID);
        GLuint setTexture(QString imagePath);
        GLuint texProg;
        GLuint texVao;
        GLint texProjMatrixLoc;
        GLint texViewMatrixLoc;
        GLint texModelMatrixLoc;
        GLuint albumArt;
        GLuint concrete;
        GLuint metal;
        GLuint handle;
        GLuint wall;
        GLuint wall2;
        GLuint wall3;
        GLuint wall4;
        GLuint red;
        GLuint copper;
        GLuint blue;
        GLuint chalkboard;
        GLuint desk;
        GLuint desk2;

        mat4 texProjMatrix;
        mat4 texViewMatrix;
        mat4 texModelMatrix;

        void initializeGrid();
        void renderGrid();

        GLuint gridProg;
        GLuint gridVao;
        GLint gridProjMatrixLoc;
        GLint gridViewMatrixLoc;
        GLint gridModelMatrixLoc;

        GLint lightPosition;
        GLint ambientLoc;
        GLint diffuseLoc;
        GLint lightColor;
        GLint specIntensity;

        GLint texLightPosition;
        GLint texAmbientLoc;
        GLint texDiffuseLoc;
        GLint texLightColor;
        GLint texSpecIntensity;

        mat4 projMatrix;
        mat4 viewMatrix;
        mat4 modelMatrix;

        // Part 1 - Add two mat4 variables for pitch and yaw.
        // Also add two float variables for the pitch and yaw angles.
        float pitch;
        float yaw;

        mat4 pitchMatrix;
        mat4 yawMatrix;
        mat4 orientation;

        // Part 2 - Add a QTimer variable for our render loop.
        QTimer *timer;
        float time;

        mat4 quatSlerp;
//        std::vector<vec3> rotations;

        void animateObjects(std::vector<vec3> rotations);
        // Part 3 - Add state variables for keeping track
        //          of which movement keys are being pressed
        //        - Add two vec3 variables for position and velocity.
        //        - Add a variable for toggling fly mode
        bool forward;
        bool back;
        bool left;
        bool right;
        bool up;
        bool down;
        bool flymode;

        vec3 position;
        vec3 velocity;
        vec3 brickSize;
        vec3 cubeColor;
        float pos[2];

        GLuint colorBuffer;
        void setCubeColor(vec3 xColor,vec3 yColor,vec3 zColor);

        int width;
        int height;
        int maxNumBricks;
        int numRows;
        float spacing;
        float walk;
        float pos1;
        float pos2;

        glm::vec2 lastPt;
        void updateView();
};

#endif
