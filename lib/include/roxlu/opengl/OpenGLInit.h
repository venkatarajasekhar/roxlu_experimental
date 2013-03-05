#ifndef ROXLU_OPENGLINCLUDESH
#define ROXLU_OPENGLINCLUDESH

#include <roxlu/core/platform/Platform.h>

#define GLSL(version, shader)  "#version " #version "\n" #shader

/*
#if ROXLU_GL_WRAPPER == ROXLU_GLFW
 #if ROXLU_PLATFORM == ROXLU_LINUX
   #define GL_GLEXT_PROTOTYPES
   #include <GL/gl.h>
   #include <GL/glext.h>
 #elif ROXLU_PLATFORM == ROXLU_APPLE

//  #include <GL/glew.h>
//  #include <GL/glfw3.h>
// #include <GLXW/glxw.h>
//  #undef glGenVertexArrays
//  #undef glBindVertexArray
//  #define glGenVertexArrays glGenVertexArraysAPPLE
//  #define glBindVertexArray glBindVertexArrayAPPLE

 #elif ROXLU_PLATFORM == ROXLU_WINDOWS
  #include <GL/glew.h>
  #include <GL/glfw3.h>
 #endif // ROXLU_GL_WRAPPER == ROXLU_WINDOWS
#elif ROXLU_GL_WRAPPER == ROXLU_OPENFRAMEWORKS
//#include <ofMain.h>
  #include <GL/glew.h>
  #include <GL/glfw3.h>
#elif ROXLU_GL_WRAPPER == ROXLU_IOS
  #include <OpenGLES/ES2/gl.h>
  #include <OpenGLES/ES2/glext.h>

  #ifndef glGenVertexArrays
    #define glGenVertexArrays glGenVertexArraysOES
  #endif
  #ifndef glBindVertexArray
    #define glBindVertexArray glBindVertexArrayOES
  #endif
#elif ROXLU_GL_WRAPPER == ROXLU_CINDER
#elif ROXLU_GL_WRAPPER == ROXLU_COCOA
#include <OpenGL/gl.h>
#endif // ROXLU_GL_WRAPPER
*/

#include <GLXW/glxw.h>
#include <GL/glfw3.h>



#endif
