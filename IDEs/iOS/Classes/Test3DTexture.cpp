//
//  Test3DTexture.cpp
//  VolumeRenderIOS
//
//  Created by Kristofer Schlachter on 5/16/14.
//
//
#include "Test3DTexture.h"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "trackball.h"
#include "shadersource.h"
#include "vmath.hpp"
#include "Platform.h"

extern "C" {
#include "perlin.h"
}

using namespace renderlib;
using namespace vmath;

struct ProgramHandles {
    GLuint SinglePass;
};
//Prototypes
static ProgramHandles Programs;
static GLuint CubeCenterVbo;
static GLuint CubeCenterVao;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static Point3 EyePosition;

static int winWidth = 500;
static int winHeight = 500;

static GLuint CloudTexture;
static SurfacePod IntervalsFbo[2];
static bool SinglePass = true;
static float fieldOfView = 0.7f;
static Trackball* trackball;
static int mouseX, mouseY;
static bool mouseDown = false;





//static void error_callback(int error, const char* description)
//{
//    fputs(description, stderr);
//}
//
//
//
//static void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
//{
//    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
//    {
//        mouseDown = true;
//        trackball->MouseDown(mouseX, mouseY);
//    }
//    else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
//    {
//        mouseDown = false;
//        trackball->MouseUp(mouseX, mouseY);
//    }
//    else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
//    {
//        trackball->ReturnHome();
//    }
//}
//
//static void mousePositionHandler(GLFWwindow* window, double x, double y)
//{
//	if(mouseX != (int)x || mouseY != (int)y){
//        mouseX = (int)x;
//        mouseY = (int)y;
//        if(mouseDown)
//            trackball->MouseMove(x, y);
//	}
//}
//
//void resizeViewport(GLFWwindow* window){
//    glfwGetFramebufferSize(window, &winWidth, &winHeight);
//    glViewport(0, 0, winWidth, winHeight);
//    glClear(GL_COLOR_BUFFER_BIT);
//}

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
    while(fin.read((char*)&data[position],chunkSize)){
		position += 512;
        std::cout << position << std::endl;
    }
    
	fin.close();
    
#if TARGETIPHONE
    
    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_R16UI,
                 dx, dy, dz, 0,
                 GL_RED,
                 GL_UNSIGNED_SHORT,
                 data);
#else
    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_RED,
                 dx, dy, dz, 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 data);
#endif
    
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
                std::cout << off << " ";
                float d = sqrtf(dx*dx+dy*dy+dz*dz)/(n);
                bool isFilled = (d-off) < r;
                *ptr++ = isFilled ? 255 : 0;
            }
            std::cout << std::endl;
        }
        fprintf(stdout,"Slice %d of %d\n", x, n);
    }
 
#if TARGETIPHONE
    
    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_R16UI,
                 n, n, n, 0,
                 GL_RED,
                 GL_UNSIGNED_SHORT,
                 data);
#else
    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_RED,
                 n, n, n, 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 data);
#endif
    delete[] data;
    return handle;
}

void initialize()
{
    trackball = new Trackball(winWidth * 1.0f, winHeight * 1.0f, winWidth * 0.5f);
    
    //Test volume render shaders
    std::string path;
    GetFullFilePathFromResource("SinglePassRayMarch.glsl", path);
    Programs.SinglePass = GLUtil::complileAndLinkProgram(path, "VS", "FS_CTSCAN", "");
    CreatePointVbo(Programs.SinglePass, &CubeCenterVbo, &CubeCenterVao);
    
    CloudTexture = CreatePyroclasticVolume(8, 0.025f);
    //GetFullFilePathFromResource("512x512x512x_uint16.raw", path);
    //CloudTexture = load3DScan(path, 512,512,512);
    
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
    SetUniform("WindowSize", float(winWidth), float(winHeight));
}

void render()
{
    glClearColor(0.1f, 0.2f, 0.4f, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(Programs.SinglePass);
    glBindTexture(GL_TEXTURE_3D, CloudTexture);
	glBindVertexArray(CubeCenterVao);
    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);
    
    loadUniforms();
    glDrawArrays(GL_POINTS, 0, 1);
    //glDrawArrays(GL_TRIANGLES, 0, 3);
}

void update(double seconds)
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

//int main(void)
//{
//    //test();
//    GLFWwindow* window;
//    glfwSetErrorCallback(error_callback);
//    if (!glfwInit())
//        exit(EXIT_FAILURE);
//
//
//
//
//    hintOpenGL32CoreProfile();
//    winWidth = 500;
//    winHeight = 500;
//
//    window = glfwCreateWindow(winWidth, winHeight, "Volume Renderer", NULL, NULL);
//
//    if (!window)
//    {
//        glfwTerminate();
//        exit(EXIT_FAILURE);
//    }
//
//
//
//    glfwMakeContextCurrent(window);
//    glfwSetKeyCallback(window, keyHandler);
//    glfwSetCursorPosCallback(window, mousePositionHandler);
//    glfwSetMouseButtonCallback(window, mouseButtonHandler);
//
//
//#if!__APPLE__
//    // initialise GLEW
//	glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
//	if(glewInit() != GLEW_OK)
//	{
//		fprintf(stderr,"glewInit failed");
//		return -1;
//	}
//#endif
//
//    std::cout << GLUtil::getOpenGLInfo() << std::endl;std::cout.flush();
//    initialize();
//
//    mouseX = mouseY = 100;
//
//    while (!glfwWindowShouldClose(window))
//    {
//        update(glfwGetTime());
//        render();
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//    glfwDestroyWindow(window);
//    glfwTerminate();
//    exit(EXIT_SUCCESS);
//}