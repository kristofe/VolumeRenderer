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

static inline const char * GetGLErrorString(GLenum error)
{
	const char *str;
	switch( error )
	{
		case GL_NO_ERROR:
			str = "GL_NO_ERROR";
			break;
		case GL_INVALID_ENUM:
			str = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			str = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			str = "GL_INVALID_OPERATION";
			break;
#if defined __gl_h_ || defined __gl3_h_
		case GL_OUT_OF_MEMORY:
			str = "GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			str = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
#endif
#if defined __gl_h_
		case GL_STACK_OVERFLOW:
			str = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			str = "GL_STACK_UNDERFLOW";
			break;
		case GL_TABLE_TOO_LARGE:
			str = "GL_TABLE_TOO_LARGE";
			break;
#endif
		default:
			str = "(ERROR: Unknown Error Enum)";
			break;
	}
	return str;
}

#define GetGLError()									\
{														\
GLenum err = glGetError();							\
while (err != GL_NO_ERROR) {						\
printf("GLError %s set in File:%s Line:%d\n",	\
GetGLErrorString(err),					\
__FILE__,								\
__LINE__);								\
err = glGetError();								\
}													\
}


using namespace renderlib;
using namespace vmath;

struct ProgramHandles {
    GLuint SinglePass;
};
//Prototypes
//static ProgramHandles Programs;
//static GLuint TriangleVbo;
//static GLuint TriangleVao;
//static GLuint CubeVbo;
//static GLuint CubeVao;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static Point3 EyePosition;

static int winWidth = 500;
static int winHeight = 500;

//static GLuint CloudTexture;
static float fieldOfView = 0.7f;
static Trackball* trackball;
static int mouseX, mouseY;
static bool mouseDown = false;

#include "glprogram.h"
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

void loadTriangleAndProgram()
{
  //Load Program
  std::string path;
  GetFullFilePathFromResource("triangleShaders.glsl", path);
  
  program1.loadShaders(path, "VS", "FS", "");
  
  GetGLError();
  // make and bind the VAO
  glGenVertexArrays(1, &gVAO);
  glBindVertexArray(gVAO);
  
  // make and bind the VBO
  glGenBuffers(1, &gVBO);
  glBindBuffer(GL_ARRAY_BUFFER, gVBO);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
  
  // connect the xyz to the "vert" attribute of the vertex shader
  GLint vertSlot = program1.getAttributeLocation("vert");
  glEnableVertexAttribArray(vertSlot);
  glVertexAttribPointer(vertSlot, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle),
                        BUFFER_OFFSET(0));
  
  GetGLError();
  std::cout << "Setup Vert Slot = " << vertSlot << std::endl; std::cout.flush();
  GetGLError();
  
  GLint colorSlot = program1.getAttributeLocation("color");
  GetGLError();
  glEnableVertexAttribArray(colorSlot);
  glVertexAttribPointer(colorSlot, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle),
                        BUFFER_OFFSET(sizeof(GLfloat)*3));
  GetGLError();
  
  std::cout << "Setup Color Slot = " << colorSlot << std::endl; std::cout.flush();
  GetGLError();
  
  
  //    GLint modelViewSlot = program1.getUniformLocation("modelview");
  //    std::cout << "Setup modelViewl Uniform Slot = " << modelViewSlot << std::endl; std::cout.flush();
  std::cout << "Done setting up triangle" << std::endl; std::cout.flush();
  
  
  GetGLError();
}

void drawTriangle()
{
  float secs = GetTicks() * 0.001f;
  glClearColor(0.1f, 0.2f, 0.4f, 0);
//  glClearColor(sinf(secs / 1.0f) * 0.5f + 0.5f, 0.2f, 0.4f, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  // BUG: THERE IS AN OPENGL GL_ERROR 1282 if glEnableVertexAttribArray is
  // called in this function
  Matrix4 rotMat = Matrix4::rotation(secs, Vector3(0,0,1));
  
  
  // bind the program (the shaders)
  glUseProgram(program1.getID());
  GetGLError();
  
  SetUniform("modelview", rotMat);
  GetGLError();
  
  // bind the VAO (the triangle)
  glBindVertexArray(gVAO);
  GetGLError();
  
  
  // draw the VAO
  glDrawArrays(GL_TRIANGLES, 0, sizeof(triangle)/sizeof(Triangle));
  
  GetGLError();
  
  // unbind the VAO
  glBindVertexArray(0);
  
  
  // unbind the program
  glUseProgram(0);
  GetGLError();
//  GLUtil::checkGLErrors();
}



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
    //unsigned short *data = new unsigned short[array_size];
    unsigned char *char_data = new unsigned char[array_size];
    
    int position = 0;
	size_t chunkSize = sizeof(unsigned short)*512;
    float scale = 255.0f/65535.0f;
	std::ifstream fin(filename.c_str(), std::ios::in|std::ios::binary);
    std::cout << " good()=" << fin.good();
    std::cout << " eof()=" << fin.eof();
    std::cout << " fail()=" << fin.fail();
    std::cout << " bad()=" << fin.bad();
    
    unsigned short buff[chunkSize];
    
    //This copies the data straight into the array
    //while(fin.read((char*)data[position],chunkSize)){
//     position += 512;
//    }
    
    //Convert the data from unsigned short to unsigned byte.
    while(fin.read((char*)buff,chunkSize)){
        int count = fin.gcount() * 0.5;//Multiplied by half because unsigned short is two bytes
        for(int i = 0; i < count; i++)
        {
            char_data[position + i] = buff[i] * scale;
        }
        position += 512;
    }
    
	fin.close();
    
#if TARGETIPHONE
    GLint size;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &size);
    printf("GL_MAX_3D_TEXTURE_SIZE: %d\n", size);
    
//    glTexImage3D(GL_TEXTURE_3D, 0,
//                 GL_R16UI,
//                 dx, dy, dz, 0,
//                 GL_RED_INTEGER,
//                 GL_UNSIGNED_SHORT,
//                 data);
    
    
    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_LUMINANCE,
                 dx, dy, dz, 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 char_data);
#else
    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_RED,
                 dx, dy, dz, 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 data);
#endif
    GetGLError();
    //delete[] data;
    delete[] char_data;

    return handle;
}
static GLuint CreatePyroclasticVolume(int n, float r)
{
    GLuint handle;
    GetGLError();
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_3D, handle);
    GetGLError();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GetGLError();
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
                //std::cout << off << " ";
                float d = sqrtf(dx*dx+dy*dy+dz*dz)/(n);
                bool isFilled = (d-off) < r;
                *ptr++ = isFilled ? 255 : 0;
            }
            //std::cout << std::endl;
        }
        fprintf(stdout,"Slice %d of %d\n", x, n);
    }
 
#if TARGETIPHONE
    
    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_LUMINANCE,
                 n, n, n, 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 data);
    GetGLError();
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
    loadTriangleAndProgram();
    
//    trackball = new Trackball(winWidth * 1.0f, winHeight * 1.0f, winWidth * 0.5f);
//  
//    //Test volume render shaders
//    std::string path;
//    GetFullFilePathFromResource("SinglePassRayMarch.glsl", path);
//    GetGLError();
//    Programs.SinglePass = GLUtil::complileAndLinkProgram(path, "VS", "FS_CTSCAN", "");
//    //CreateCubeVbo(Programs.SinglePass, &CubeVbo, &CubeVao);
//
//    GetGLError();
//    
//    CloudTexture = CreatePyroclasticVolume(128, 0.025f);
//    //GetFullFilePathFromResource("volume_texture_uint16.raw", path);
//    //CloudTexture = load3DScan(path, 512,512,512);
//    GetGLError();
//    
//    glDisable(GL_DEPTH_TEST);
//    glEnable(GL_BLEND);
//    glDisable(GL_CULL_FACE);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    
    float focalLength = 1.0f / tan(fieldOfView / 2);
    SetUniform("FocalLength", focalLength);
    SetUniform("WindowSize", float(winWidth), float(winHeight));
}




void render()
{
  drawTriangle();
//    glClearColor(0.1f, 0.2f, 0.4f, 0);
//    glClear(GL_COLOR_BUFFER_BIT);
//    glUseProgram(Programs.SinglePass);
//    glBindTexture(GL_TEXTURE_3D, CloudTexture);
//    glBindVertexArray(CubeCenterVao);
//    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);
//  
//    loadUniforms();
//    glDrawArrays(GL_POINTS, 0, 1);
//    GetGLError();
}

void update(double seconds)
{
  return;
//    float dt = seconds;
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
