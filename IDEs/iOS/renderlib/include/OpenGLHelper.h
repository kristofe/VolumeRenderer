//
//  OpenGL.h
//  VolumeRenderIOS
//
//  Created by Kristofer Schlachter on 5/15/14.
//
//

#ifndef VolumeRenderIOS_OpenGL_h
#define VolumeRenderIOS_OpenGL_h

#ifdef TARGETIPHONE
    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>
    #include "glutil.h"
#elif TARGETMAC
    #include <OpenGL/gl3.h>
    #include "gl_desktop_util.h"
#elif WIN32
    #include <GL/glew.h>
    #include "glutil.h"
#endif



#endif //VolumeRenderIOS_OpenGL_h
