#ifndef TRIFECTA_SHADERS_H
#define TRIFECTA_SHADERS_H

#ifdef __APPLE__

/* define to silence OpenGL deprecation warnings */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#endif /* end __APPLE__ */

#ifdef WIN32

#include "Trifecta_Win32.h"

#include <GL/glew.h>

#endif /* end WIN32 */

#ifdef __linux__

#include <GL/glew.h>

#endif

/* Loads the shaders and returns the programId */
GLuint _loadShaders();

#endif