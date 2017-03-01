#include "glwidget.h"
#include <iostream>
#include <QOpenGLTexture>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QTextStream>
#include <QOpenGLTexture>
#include <QImage>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using glm::inverse;
using glm::vec2;
using glm::vec3;
using glm::mat3;
using glm::mat4;
using glm::perspective;
using glm::normalize;
using glm::length;
using glm::cross;
using glm::dot;
using glm::rotate;
using glm::value_ptr;
using glm::lookAt;
using glm::quat;

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) { 
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(16);

    quatSlerp = mat4(1.0f);

    time = 0;

    forward = false;
    back = false;
    left = false;
    right = false;
    up = false;
    down = false;
    flymode = false;
    position = vec3(-4.5,1,18);
    yaw = 0.f;
    // initialize where camera points
    pitch = -3*M_PI/32.f;
    pitchMatrix = glm::rotate(mat4(1.0),pitch,vec3(1,0,0));
    orientation = pitchMatrix;
    walk = 0;
    pos1 = 19.8;
    pos2 = -19.8;
}

GLWidget::~GLWidget() {

}

void GLWidget::initializeGrid() {
    glGenVertexArrays(1, &gridVao);
    glBindVertexArray(gridVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    vec3 pts[84];
    for(int i = -10; i <= 10; i++) {

        pts[2*(i+10)] = vec3(i, -.5f, 10);
        pts[2*(i+10)+1] = vec3(i, -.5f, -10);

        pts[2*(i+10)+42] = vec3(10,-.5f, i);
        pts[2*(i+10)+43] = vec3(-10,-.5f, i);
    }

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/grid_vert.glsl", ":/grid_frag.glsl");
    glUseProgram(program);
    gridProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    gridProjMatrixLoc = glGetUniformLocation(program, "projection");
    gridViewMatrixLoc = glGetUniformLocation(program, "view");
    gridModelMatrixLoc = glGetUniformLocation(program, "model");
}

void GLWidget::initializeCube() {

    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    //initialize cube color
//    cubeColor = vec3(1,0,0);
    vec3 pts[] = {
        // top
        vec3(1,1,1),    // 0
        vec3(1,1,-1),   // 1
        vec3(-1,1,-1),  // 2
        vec3(-1,1,1),   // 3

        // bottom
        vec3(1,-1,1),   // 4
        vec3(-1,-1,1),  // 5
        vec3(-1,-1,-1), // 6
        vec3(1,-1,-1),  // 7

        // front
        vec3(1,1,1),    // 8
        vec3(-1,1,1),   // 9
        vec3(-1,-1,1),  // 10
        vec3(1,-1,1),   // 11

        // back
        vec3(-1,-1,-1), // 12
        vec3(-1,1,-1),  // 13
        vec3(1,1,-1),   // 14
        vec3(1,-1,-1),  // 15

        // right
        vec3(1,-1,1),   // 16
        vec3(1,-1,-1),  // 17
        vec3(1,1,-1),   // 18
        vec3(1,1,1),     // 19

        // left
        vec3(-1,-1,1),  // 20
        vec3(-1,1,1),   // 21
        vec3(-1,1,-1),  // 22
        vec3(-1,-1,-1) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

    vec3 colors[] = {
        // top
        vec3(.9,.9,1),
        vec3(.9,.9,1),
        vec3(.9,.9,1),
        vec3(.9,.9,1),

        // bottom
        vec3(.9,.9,1),
        vec3(.9,.9,1),
        vec3(.9,.9,1),
        vec3(.9,.9,1),

        // front
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // back
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),
        vec3(1,1,1),

        // right
        vec3(.3f,0,.5),
        vec3(.3f,0,.5),
        vec3(.3f,0,.5),
        vec3(.3f,0,.5),

        // left
        vec3(.3f,0,.5),
        vec3(.3f,0,.5),
        vec3(.3f,0,.5),
        vec3(.3f,0,.5)
    };

    //Normal
    vec3 normals[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),    // 1
        vec3(0,1,0),    // 2
        vec3(0,1,0),    // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),   // 5
        vec3(0,-1,0),   // 6
        vec3(0,-1,0),   // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),    // 9
        vec3(0,0,1),   // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1),  // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),  // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),   // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),   // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),  // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0)   // 23
    };

    vec2 uvs[] = {
        // top
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // bottom
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // front
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // back
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // right
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // left
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0)

    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/vert.glsl", ":/frag.glsl");
    glUseProgram(program);
    cubeProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    GLint uvIndex = glGetAttribLocation(program, "uv");
    glEnableVertexAttribArray(uvIndex);
    glVertexAttribPointer(uvIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

    cubeProjMatrixLoc = glGetUniformLocation(program, "projection");
    cubeViewMatrixLoc = glGetUniformLocation(program, "view");
    cubeModelMatrixLoc = glGetUniformLocation(program, "model");

    // create a center light source
    lightPosition = glGetUniformLocation(program, "lightPos");
    glUniform3f(lightPosition, 0, 10, 0);
    ambientLoc = glGetUniformLocation(program, "ambient");
    diffuseLoc = glGetUniformLocation(program, "diffuse");
    lightColor = glGetUniformLocation(program, "lightColor");
    glUniform3f(lightColor, 1, 1, 1);
    specIntensity = glGetUniformLocation(program, "specIntensity");
}

void GLWidget::initializeTexture(){
    // Create a new Vertex Array Object on the GPU which
    // saves the attribute layout of our vertices.
    glGenVertexArrays(1, &texVao);
    glBindVertexArray(texVao);

    // Create a buffer on the GPU for position data
    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint normalBuffer;
    glGenBuffers(1, &normalBuffer);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(1,1,1),    // 0
        vec3(1,1,-1),   // 1
        vec3(-1,1,-1),  // 2
        vec3(-1,1,1),   // 3

        // bottom
        vec3(1,-1,1),   // 4
        vec3(-1,-1,1),  // 5
        vec3(-1,-1,-1), // 6
        vec3(1,-1,-1),  // 7

        // front
        vec3(1,1,1),    // 8
        vec3(-1,1,1),   // 9
        vec3(-1,-1,1),  // 10
        vec3(1,-1,1),   // 11

        // back
        vec3(-1,-1,-1), // 12
        vec3(-1,1,-1),  // 13
        vec3(1,1,-1),   // 14
        vec3(1,-1,-1),  // 15

        // right
        vec3(1,-1,1),   // 16
        vec3(1,-1,-1),  // 17
        vec3(1,1,-1),   // 18
        vec3(1,1,1),     // 19

        // left
        vec3(-1,-1,1),  // 20
        vec3(-1,1,1),   // 21
        vec3(-1,1,-1),  // 22
        vec3(-1,-1,-1) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .5;
    }

//    vec3 colors[] = {
//        // top
//        vec3(.9,.9,1),
//        vec3(.9,.9,1),
//        vec3(.9,.9,1),
//        vec3(.9,.9,1),

//        // bottom
//        vec3(.9,.9,1),
//        vec3(.9,.9,1),
//        vec3(.9,.9,1),
//        vec3(.9,.9,1),

//        // front
//        vec3(1,1,1),
//        vec3(1,1,1),
//        vec3(1,1,1),
//        vec3(1,1,1),

//        // back
//        vec3(1,1,1),
//        vec3(1,1,1),
//        vec3(1,1,1),
//        vec3(1,1,1),

//        // right
//        vec3(.3f,0,.5),
//        vec3(.3f,0,.5),
//        vec3(.3f,0,.5),
//        vec3(.3f,0,.5),

//        // left
//        vec3(.3f,0,.5),
//        vec3(.3f,0,.5),
//        vec3(.3f,0,.5),
//        vec3(.3f,0,.5)
//    };

    //Normal
    vec3 normals[] = {
        // top
        vec3(0,1,0),    // 0
        vec3(0,1,0),    // 1
        vec3(0,1,0),    // 2
        vec3(0,1,0),    // 3

        // bottom
        vec3(0,-1,0),   // 4
        vec3(0,-1,0),   // 5
        vec3(0,-1,0),   // 6
        vec3(0,-1,0),   // 7

        // front
        vec3(0,0,1),    // 8
        vec3(0,0,1),    // 9
        vec3(0,0,1),   // 10
        vec3(0,0,1),   // 11

        // back
        vec3(0,0,-1),  // 12
        vec3(0,0,-1),  // 13
        vec3(0,0,-1),  // 14
        vec3(0,0,-1),  // 15

        // right
        vec3(1,0,0),   // 16
        vec3(1,0,0),   // 17
        vec3(1,0,0),   // 18
        vec3(1,0,0),   // 19

        // left
        vec3(-1,0,0),  // 20
        vec3(-1,0,0),  // 21
        vec3(-1,0,0),  // 22
        vec3(-1,0,0)   // 23
    };

    vec2 uvs[] = {
        // top
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // bottom
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // front
        vec2(1,0),
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),

        // back
        vec2(1,1),
        vec2(1,0),
        vec2(0,0),
        vec2(0,1),


        // right
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),
        vec2(0,0),

        // left
        vec2(1,1),
        vec2(1,0),
        vec2(0,0),
        vec2(0,1)
    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

//    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // create distinct buffers for each texture,
    // so the picture data is pushed once per image

    // push wall_1
    glGenTextures(1, &albumArt);
    glBindTexture(GL_TEXTURE_2D, albumArt);
    albumArt = setTexture("C:/Graphics/origin/program3/images/wall_1.jpg");
    // push concrete
    glGenTextures(1, &concrete);
    glBindTexture(GL_TEXTURE_2D, concrete);
    concrete = setTexture("C:/Graphics/origin/program3/images/concrete.jpg");
    // push iron
    glGenTextures(1, &metal);
    glBindTexture(GL_TEXTURE_2D, metal);
    metal = setTexture("C:/Graphics/origin/program3/images/Metallic-Silver.jpg");
    //push handle
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    handle = setTexture("C:/Graphics/origin/program3/images/handle.jpg");
    //push red
    glGenTextures(1, &red);
    glBindTexture(GL_TEXTURE_2D, red);
    red = setTexture("C:/Graphics/origin/program3/images/red.jpg");
    //push blue
    glGenTextures(1, &blue);
    glBindTexture(GL_TEXTURE_2D, blue);
    blue = setTexture("C:/Graphics/origin/program3/images/blue.jpg");
    //push copper
    glGenTextures(1, &copper);
    glBindTexture(GL_TEXTURE_2D, copper);
    copper = setTexture("C:/Graphics/origin/program3/images/copper.jpg");
    //push wall
    glGenTextures(1, &wall);
    glBindTexture(GL_TEXTURE_2D, wall);
    wall = setTexture("C:/Graphics/origin/program3/images/wall.jpg");
    //push wall2
    glGenTextures(1, &wall2);
    glBindTexture(GL_TEXTURE_2D, wall2);
    wall2 = setTexture("C:/Graphics/origin/program3/images/wall_2.jpg");
    //push wall3
    glGenTextures(1, &wall3);
    glBindTexture(GL_TEXTURE_2D, wall3);
    wall3 = setTexture("C:/Graphics/origin/program3/images/wall_3.jpg");
    //push wall4
    glGenTextures(1, &wall4);
    glBindTexture(GL_TEXTURE_2D, wall4);
    wall4 = setTexture("C:/Graphics/origin/program3/images/clouds.jpg");
    //push chalkboard
    glGenTextures(1, &chalkboard);
    glBindTexture(GL_TEXTURE_2D, chalkboard);
    chalkboard = setTexture("C:/Graphics/origin/program3/images/chalkboard.jpg");
    //push desk
    glGenTextures(1, &desk);
    glBindTexture(GL_TEXTURE_2D, desk);
    desk = setTexture("C:/Graphics/origin/program3/images/desk.jpg");
    //push desk2
    glGenTextures(1, &desk2);
    glBindTexture(GL_TEXTURE_2D, desk2);
    desk2 = setTexture("C:/Graphics/origin/program3/images/desk2.jpg");

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    GLuint program = loadShaders(":/tex_vert.glsl", ":/tex_frag.glsl");
    glUseProgram(program);
    texProg = program;

    GLint texLoc = glGetUniformLocation(program, "tex");
    glUniform1i(texLoc, 0);
    //    glBindTexture(GL_TEXTURE_2D, textureObject);

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    GLint uvIndex = glGetAttribLocation(program, "uv");
    glEnableVertexAttribArray(uvIndex);
    glVertexAttribPointer(uvIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

    texProjMatrixLoc = glGetUniformLocation(program, "projection");
    texViewMatrixLoc = glGetUniformLocation(program, "view");
    texModelMatrixLoc = glGetUniformLocation(program, "model");

    // create a center light source
    texLightPosition = glGetUniformLocation(program, "lightPos");
    glUniform3f(texLightPosition, 0, 10, 0);
    texAmbientLoc = glGetUniformLocation(program, "ambient");
    texDiffuseLoc = glGetUniformLocation(program, "diffuse");
    texLightColor = glGetUniformLocation(program, "lightColor");
    texSpecIntensity = glGetUniformLocation(program, "specIntensity");
    glUniform3f(texLightColor, 1, 1, 1);
}

GLuint GLWidget::setTexture(QString imagePath){
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    QImage img = QImage(imagePath);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,img.width(),img.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,img.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureID;
}

void GLWidget::setCubeColor(vec3 xColor,vec3 yColor,vec3 zColor){
    vec3 colors[24];
    for(int i = 0; i < 8; i++){
        colors[i] = xColor;
    }
    for(int i = 8; i < 16; i++){
        colors[i] = yColor;
    }
    for(int i = 16; i < 24; i++){
        colors[i] = zColor;
    }
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
}

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

//    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearColor(0.05f, 0.01f, 0.15f, 0.05f);
    glPointSize(4.0f);

    glEnable(GL_DEPTH_TEST);
    GLuint restart = 0xFFFFFFFF;
    glPrimitiveRestartIndex(restart);
    glEnable(GL_PRIMITIVE_RESTART);

    initializeCube();
    initializeGrid();
    initializeTexture();

    viewMatrix = mat4(1.0f);
    modelMatrix = mat4(1.0f);

    //    texViewMatrix = mat4(1.0f);
    //    texModelMatrix = mat4(1.0f);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(cubeModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(gridProg);
    glUniformMatrix4fv(gridViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(gridModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(texProg);
    glUniformMatrix4fv(texViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(texModelMatrixLoc, 1, false, value_ptr(modelMatrix));
}

void GLWidget::resizeGL(int w, int h) {
    width = w;
    height = h;

    float aspect = (float)w/h;

    projMatrix = perspective(45.0f, aspect, .01f, 100.0f);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(gridProg);
    glUniformMatrix4fv(gridProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(texProg);
    glUniformMatrix4fv(texProjMatrixLoc, 1, false, value_ptr(projMatrix));
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderGrid();
    brickSize = vec3(1,.5,.5);
    spacing = .1*brickSize.x;
    maxNumBricks = 15;
    numRows = 6;
    mat4 translate;
    mat4 scale;
    mat4 rot;

    // set lighting for wall objects
    glUseProgram(cubeProg);
    glUniform1f(ambientLoc, 0.3);
    glUniform1f(diffuseLoc, 0.5);
    glUniform1f(specIntensity,5);

    // paint back wals
    translate = glm::translate(mat4(1.0),vec3(9,-.2,-15));
    renderWall(translate,wall4);

    translate = glm::translate(mat4(1.0),vec3(9,-.2,-6));
    renderWall(translate);

    maxNumBricks = 9;
    numRows = 8;
    translate = glm::translate(mat4(1.0),vec3(-9,-.2,-6));
    renderWall(translate,wall4);

    maxNumBricks = 8;
    translate = glm::translate(mat4(1.0),vec3(-14,-.2,-11));
    rot = glm::rotate(mat4(1.0),(float)M_PI/2,vec3(0,1,0));
    renderWall(translate*rot,wall4);

    maxNumBricks = 19;
    translate = glm::translate(mat4(1.0),vec3(-9,-.2,-19));
    renderWall(translate);

//    //render red cube
//    setCubeColor(vec3(1,0,0),vec3(1,0,0),vec3(1,0,0));
//    renderCube(glm::translate(mat4(1.0),vec3(2,2,2)));

    // paint classroom
    translate = glm::translate(mat4(1.0),vec3(0,0,19.75));
    renderClassroom(translate);

    // paint funwall gears
    numRows = 4;
    maxNumBricks = 5;
    std::vector<vec3>rotations3;
    rotations3.push_back(vec3(0, 0, 0));
    rotations3.push_back(vec3(0, M_PI, 0));
    rotations3.push_back(vec3(0,2*M_PI,0));
    animateObjects(rotations3);
    translate = glm::translate(mat4(1.0),vec3(7,0,-10));
    renderFunWall(translate*quatSlerp,red);

    rotations3.push_back(vec3(0, 0, 0));
    rotations3.push_back(vec3(0, -M_PI, 0));
    rotations3.push_back(vec3(0,-2*M_PI,0));
    animateObjects(rotations3);
    numRows = 6;
    maxNumBricks = 3;
    translate = glm::translate(mat4(1.0),vec3(11,0,-8));
    renderFunWall(translate*quatSlerp,blue);

    rotations3.push_back(vec3(0, 0, 0));
    rotations3.push_back(vec3(0, M_PI, 0));
    rotations3.push_back(vec3(0,2*M_PI,0));
    animateObjects(rotations3);
    numRows = 7;
    maxNumBricks = 3;
    translate = glm::translate(mat4(1.0),vec3(10,0,-12));
    renderFunWall(translate*quatSlerp,copper);

    // paint MEAT GRINDER
    translate = glm::translate(mat4(1.0),vec3(15,0,-18));
    renderMeatGrinder(translate);

    // textured wall objects
    glUseProgram(texProg);

    // paint pinkFloyd's brick
    glUniform1f(texAmbientLoc, .3);
    glUniform1f(texDiffuseLoc, 1);
    glUniform1f(texSpecIntensity,1);
    translate = glm::translate(mat4(1.0),vec3(0,1,-5));
    scale = glm::scale(mat4(1.0),vec3(4,3,.5));
    renderTexture(translate*scale,albumArt);

    // paint concrete floor
    glUniform1f(texAmbientLoc, 0.2);
    glUniform1f(texDiffuseLoc, 0.1);
    glUniform1f(texSpecIntensity,120);
    translate = glm::translate(mat4(1.0),vec3(0,-.5,0));
    scale = glm::scale(mat4(1.0),vec3(40,.1,40));
    renderTexture(translate*scale,concrete);

    // paint hammers, stationary set
    std::vector<vec3>rotations1;
    rotations1.push_back(vec3(0,0,0));
    rotations1.push_back(vec3(0,0,M_PI/2));
    rotations1.push_back(vec3(0,0,0));
    animateObjects(rotations1);
    for(int i = 0; i < 2; i++){
        translate = glm::translate(mat4(1.0),vec3(19.8,1.5,3.5 + 2*i));
        renderHammer(translate*quatSlerp);
        translate = glm::translate(mat4(1.0),vec3(-19.8,1.5,3.5 + 2*i));
        rot = glm::rotate(mat4(1.0),(float)M_PI,vec3(0,1,0));
        renderHammer(translate*rot*quatSlerp);
    }

    // paint hammers, WALKING set
    std::vector<vec3>rotations2;
    rotations2.push_back(vec3(0, 0, 0));
    rotations2.push_back(vec3(0, 0, -M_PI/8));
    rotations2.push_back(vec3(0,0,0));
    animateObjects(rotations2);
    pos[0]=(1-walk)*pos1+walk*pos2;
    pos[1]= (1-walk)*pos2+walk*pos1;
    for(int i = 0; i < 3; i++){
        scale = glm::scale(mat4(1.0),vec3(.8,.6,.8));
        translate = glm::translate(mat4(1.0),vec3(pos[0],.75,-3 + 2*i));
        renderHammer(translate*scale*quatSlerp);
        translate = glm::translate(mat4(1.0),vec3(pos[1],.75,-2 + 2*i));
        rot = glm::rotate(mat4(1.0),(float)M_PI,vec3(0,1,0));
        renderHammer(translate*scale*rot*quatSlerp);
//        cout<<"walk: "<<walk<<endl;
//        cout<<"pos[0]: "<<pos[0]<<endl;
//        cout<<"pos[1]: "<<pos[1]<<endl;
    }

}

void GLWidget::renderFunWall(mat4 transform){
    //    float wallWidth = maxNumBricks*brickSize.x + (maxNumBricks-1)*spacing;
    //render front
    renderWall(transform);
    //render 1st cross
    mat4 rotate= glm:: rotate(mat4(1.0), 45.f, glm::vec3(0,1,0));
    renderWall(transform*rotate);
    //    render 2nd cross
    rotate= glm::rotate(mat4(1.0), -45.f, glm::vec3(0,1,0));
    renderWall(transform*rotate);
}

void GLWidget::renderWall(mat4 transform){
    for(int i = 0; i < numRows; i++){
        if(i%2==0){
            //first and even rows have the max row length
            mat4 translate = glm::translate(mat4(1.0),vec3(0,i*(brickSize.y + spacing),0));
            renderRow(transform*translate);
        }else{
            //in between rows (odd) have max-1 row length
            maxNumBricks--;
            mat4 translate = glm::translate(mat4(1.0),vec3(0,i*(brickSize.y + spacing),0));
            renderRow(transform*translate);
            //reset numBricks
            maxNumBricks++;
        }
    }
}

void GLWidget::renderRow(mat4 transform){
    float wallWidth = maxNumBricks*brickSize.x + (maxNumBricks-1)*spacing;
    for(int i = 0; i < maxNumBricks; i++){
        mat4 scale = glm::scale(mat4(1.0), brickSize);
        //the PERFECT translation to keep the wall PERFECTLY centered for all wall sizes
        mat4 translate = glm::translate(mat4(1.0),vec3(i*(brickSize.x+spacing)+.5*(brickSize.x-wallWidth),0,0));
        renderCube(transform*translate*scale);
    }
}

void GLWidget::renderBrick(mat4 transform){
    mat4 scale = glm::scale(mat4(1.0),vec3(1,.5,.5));
}

void GLWidget::renderCube(mat4 transform) {
    glUseProgram(cubeProg);
    glBindVertexArray(cubeVao);
    mat4 temp = modelMatrix*transform;
    glUniformMatrix4fv(cubeModelMatrixLoc, 1, false, value_ptr(temp));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::renderGrid() {
    glUseProgram(gridProg);
    glBindVertexArray(gridVao);
    glDrawArrays(GL_LINES, 0, 84);
}

void GLWidget::renderClassroom(mat4 transform){
    mat4 translate;
    mat4 scale;
    mat4 rot;

    // back wall
    translate = glm::translate(mat4(1.0),vec3(0,3.5,0));
    scale = glm::scale(mat4(1.0),vec3(38,8,.5));
    renderCube(transform*translate*scale);

    // front wall
    translate = glm::translate(mat4(1.0),vec3(-5,3.5,-10));
    scale = glm::scale(mat4(1.0),vec3(28.52,8,.5));
    renderCube(transform*translate*scale);

    // left wall
    translate = glm::translate(mat4(1.0),vec3(-19,3.5,-5));
    scale = glm::scale(mat4(1.0),vec3(10,8,.5));
    rot = glm::rotate(mat4(1.0),(float)M_PI/2,vec3(0,1,0));
    renderCube(transform*translate*rot*scale);

    // middle wall
    translate = glm::translate(mat4(1.0),vec3(9,3.5,-6.25));
    scale = glm::scale(mat4(1.0),vec3(7,8,.5));
    rot = glm::rotate(mat4(1.0),(float)M_PI/2,vec3(0,1,0));
    renderCube(transform*translate*rot*scale);

    // right wall
    translate = glm::translate(mat4(1.0),vec3(19.25,3.5,-5.25));
    scale = glm::scale(mat4(1.0),vec3(10,8,.5));
    rot = glm::rotate(mat4(1.0),(float)M_PI/2,vec3(0,1,0));
    renderCube(transform*translate*rot*scale);

    // front, right wall
    translate = glm::translate(mat4(1.0),vec3(16.25,3.5,-10.25));
    scale = glm::scale(mat4(1.0),vec3(6.5,8,.5));
    renderCube(transform*translate*scale);

    // chalckboard
    translate = glm::translate(mat4(1.0),vec3(-5,4,-9.99));
    scale = glm::scale(mat4(1.0),vec3(5.5,4,.5));
    renderTexture(transform*translate*scale,chalkboard);

    // teacher's desk
    translate = glm::translate(mat4(1.0),vec3(-5,1,-7.5));
    renderTeacherDesk(transform*translate);

    // students' desks
    for (int i = 0; i < 2; i++){
        for (int j = 0; j < 5; j++){
            translate = glm::translate(mat4(1.0),vec3(-9+2*j,.25,-3.5+2*i));
            renderStudentDesk(transform*translate);
        }
    }
}

void GLWidget::renderTeacherDesk(mat4 transform){
    mat4 translate;
    mat4 scale;
    // paint desktop
    scale = glm::scale(mat4(1.0),vec3(4,.1,3));
    renderTexture(transform*scale,desk2);

    // paint pretend drawers
    translate = glm::translate(mat4(1.0),vec3(-1.25,-.5,0));
    scale = glm::scale(mat4(1.0),vec3(1.25,1,1.25));
    renderPretendDrawer(transform*translate);

    translate = glm::translate(mat4(1.0),vec3(1.25,-.5,0));
    scale = glm::scale(mat4(1.0),vec3(1.25,1,1.25));
    renderPretendDrawer(transform*translate);
}

void GLWidget::renderPretendDrawer(mat4 transform){
    mat4 scale = glm::scale(mat4(1.0),vec3(1,1,1.25));
    renderTexture(transform*scale,desk2);

    // paint desk legs
    mat4 translate = glm::translate(mat4(1.0),vec3(.35,-.75,.5));
    renderLeg(transform*translate,desk2);

    translate = glm::translate(mat4(1.0),vec3(.35,-.75,-.5));
    renderLeg(transform*translate,desk2);

    translate = glm::translate(mat4(1.0),vec3(-.35,-.75,.5));
    renderLeg(transform*translate,desk2);

    translate = glm::translate(mat4(1.0),vec3(-.35,-.75,-.5));
    renderLeg(transform*translate,desk2);
}

void GLWidget::renderStudentDesk(mat4 transform){
    mat4 translate;
    mat4 scale;
    //top of desk
    scale = glm::scale(mat4(1.0),vec3(1.5,.25,1.2));
    renderTexture(transform*scale,desk);

    //middle of desk
    translate = glm::translate(mat4(1.0),vec3(0,-.2,-.1));
    scale = glm::scale(mat4(1.0),vec3(1.5,.2,1));
    renderTexture(transform*translate*scale,desk);

    //desk legs
    translate = glm::translate(mat4(1.0),vec3(.35,-.5,.25));
    renderLeg(transform*translate,desk);

    translate = glm::translate(mat4(1.0),vec3(.35,-.5,-.5));
    renderLeg(transform*translate,desk);

    translate = glm::translate(mat4(1.0),vec3(-.35,-.5,.25));
    renderLeg(transform*translate,desk);

    translate = glm::translate(mat4(1.0),vec3(-.35,-.5,-.5));
    renderLeg(transform*translate,desk);
}

void GLWidget::renderLeg(mat4 transform,GLuint textureID){
    mat4 scale = glm::scale(mat4(1.0),vec3(.1,.5,.1));
    renderTexture(transform*scale,textureID);
}

void GLWidget::renderMeatGrinder(mat4 transform){
    mat4 translate;
    mat4 scale;
    mat4 rot;
    std::vector<vec3>rotations;

    // render lower funnel
    translate = glm::translate(mat4(1.0),vec3(-2,1.25,0));
    scale = glm::scale(mat4(1.0),vec3(2.5,2,2));
    renderTexture(transform*translate*scale,wall);

    // render body
    translate = glm::translate(mat4(1.0),vec3(0,1.5,0));
    scale = glm::scale(mat4(1.0),vec3(2.75,2.75,2.75));
    renderTexture(transform*translate*scale, wall2);

    // render upper funnel
    renderFunnel(transform*translate);

     // render base
 //    translate = glm::translate(mat4(1.0),vec3(-1,0,0));
     scale = glm::scale(mat4(1.0),vec3(1.5,1,1.5));
     renderCube(transform*scale);

    // render gear
    rotations.push_back(vec3(0, 0, 0));
    rotations.push_back(vec3(0, M_PI, 0));
    rotations.push_back(vec3(0,2*M_PI,0));
    animateObjects(rotations);
    numRows = 5;
    maxNumBricks = 3;
    brickSize = vec3(.5,.25,.25);
    translate = glm::translate(mat4(1.0),vec3(-2.75,1.25,0));
    rot = glm::rotate(mat4(),(float)M_PI/2,vec3(0,0,1));
    renderFunWall(transform*translate*rot*quatSlerp,metal);
}

void GLWidget::renderFunnel(mat4 transform){
    mat4 translate;
    mat4 scale;
    mat4 rot;
    std::vector<vec3>rotations;

    // render upper funnel, level1
    translate = glm::translate(mat4(1.0),vec3(0,1.5,0));
    scale = glm::scale(mat4(1.0),vec3(1,.25,1.5));
    renderCube(transform*translate*scale);

    // render upper funnel, level2
    translate = glm::translate(mat4(1.0),vec3(0,1.88,0));
    scale = glm::scale(mat4(1.0),vec3(1.5,.5,1.5));
    renderCube(transform*translate*scale);

    // render upper funnel, level3
    translate = glm::translate(mat4(1.0),vec3(0,2.5,0));
    scale = glm::scale(mat4(1.0),vec3(2,.75,2));
    renderCube(transform*translate*scale);

    // render upper funnel, level4
    translate = glm::translate(mat4(1.0),vec3(0,3,0));
    scale = glm::scale(mat4(1.0),vec3(2.5,.75,2.5));
    renderCube(transform*translate*scale);
}

void GLWidget::renderFunWall(mat4 transform, GLint textureID){
    //    float wallWidth = maxNumBricks*brickSize.x + (maxNumBricks-1)*spacing;
    //render front
    renderWall(transform,textureID);
    //render 1st cross
    mat4 rotate= glm:: rotate(mat4(1.0), 45.f, glm::vec3(0,1,0));
    renderWall(transform*rotate,textureID);
    //    render 2nd cross
    rotate= glm::rotate(mat4(1.0), -45.f, glm::vec3(0,1,0));
    renderWall(transform*rotate,textureID);
}

void GLWidget::renderWall(mat4 transform, GLint textureID){
    for(int i = 0; i < numRows; i++){
        if(i%2==0){
            //first and even rows have the max row length
            mat4 translate = glm::translate(mat4(1.0),vec3(0,i*(brickSize.y + spacing),0));
            renderRow(transform*translate,textureID);
        }else{
            //in between rows (odd) have max-1 row length
            maxNumBricks--;
            mat4 translate = glm::translate(mat4(1.0),vec3(0,i*(brickSize.y + spacing),0));
            renderRow(transform*translate,textureID);
            //reset numBricks
            maxNumBricks++;
        }
    }
}

void GLWidget::renderRow(mat4 transform, GLint textureID){
    float wallWidth = maxNumBricks*brickSize.x + (maxNumBricks-1)*spacing;
    for(int i = 0; i < maxNumBricks; i++){
        mat4 scale = glm::scale(mat4(1.0), brickSize);
        //the PERFECT translation to keep the wall PERFECTLY centered for all wall sizes
        mat4 translate = glm::translate(mat4(1.0),vec3(i*(brickSize.x+spacing)+.5*(brickSize.x-wallWidth),0,0));
        renderTexture(transform*translate*scale,textureID);
    }
}

void GLWidget::renderHammer(mat4 transform){
    int h = 4;
    mat4 scale = glm::scale(mat4(1.0),vec3(.25,h,.25));
    renderTexture(transform*scale,handle);
    renderHead(transform,h);
}

void GLWidget::renderHead(mat4 transform, int h){
    // set lighting for iron texture on hammers
    glUniform1f(texAmbientLoc, 0.2);
    glUniform1f(texDiffuseLoc, 0.5);
    glUniform1f(texSpecIntensity,1);
    mat4 translate = glm::translate(mat4(1.0),vec3(-.1,h-2,0));
    mat4 scale = glm::scale(mat4(1.0),vec3(1,.3,.3));
    renderTexture(transform*translate*scale,metal);
    renderFace(transform*translate);
}

void GLWidget::renderFace(mat4 transform){
    mat4 translate = glm::translate(mat4(1.0),vec3(-.5,0,0));
    mat4 scale = glm::scale(mat4(1.0),vec3(.4,.4,.4));
    renderTexture(transform*translate*scale,metal);
}

void GLWidget::renderTexture(mat4 transform,GLuint textureID){

    glUseProgram(texProg);
    glBindVertexArray(texVao);
    mat4 temp = texModelMatrix*transform;
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniformMatrix4fv(texModelMatrixLoc, 1, false, value_ptr(temp));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

GLuint GLWidget::loadShaders(const char* vertf, const char* fragf) {
    GLuint program = glCreateProgram();

    // read vertex shader from Qt resource file
    QFile vertFile(vertf);
    vertFile.open(QFile::ReadOnly | QFile::Text);
    QString vertString;
    QTextStream vertStream(&vertFile);
    vertString.append(vertStream.readAll());
    std::string vertSTLString = vertString.toStdString();

    const GLchar* vertSource = vertSTLString.c_str();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, NULL);
    glCompileShader(vertShader);
    {
        GLint compiled;
        glGetShaderiv( vertShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( vertShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( vertShader, len, &len, log );
            std::cout << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, vertShader);

    // read fragment shader from Qt resource file
    QFile fragFile(fragf);
    fragFile.open(QFile::ReadOnly | QFile::Text);
    QString fragString;
    QTextStream fragStream(&fragFile);
    fragString.append(fragStream.readAll());
    std::string fragSTLString = fragString.toStdString();

    const GLchar* fragSource = fragSTLString.c_str();

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);
    {
        GLint compiled;
        glGetShaderiv( fragShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( fragShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( fragShader, len, &len, log );
            std::cerr << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    {
        GLint linked;
        glGetProgramiv( program, GL_LINK_STATUS, &linked );
        if ( !linked ) {
            GLsizei len;
            glGetProgramiv( program, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetProgramInfoLog( program, len, &len, log );
            std::cout << "Shader linker failed: " << log << std::endl;
            delete [] log;
        }
    }

    return program;
}

void GLWidget::keyPressEvent(QKeyEvent *event) {    
    switch(event->key()) {
    case Qt::Key_W:
        // forward
        forward = true;
        break;
    case Qt::Key_A:
        // left
        left = true;
        break;
    case Qt::Key_D:
        // right
        right = true;
        break;
    case Qt::Key_S:
        // backward
        back = true;
        break;
    case Qt::Key_Tab:
        // toggle fly mode
        flymode = !flymode;
        break;
    case Qt::Key_Shift:
        // down
        down = true;
        break;
    case Qt::Key_Space:
        // up or jump
        up = true;
        break;
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent *event) {
    switch(event->key()) {
    case Qt::Key_W:
        // forward
        forward = false;
        break;
    case Qt::Key_A:
        // left
        left = false;
        break;
    case Qt::Key_D:
        // right
        right = false;
        break;
    case Qt::Key_S:
        // backward
        back = false;
        break;
    case Qt::Key_Tab:
        break;
    case Qt::Key_Shift:
        // down
        down = false;
        break;
    case Qt::Key_Space:
        // up or jump
        up = false;
        break;
    }
}

void GLWidget::animate(){
    float dt = 0.016;
    vec3 forwardVec = -vec3(yawMatrix[2]);
    vec3 rightVec = vec3(orientation[0]);
    vec3 upVec = vec3(0,1,0);
    float speed = 10;
    velocity = vec3(0,0,0);

    if(flymode){
        forwardVec = -vec3(orientation[2]);
    }else if(!flymode){
        forwardVec = -vec3(yawMatrix[2]);
    }

    if(forward){
        velocity += forwardVec;
    }
    if(back){
        velocity += -forwardVec;
    }
    if(right){
        velocity += rightVec;
    }
    if(left){
        velocity += -rightVec;
    }
    if(up){
        velocity += upVec;
    }
    if(down){
        velocity += -upVec;
    }

    if(length(velocity)>0){
//        velocity = normalize(velocity);
        position += normalize(velocity)*speed*dt;
    }

//    position += velocity*speed*dt;
    updateView();
    update();
}

void GLWidget::animateObjects(std::vector<vec3> rotations){
    // increment time by our time step
    float dt = .016;
    time += dt;
    if(walk >= 1){
        walk = 0;
    }else{
        walk += .0003;
    }

    // we want 2 seconds of animation per rotation then to start over, so
    // restart to 0 once we've reached our max time
    if(time > 2*(rotations.size()-1)) {
        time = 0;
    }

    // Convert time to a value between 0 and 1
    // at 0 we're at the beginning of our rotations
    // array, and at 1 we've reach the last one
    float t = fmin(time/(2*(rotations.size()-1)),1);


    // Get two indices into our rotations array
    // that represent our current animation
    unsigned int fromIndex = t*(rotations.size()-1);
    unsigned int toIndex = fromIndex+1;

    // when t = 1 toIndex will be out of range, so
    // just clamp it to the last index in the array
    if(toIndex > rotations.size()-1) {
        toIndex = rotations.size()-1;
    }

    // we want t to be a 0-1 value that represents the
    // percentage between two consecutive indices, so
    // get our current index as a floating point number
    // then subtract off the integer portion
    t = t*(rotations.size()-1)-(int)(t*(rotations.size()-1));

    // Euler angle representations of
    vec3 from = rotations[fromIndex];
    vec3 to = rotations[toIndex];

    // Part 3 - Quaternions are another way to represent orientation.
    // glm has a quaternion data structure called quat. It's constructor
    // can take a vec3 that represents Euler angles. Construct two quaternions
    // using the from and to euler angles.

    quat fromQ(from);
    quat toQ(to);

    // Interpolate the two quaternions using glm::slerp. slerp stands for
    // spherical linear interpolation and is how quaternions can be animated
    // along the shortest path. glm::slerp takes 3 arguments:
    // glm::slerp(glm::quat q1, glm::quat q2, float t)
    // where t is in the range 0-1 and returns a quaternion t percent
    // between q1 and q2

    quat q = glm::slerp(fromQ,toQ,t);

    // The last step is to convert the resulting quaternion into a matrix
    // for use in our fragment shader. Use glm::toMat4(glm::quat) to do so
    // and store the resulting matrix in quatSlerp. Again, quatSlerp is used
    // in paintGL to render our third cube.

    quatSlerp = glm::toMat4(q);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    vec2 pt(event->x(), event->y());
    lastPt = pt;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    vec2 pt(event->x(), event->y());
    vec2 d = pt-lastPt;
    // Part 1 - use d.x and d.y to modify your pitch and yaw angles
    // before constructing pitch and yaw rotation matrices with them
    yaw -= d.x/100;
    pitch -= d.y/100;

    if(pitch>M_PI/2){
        pitch = M_PI/2;
    }else if(pitch<-M_PI/2){
        pitch = -M_PI/2;
    }

    yawMatrix = glm::rotate(mat4(1.0),yaw,vec3(0,1,0));
    pitchMatrix = glm::rotate(mat4(1.0),pitch,vec3(1,0,0));

    //    viewMatrix = inverse(yawMatrix*pitchMatrix);
    orientation = yawMatrix*pitchMatrix;
    //    glUseProgram(cubeProg);
    //    glUniformMatrix4fv(cubeViewMatrixLoc,1,false,value_ptr(viewMatrix));
    //    update();
    //    updateView();
    lastPt = pt;
}

void GLWidget::updateView(){
    // set bounds on the world
    if(position.x > 25){
        position.x = 25;
    } else if (position.x < -25){
        position.x = -25;
    }
    if(position.z > 25){
        position.z = 25;
    } else if (position.z < -25){
        position.z = -25;
    }
    if(position.y > 100){
        position.y = 100;
    } else if (position.y < 0){
        position.y = 0;
    }
    mat4 trans = glm::translate(mat4(1.0),position);
    viewMatrix = inverse(trans*orientation);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeViewMatrixLoc,1,false,value_ptr(viewMatrix));

    glUseProgram(gridProg);
    glUniformMatrix4fv(gridViewMatrixLoc,1,false,value_ptr(viewMatrix));

    glUseProgram(texProg);
    glUniformMatrix4fv(texViewMatrixLoc,1,false,value_ptr(viewMatrix));
}
