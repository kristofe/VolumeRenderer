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
#include "renderlib/include/trackball.h"
#include "renderlib/shadersource.h"
/*
#include "renderlib/glm/glm.hpp"
#include "renderlib/glm/gtc/matrix_transform.hpp"
#include "renderlib/glm/gtc/type_ptr.hpp"
*/

#include "renderlib/vmath.hpp"

extern "C" {
#include "renderlib/perlin.h"
}

using namespace renderlib;
using namespace vmath;

struct ProgramHandles {
    GLuint SinglePass;
};
//Prototypes
static ProgramHandles Programs;
static GLuint CreatePyroclasticVolume(int n, float r);
//static ITrackball* Trackball;
static GLuint CubeCenterVbo;
static GLuint CubeCenterVao;
/*
static glm::mat4 ProjectionMatrix;
static glm::mat4 ModelviewMatrix;
static glm::mat4 ViewMatrix;
static glm::mat4 ModelviewProjection;
static glm::vec3 EyePosition;
*/
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static Point3 EyePosition;


static GLuint CloudTexture;
static SurfacePod IntervalsFbo[2];
static bool SinglePass = true;
static float fieldOfView = 0.7f;
static Trackball* trackball;
static int mouseX, mouseY;
static bool mouseDown = false;
//static glm::mat4 modelMatrix;


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
  if (button = GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
  {
	  mouseDown = true;
    trackball->MouseDown(mouseX, mouseY);
  }
  else if (button = GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
  {
	  mouseDown = false;
	  trackball->MouseUp(mouseX, mouseY);
  }
  else if (button = GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
  {
    trackball->ReturnHome();
  }
}

static void mousePositionHandler(GLFWwindow* window, double x, double y)
{
	if(mouseX != (int)x || mouseY != (int)y){
	  mouseX = (int)x;
	  mouseY = (int)y;
	  if(mouseDown)
		  trackball->MouseMove(x, y);
	}
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
        //fprintf(stdout,"Slice %d of %d\n", x, n);
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
	  //modelMatrix = glm::mat4();
    trackball = new Trackball(cfg.width * 1.0f, cfg.height * 1.0f, cfg.width * 0.5f);
    Programs.SinglePass = GLUtil::loadProgram("shaders/SinglePassRayMarch.glsl", "VS", "FS", "GS");
    //Programs.SinglePass = GLUtil::loadProgram("shaders/SinglePassRayMarch.glsl", "VS", "FS", "");
    ///Programs.SinglePass = GLUtil::loadShaders("Shaders/vertShader.glsl", "Shaders/fragShader.glsl", "");
    
    //CubeCenterVbo = CreatePointVbo(0, 0, 0);
	CreatePointVbo(Programs.SinglePass, &CubeCenterVbo, &CubeCenterVao);

    CloudTexture = CreatePyroclasticVolume(128, 0.025f);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
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

    Vector4 rayOrigin(transpose(ModelviewMatrix) * EyePosition);
    SetUniform("RayOrigin", rayOrigin.getXYZ());

    float focalLength = 1.0f / std::tan(fieldOfView / 2);
    SetUniform("FocalLength", focalLength);
	/*
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

    float focalLength = 1.0f / std::tan(fieldOfView / 2.0f);
    SetUniform("FocalLength", focalLength);
	*/
    RenderConfig cfg = getConfig();
    SetUniform("WindowSize", float(cfg.width), float(cfg.height));
}

void render()
{
	/*
    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(SlotPosition);
    glBindTexture(GL_TEXTURE_3D, CloudTexture);
	*/


	/*
	glBindVertexArray(CubeCenterVao);
	GLint vertLoc = glGetAttribLocation(Programs.SinglePass, "vert");
    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);
    glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(vertLoc);
	*/

    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(Programs.SinglePass);
    glBindTexture(GL_TEXTURE_3D, CloudTexture);
	glBindVertexArray(CubeCenterVao);
    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);

    loadUniforms();
    glDrawArrays(GL_POINTS, 0, 1);
    //glDrawArrays(GL_TRIANGLES, 0, 3);
}

static void update(double seconds)
{
    float dt = seconds;
	  unsigned int microseconds = seconds * 1000 * 1000;
    

	/////////
    //Trackball->Update(microseconds);

    EyePosition = Point3(0, 0, 5 + trackball->GetZoom());
    
    Vector3 up(0, 1, 0); Point3 target(0);
    ViewMatrix = Matrix4::lookAt(EyePosition, target, up);

    //Matrix4 modelMatrix(transpose(trackball->GetRotation()), Vector3(0));
    Quat q = Quat::identity();
    q = q.rotationY(dt);
    Matrix4 modelMatrix(transpose(Matrix3(q)), Vector3(0));
    ModelviewMatrix = ViewMatrix * modelMatrix;

    float n = 1.0f;
    float f = 100.0f;

    ProjectionMatrix = Matrix4::perspective(fieldOfView, 1, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;

	/*
    trackball->Update(microseconds);
	EyePosition = glm::vec3(0, 0, 5 + trackball->GetZoom());

    glm::vec3 up(0, 1, 0); glm::vec3 target(0);
    ViewMatrix = glm::lookAt(EyePosition, target, up);

	//glm::mat4 modelMatrix(glm::transpose(trackball->GetRotation()), glm::vec3(0));
	//glm::mat4 modelMatrix(glm::transpose(trackball->GetRotation()));
	modelMatrix = glm::rotate(glm::mat4(),(float)seconds* 20.0f,glm::normalize(glm::vec3(0,1,0)));
    //ModelviewMatrix = ViewMatrix * glm::transpose(modelMatrix);
    ModelviewMatrix = ViewMatrix * glm::transpose(glm::mat4());

    float n = 1.0f;
    float f = 100.0f;
	RenderConfig cfg = getConfig();
	float aspect = cfg.height/(float)cfg.width;
	const float radToDeg = 57.2957795f;
    ProjectionMatrix = glm::perspective(fieldOfView * radToDeg, aspect, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;
	*/
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


#if!__APPLE__
       // initialise GLEW
	glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
	if(glewInit() != GLEW_OK)
	{
		fprintf(stderr,"glewInit failed");
		return -1;
	}
#endif

  std::cout << GLUtil::getOpenGLInfo() << std::endl;std::cout.flush();
  initialize();
  /*
    program1.loadShaders("Shaders/vertShader.glsl","Shaders/fragShader.glsl","");
  loadTriangle();
  glm::mat4 identityMatrix = glm::mat4(1.0);//Identity matrix
  program1.enableVertexAttributes();
  */

  mouseX = mouseY = 100;

  while (!glfwWindowShouldClose(window))
  {
    //sizeViewport(window);
	//glm::mat4 rotMat = glm::rotate(identityMatrix,(float) glfwGetTime() * 50.f, glm::vec3(0.f,0.f,1.f));

    //drawTriangle(rotMat);
    
	update(glfwGetTime());
    render();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

