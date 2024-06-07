#ifndef TRIFECTA_GRAPHICS_H
#define TRIFECTA_GRAPHICS_H

/* define to silence OpenGL deprecation warnings */
#define GL_SILENCE_DEPRECATION

#include <OpenGL/gl.h>

/*
 * A Graphics object is capable of drawing graphics to
 * the screen
 */
typedef struct _TFGraphics{
    int _graphicsWidth;
    int _graphicsHeight;
} TFGraphics;

/* 
 * Constructs, initializes, and returns a new
 * TFGraphics object by value
 */
TFGraphics _tfGraphicsMake(
    int graphicsWidth,
    int graphicsHeight
);

//todo: temp function
void testDraw(TFGraphics *graphicsPtr);

#endif