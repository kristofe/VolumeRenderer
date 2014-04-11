//
//  main.cpp
//  Texture3DExperiments
//
//  Created by Kristofer Schlachter on 7/8/13.
//  Copyright (c) 2013 Kristofer Schlachter. All rights reserved.
//
#ifdef __APPLE__
	#define GLFW_INCLUDE_GLCOREARB
#elif WIN32
	#pragma comment(lib, "GLFW/glfw3dll.lib")
	#pragma comment(lib, "opengl32.lib")
	#include <GL/glew.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "GLFW/glfw3.h" // - lib is in /usr/local/lib/libglfw3.a
#include "test.h"
#include "glutil.h"
#include "glprogram.h"
#include "renderlib/shadersource.h"
#include "renderlib/glm/glm.hpp"
#include "renderlib/glm/gtc/matrix_transform.hpp"
#include "renderlib/glm/gtc/type_ptr.hpp"

extern "C" {
#include "renderlib/perlin.h"
}

using namespace renderlib;

struct ProgramHandles {
    GLuint SinglePass;
};
//Prototypes
static ProgramHandles Programs;
static GLuint CreatePyroclasticVolume(int n, float r);
//static ITrackball* Trackball;
static GLuint CubeCenterVbo;
static glm::mat4 ProjectionMatrix;
static glm::mat4 ModelviewMatrix;
static glm::mat4 ViewMatrix;
static glm::mat4 ModelviewProjection;
static glm::vec3 EyePosition;
static GLuint CloudTexture;
static SurfacePod IntervalsFbo[2];
static bool SinglePass = true;
static float fieldOfView = 0.7f;


void hintOpenGL32CoreProfile(){
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}



static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

static void keyHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  if (key == '1') fieldOfView += 0.05f;
  if (key == '2') fieldOfView -= 0.05f;
}

static void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
  /*
  if (button = GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
      Trackball->MouseDown(x, y);
  else if (button = GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
    Trackball->MouseUp(x, y);
  else if (button = GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
      Trackball->ReturnHome();
  */
}

static void mousePositionHandler(GLFWwindow* window, double x, double y)
{
      //Trackball->MouseMove(x, y);
}
RenderConfig getConfig()
{
    RenderConfig config;
    config.title = "Raycast";
    config.width = 800;
    config.height = 800;
    config.multisample = 0;
    config.verticalSync = 0;
    return config;
}
void resizeViewport(GLFWwindow* window){
  RenderConfig cfg = getConfig();
  /*
  float ratio;
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  ratio = width / (float) height;
  */
  glViewport(0, 0, cfg.width, cfg.height);
  glClear(GL_COLOR_BUFFER_BIT);
}
static GLuint CreatePyroclasticVolume(int n, float r)
{
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_3D, handle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char *data = new unsigned char[n*n*n];
    unsigned char *ptr = data;

    float frequency = 3.0f / n;
    float center = n / 2.0f + 0.5f;

    for(int x=0; x < n; ++x) {
        for (int y=0; y < n; ++y) {
            for (int z=0; z < n; ++z) {
                float dx = center-x;
                float dy = center-y;
                float dz = center-z;

                float off = fabsf((float) PerlinNoise3D(
                    x*frequency,
                    y*frequency,
                    z*frequency,
                    5,
                    6, 3));

                float d = sqrtf(dx*dx+dy*dy+dz*dz)/(n);
                bool isFilled = (d-off) < r;
                *ptr++ = isFilled ? 255 : 0;
            }
        }
        fprintf(stdout,"Slice %d of %d\n", x, n);
    }

    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_RED,
                 n, n, n, 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 data);

    delete[] data;
    return handle;
}

void initialize()
{
    RenderConfig cfg = getConfig();

    //Trackball = createTrackball(cfg.Width * 1.0f, cfg.Height * 1.0f, cfg.Width * 0.5f);
    Programs.SinglePass = GLUtil::loadProgram("shaders/SinglePassRayMarch.glsl", "VS", "FS", "GS");
    CubeCenterVbo = CreatePointVbo(0, 0, 0);
    CloudTexture = CreatePyroclasticVolume(128, 0.025f);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


static void loadUniforms()
{
    SetUniform("ModelviewProjection", ModelviewProjection);
    SetUniform("Modelview", ModelviewMatrix);
    SetUniform("ViewMatrix", ViewMatrix);
    SetUniform("ProjectionMatrix", ProjectionMatrix);
    SetUniform("RayStartPoints", 1);
    SetUniform("RayStopPoints", 2);
    SetUniform("EyePosition", EyePosition);


    glm::vec4 eye(EyePosition.x,EyePosition.y, EyePosition.z,0);
    glm::vec4 rayOrigin(glm::transpose(ModelviewMatrix) * eye);
    SetUniform("RayOrigin", rayOrigin);

    float focalLength = 1.0f / std::tan(fieldOfView / 2);
    SetUniform("FocalLength", focalLength);

    RenderConfig cfg = getConfig();
    SetUniform("WindowSize", float(cfg.width), float(cfg.height));
}

void render()
{
    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(SlotPosition);
    glBindTexture(GL_TEXTURE_3D, CloudTexture);

    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(Programs.SinglePass);
    loadUniforms();
    glDrawArrays(GL_POINTS, 0, 1);
}

static void update(unsigned int microseconds)
{
    float dt = microseconds * 0.000001f;

    /*
    Trackball->Update(microseconds);
    EyePosition = Point3(0, 0, 5 + Trackball->GetZoom());

    Vector3 up(0, 1, 0); Point3 target(0);
    ViewMatrix = Matrix4::lookAt(EyePosition, target, up);

    Matrix4 modelMatrix(transpose(Trackball->GetRotation()), Vector3(0));
    ModelviewMatrix = ViewMatrix * modelMatrix;

    float n = 1.0f;
    float f = 100.0f;

    ProjectionMatrix = Matrix4::perspective(fieldOfView, 1, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;
    */
    EyePosition = glm::vec3(0, 0, 5);

    glm::vec3 up(0, 1, 0); glm::vec3 target(0);
    ViewMatrix = glm::lookAt(EyePosition, target, up);

    glm::mat4 modelMatrix;
    ModelviewMatrix = ViewMatrix * modelMatrix;

    float n = 1.0f;
    float f = 100.0f;

    ProjectionMatrix = glm::perspective(fieldOfView, 1.0f, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;
}
///////
typedef struct {
   GLfloat x, y, z, r, g, b;
} Triangle;

Triangle triangle[3] = {
   {-0.6f, -0.4f, 0.f, 1.f, 0.f, 0.f},
   {0.6f, -0.4f, 0.f,0.f, 1.f, 0.f},
   {0.f, 0.6f, 0.f,0.f, 0.f, 1.f}
};

GLubyte indices[3] = {
    0, 1, 2
};

GLuint gVAO = 0;
GLuint gVBO = 0;
GLProgram program1;

void sizeViewport(GLFWwindow* window){
  float ratio;
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  ratio = width / (float) height;
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);
  //glMatrixMode(GL_PROJECTION);
  glm::mat4 ortho = glm::ortho(-ratio,ratio, -1.0f, 1.0f, 1.0f, -1.0f);
  //glLoadMatrixf(&ortho[0][0]);

  //glLoadIdentity();
  //glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
}
void loadTriangle()
{
   // make and bind the VAO
   glGenVertexArrays(1, &gVAO);
   glBindVertexArray(gVAO);

   // make and bind the VBO
   glGenBuffers(1, &gVBO);
   glBindBuffer(GL_ARRAY_BUFFER, gVBO);

   // Put the three triangle verticies into the VBO
   GLfloat vertexData[] = {
      //  X     Y     Z
      0.0f, 0.8f, 0.0f,1.0,0.0,0.0,
      -0.8f,-0.8f, 0.0f,0.0,1.0,0.0,
      0.8f,-0.8f, 0.0f,0.0,0.0,1.0
   };
   glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

   // connect the xyz to the "vert" attribute of the vertex shader
   GLint vertSlot = program1.getAttributeLocation("vert");
   glEnableVertexAttribArray(vertSlot);
   glVertexAttribPointer(vertSlot, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle),
                         BUFFER_OFFSET(0));

   std::cout << "Setup Vert Slot = " << vertSlot << std::endl; std::cout.flush();
   GLUtil::checkGLErrors();

   GLint colorSlot = program1.getAttributeLocation("color");
   std::cout << "Color Slot = " << colorSlot << std::endl; std::cout.flush();
   glEnableVertexAttribArray(colorSlot);
   glVertexAttribPointer(colorSlot, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle),
                         BUFFER_OFFSET(sizeof(GLfloat)*3));

   std::cout << "Setup Color Slot = " << colorSlot << std::endl; std::cout.flush();
   GLUtil::checkGLErrors();


   GLint modelViewSlot = program1.getUniformLocation("modelview");
   std::cout << "Setup modelViewl Uniform Slot = " << modelViewSlot << std::endl; std::cout.flush();
   std::cout << "Done setting up triangle" << std::endl; std::cout.flush();

   glDisableVertexAttribArray(vertSlot);
   glDisableVertexAttribArray(colorSlot);

   GLUtil::checkGLErrors();
}

void drawTriangle(glm::mat4& mat)
{
   // BUG: THERE IS AN OPENGL GL_ERROR 1282 if glEnableVertexAttribArray is
   // called in this function

   // bind the program (the shaders)
    glUseProgram(program1.getID());

    //std::cout << "setting uniform = " << modelViewSlot << std::endl; std::cout.flush();
    //glUniformMatrix4fv(program1.getUniformLocation("modelview"), 1, 0, &mat[0][0]);
    glUniformMatrix4fv(program1.getUniformLocation("modelview"), 1, 0, glm::value_ptr(mat));
    //GLUtil::checkGLErrors();

    // bind the VAO (the triangle)
    glBindVertexArray(gVAO);


    // draw the VAO
    glDrawArrays(GL_TRIANGLES, 0, sizeof(triangle)/sizeof(Triangle));


    // unbind the VAO
    glBindVertexArray(0);


    // unbind the program
    glUseProgram(0);

    GLUtil::checkGLErrors();
}

int main(void)
{
  //test();
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);




  hintOpenGL32CoreProfile();
  RenderConfig cfg = getConfig();
  window = glfwCreateWindow(cfg.width, cfg.height, cfg.title.c_str(), NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }



  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keyHandler);
  glfwSetCursorPosCallback(window, mousePositionHandler);
  glfwSetMouseButtonCallback(window, mouseButtonHandler);


       // initialise GLEW
	glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
	if(glewInit() != GLEW_OK)
	{
		fprintf(stderr,"glewInit failed");
		return -1;
	}

  std::cout << GLUtil::getOpenGLInfo() << std::endl;std::cout.flush();
  //initialize();
    program1.loadShaders("Shaders/vertShader.glsl","Shaders/fragShader.glsl","");
  loadTriangle();
  glm::mat4 identityMatrix = glm::mat4(1.0);//Identity matrix


  while (!glfwWindowShouldClose(window))
  {
    sizeViewport(window);
	glm::mat4 rotMat = glm::rotate(identityMatrix,(float) glfwGetTime() * 50.f,
                                   glm::vec3(0.f,0.f,1.f));

    drawTriangle(rotMat);
    
	//update(glfwGetTime());
    //render();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

