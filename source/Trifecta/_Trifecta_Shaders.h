#ifndef TRIFECTA_SHADERS_H
#define TRIFECTA_SHADERS_H

#ifdef __APPLE__

/* define to silence OpenGL deprecation warnings */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#endif /* end __APPLE__ */

#ifdef WIN32

#include "GLFW/glfw3.h"

#endif /* end WIN32 */

/* Loads the shaders and returns the programID */
GLuint _loadShaders();

#endif