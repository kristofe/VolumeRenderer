cmake_minimum_required  (VERSION 2.6)
project(VolumeRenderer)

set(SOURCES
   main.cpp
   renderlib/glutil_GL3Plus.cpp
   renderlib/glprogram.cpp
   renderlib/shadersource.cpp
   renderlib/perlin.c
   renderlib/trackball.cpp
#   glew/src/glew.c
)

set(HEADERS
  renderlib/include/utils.h
  renderlib/include/glutil.h
  renderlib/include/glprogram.h
  renderlib/include/glmesh.h
  renderlib/include/vector2.h
  renderlib/include/vector3.h
  renderlib/include/vertex.h
  renderlib/include/color.h
  renderlib/include/trackball.h
  renderlib/shadersource.h
  renderlib/vmath.hpp
  renderlib/perlin.h
)

set(SHADERS
  shaders/vertShader.glsl
  shaders/fragShader.glsl
  shaders/SinglePassRayMarch.glsl
)


# -g adds debugging symbols
# -Wall turns on all warnings
# -Wextra turns on a lot of warnings (but not too pedantic)
if(APPLE)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wextra -Wno-ignored-qualifiers -Wno-unused-parameter -std=c++11")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g -Wextra -Wno-ignored-qualifiers -Wno-unused-parameter")
add_definitions(-DTARGETMAC)
ENDIF(APPLE)

if(WIN32)
include_directories(glew/include)
LIST(APPEND SOURCES glew/src/glew.c)
ENDIF(WIN32)

#SET(CMAKE_OSX_ARCHITECTURES "i386")


if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    message("adding debug flags")
    add_definitions(-DDEBUG)
    add_definitions(-D_DEBUG)
    add_definitions(-DBREAK_ON_EXCEPTION)
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

set(GLFWLIB_PATH "/Users/kristofe/Documents/Projects/glfw-3.0.1/CMake/src")


#add_subdirectory(renderlib) #requires the directory to have a CMakeLists.txt file
include_directories(renderlib/include)
include_directories(renderlib/glm)
link_directories(${GLFWLIB_PATH})

if(APPLE)
   FIND_LIBRARY(IOKIT_LIBRARY IOKit)
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(OPENGL_LIBRARY OpenGL)

   MARK_AS_ADVANCED(IOKIT_LIBRARY
                     COCOA_LIBRARY
                     OPENGL_LIBRARY)
   SET(EXTRA_LIBS ${IOKIT_LIBRARY} ${COCOA_LIBRARY} ${OPENGL_LIBRARY} libglfw3.a )
ENDIF(APPLE)

if(WIN32)
  find_package(OpenGL REQUIRED)
  include_directories(${OpenGL_INCLUDE_DIRS})
  link_directories(${OpenGL_LIBRARY_DIRS})
  add_definitions(${OpenGL_DEFINITIONS})
  if(NOT OPENGL_FOUND)
      message(ERROR " OPENGL not found!")
  endif(NOT OPENGL_FOUND)
ENDIF(WIN32)

add_definitions(-DGLEW_STATIC)
add_executable(VolumeRenderer ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(VolumeRenderer ${EXTRA_LIBS})
