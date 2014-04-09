//
//  main.cpp
//  Texture3DExperiments
//
//  Created by Kristofer Schlachter on 7/8/13.
//  Copyright (c) 2013 Kristofer Schlachter. All rights reserved.
//
#define GLFW_INCLUDE_GLCOREARB
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

using namespace renderlib;


//TODO: Move this kind of stuff into a renderer
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

/////

void hintOpenGL32CoreProfile(){
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

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

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
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
    glUniformMatrix4fv(program1.getUniformLocation("modelview"), 1, 0, &mat[0][0]);
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
  window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);

  std::cout << GLUtil::getOpenGLInfo() << std::endl;std::cout.flush();
  program1.loadShaders("shaders/vertShader.glsl","shaders/fragShader.glsl","");
  loadTriangle();

  ShaderSource ss;
  ss.parseFile("shaders/SinglePassRayMarch.glsl", "--");
  std::cout << ss.getShader("VS") << std::endl;
  std::cout << ss.getShader("GS") << std::endl;
  std::cout << ss.getShader("FS") << std::endl;

  glm::mat4 identityMatrix = glm::mat4(1.0);//Identity matrix

  program1.enableVertexAttributes();

  while (!glfwWindowShouldClose(window))
  {
    sizeViewport(window);
    glm::mat4 rotMat = glm::rotate(identityMatrix,(float) glfwGetTime() * 50.f,
                                   glm::vec3(0.f,0.f,1.f));

    drawTriangle(rotMat);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
