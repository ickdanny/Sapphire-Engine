#ifndef TRIFECTA_GRAPHICS_H
#define TRIFECTA_GRAPHICS_H

#ifdef __APPLE__

/* define to silence OpenGL deprecation warnings */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>

#endif /* end __APPLE__ */

#ifdef WIN32

#include "GLFW/glfw3.h"

#endif /* end WIN32 */

#include "ZMath.h"
#include "Trifecta_Sprite.h"
#include "Trifecta_GlyphMap.h"

/*
 * A Graphics object is capable of drawing graphics to
 * the screen
 */
typedef struct _TFGraphics{
    int _graphicsWidth;
    int _graphicsHeight;
    GLuint _vaoID;
    GLuint _vertexBufferID;
    GLuint _texCoordBufferID;
    GLuint _programID;
    GLuint _transformID;
    GLuint _samplerID;
} _TFGraphics;

/* 
 * Constructs, initializes, and returns a new
 * _TFGraphics object by value
 */
_TFGraphics _tfGraphicsMake(
    int graphicsWidth,
    int graphicsHeight
);

/* Draws a sprite with the specified _TFGraphics */
void _tfGraphicsDrawSprite(
    _TFGraphics *graphicsPtr,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr
);

/*
 * Draws a portion of a sprite with the specified
 * _TFGraphics
 */
void _tfGraphicsDrawSubSprite(
    _TFGraphics *graphicsPtr,
    Point2D preOffsetCenter,
    const TFSpriteInstruction *spriteInstrPtr,
    const Rectangle *srcRectPtr
);

/* 
 * Draws a tiled sprite with the specified _TFGraphics
 */
void _tfGraphicsDrawTileSprite(
    _TFGraphics *graphicsPtr,
    const Rectangle *drawRectPtr,
    const TFSpriteInstruction *spriteInstrPtr,
    Point2D pixelOffset
);

/* 
 * Draws text (as a WideString) with the specified
 * _TFGraphics
 */
void _tfGraphicsDrawText(
    _TFGraphics *graphicsPtr,
    Point2D pos,
    const WideString *textPtr,
    int rightBound,
    TFGlyphMap *glyphMapPtr
);

/* Frees the specified _TFGraphics */
void _tfGraphicsFree(_TFGraphics *graphicsPtr);

#endif