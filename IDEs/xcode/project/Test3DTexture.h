//
//  Test3DTexture.h
//  VolumeRenderIOS
//
//  Created by Kristofer Schlachter on 5/16/14.
//
//

#ifndef VolumeRenderIOS_Test3DTexture_h
#define VolumeRenderIOS_Test3DTexture_h
#include "OpenGLHelper.h"

static GLuint load3DScan(const std::string& filename,int dx, int dy, int dz);
static GLuint CreatePyroclasticVolume(int n, float r);
void initialize();
static void loadUniforms();
void render();
void update(double seconds);
void resizeViewport(GLuint w, GLuint h);
#endif
