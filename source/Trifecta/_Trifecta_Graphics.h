#ifndef TRIFECTA_GRAPHICS_H
#define TRIFECTA_GRAPHICS_H

/* define to silence OpenGL deprecation warnings */
#define GL_SILENCE_DEPRECATION

#include <OpenGL/gl3.h>

/*
 * A Graphics object is capable of drawing graphics to
 * the screen
 */
typedef struct _TFGraphics{
    int _graphicsWidth;
    int _graphicsHeight;
    GLuint _vaoID;
    GLuint _programID;
} _TFGraphics;

/* 
 * Constructs, initializes, and returns a new
 * _TFGraphics object by value
 */
_TFGraphics _tfGraphicsMake(
    int graphicsWidth,
    int graphicsHeight
);

//todo: temp function
void testDraw(_TFGraphics *graphicsPtr);

/* Frees the specified _TFGraphics */
void _tfGraphicsFree(_TFGraphics *graphicsPtr);

#endif