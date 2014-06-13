//
//  main.cpp
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
#include <cmath>
#include "GLFW/glfw3.h" // - lib is in /usr/local/lib/libglfw3.a
#include "test.h"
#include "glutil.h"
#include "glprogram.h"
#include "renderlib/include/trackball.h"
#include "renderlib/shadersource.h"

#include "renderlib/vmath.hpp"

extern "C" {
#include "renderlib/perlin.h"
}

#include "Platform.h"

using namespace renderlib;
using namespace vmath;

//Prototypes

static GLuint CreatePyroclasticVolume(int n, float r);
static GLuint CubeCenterVbo;
static GLuint CubeCenterVao;
static GLuint densityProgram;
static GLuint isoProgram;
static GLuint currentProgram;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static Point3 EyePosition;

static int winWidth = 800;
static int winHeight = 800;

static GLuint CloudTexture;
static float fieldOfView = 0.7f;
static Trackball* trackball;
static int mouseX, mouseY;
static bool mouseDown = false;
static bool mouseRightDown = false;
static float minDensity = 0.4f;
static float maxDensity = 1.0f;
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

  if (key == '1' && action == GLFW_PRESS)
  {
    currentProgram = currentProgram == isoProgram?densityProgram:isoProgram;
  }
//  if (key == '1' && action == GLFW_PRESS) fieldOfView += 0.05f;
//  if (key == '2' && action == GLFW_PRESS) fieldOfView -= 0.05f;
}

static void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
  {
      mouseDown = true;
      trackball->MouseDown(mouseX, mouseY);
  }
  else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
  {
      mouseDown = false;
      trackball->MouseUp(mouseX, mouseY);
  }
  else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
  {
      mouseRightDown = true;
      //trackball->ReturnHome();
  }
  else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
  {

      mouseRightDown = false;
  }
}

static void mousePositionHandler(GLFWwindow* window, double x, double y)
{
	if(mouseX != (int)x || mouseY != (int)y){
	  if(mouseDown)
    {
      mouseX = (int)x;
      mouseY = (int)y;
      trackball->MouseMove(x, y);
    }
  }
  }

static void mouseScrollHandler(GLFWwindow* window, double xoffset, double yoffset)
{
    float diff = yoffset;
    minDensity = minDensity + diff/winHeight * 4.0f;
    if(minDensity >= 1.0f || minDensity > maxDensity)
    {
      minDensity = maxDensity - 0.001f;
    }
    if(minDensity <= 0.0f)
    {
      minDensity = 0.0f;
    }
    diff = xoffset;
    maxDensity = maxDensity + diff/winWidth;
    if(maxDensity <= 0.0f || maxDensity < minDensity)
    {
      maxDensity = minDensity + 0.001f;
    }
    if(maxDensity > 1.0f)
    {
      maxDensity = 1.0f;
    }
}

void resizeViewport(GLFWwindow* window){
  glfwGetFramebufferSize(window, &winWidth, &winHeight);
  glViewport(0, 0, winWidth, winHeight);
  glClear(GL_COLOR_BUFFER_BIT);
}

static void windowResizeHandler(GLFWwindow* window, int width, int height)
{
  resizeViewport(window);
}



#include <iostream>
#include <fstream>
static GLuint load3DScan(const std::string& filename,int dx, int dy, int dz)
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


    int array_size = dx*dy*dz;
    unsigned short *data = new unsigned short[array_size];

    int position = 0;
    size_t chunkSize = sizeof(unsigned short)*512;

    std::ifstream fin(filename.c_str(), std::ios::in|std::ios::binary);
    if(fin.fail())
    {
      std::cout << "Could not find " << filename << std::endl;
    }
    while(fin.read((char*)&data[position],chunkSize))
    {
      position += 512;

    }


    fin.close();

    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_RED,
                 dx, dy, dz, 0,
                 GL_RED,
                 GL_UNSIGNED_SHORT,
                 data);

    delete[] data;
    return handle;
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
    trackball = new Trackball(winWidth * 1.0f, winHeight * 1.0f, winWidth * 0.5f);
    densityProgram = GLUtil::complileAndLinkProgram("shaders/SinglePassRayMarch.glsl", "VS", "FS_CTSCAN", "GS");
    isoProgram = GLUtil::complileAndLinkProgram("shaders/SinglePassRayMarch.glsl", "VS", "FS_ISO", "GS");
    currentProgram = isoProgram;
    GetGLError();
    CreatePointVbo(currentProgram, &CubeCenterVbo, &CubeCenterVao);
    GetGLError();

    CloudTexture = load3DScan("Data/512x512x512x_uint16.raw", 512,512,512);
    //CloudTexture = CreatePyroclasticVolume(128, 0.025f);
    GetGLError();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


static void loadUniforms()
{
    GetGLError();
    SetUniform("ModelviewProjection", ModelviewProjection);
    GetGLError();
    SetUniform("Modelview", ModelviewMatrix);
    GetGLError();
    SetUniform("ViewMatrix", ViewMatrix);
    GetGLError();
    SetUniform("ProjectionMatrix", ProjectionMatrix);
    GetGLError();
    SetUniform("RayStartPoints", 1);
    GetGLError();
    SetUniform("RayStopPoints", 2);
    GetGLError();
    SetUniform("EyePosition", EyePosition);
    GetGLError();

    Vector4 rayOrigin(transpose(ModelviewMatrix) * EyePosition);
    SetUniform("RayOrigin", rayOrigin.getXYZ());
    GetGLError();

    float focalLength = 1.0f / std::tan(fieldOfView / 2);
    SetUniform("FocalLength", focalLength);
    GetGLError();
    SetUniform("WindowSize", float(winWidth), float(winHeight));
    GetGLError();
    SetUniform("minDensity", minDensity);
    SetUniform("maxDensity", maxDensity);
    SetUniform("isoValue", minDensity);

}

void render()
{
    glClearColor(0.1f, 0.2f, 0.4f, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(currentProgram);
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
    trackball->Update(microseconds);

    EyePosition = Point3(0, 0, 5 + trackball->GetZoom());

    Vector3 up(0, 1, 0); Point3 target(0);
    ViewMatrix = Matrix4::lookAt(EyePosition, target, up);

    Matrix4 modelMatrix(transpose(trackball->GetRotation()), Vector3(0));
    ModelviewMatrix = ViewMatrix * modelMatrix;

    float n = 1.0f;
    float f = 100.0f;

    ProjectionMatrix = Matrix4::perspective(fieldOfView, 1, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;
}

int main(void)
{
  //test();
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);


  std::cout << GetCurrentDir() << std::endl;
  ChangeParentDir("data");

  hintOpenGL32CoreProfile();
  winWidth = 800;
  winHeight = 800;

  window = glfwCreateWindow(winWidth, winHeight, "Volume Renderer", NULL, NULL);

  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }



  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keyHandler);
  glfwSetCursorPosCallback(window, mousePositionHandler);
  glfwSetMouseButtonCallback(window, mouseButtonHandler);
  glfwSetScrollCallback(window, mouseScrollHandler);
  glfwSetWindowSizeCallback(window, windowResizeHandler);


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

  mouseX = mouseY = 100;

  while (!glfwWindowShouldClose(window))
  {
    update(glfwGetTime());
    render();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

