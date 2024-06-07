#ifndef TRIFECTA_SPRITE
#define TRIFECTA_SPRITE

#include "ZMath.h"

#define _minScale 0.01f
#define _defaultRotation 0.0f
#define _defaultScale 1.0f

/* Represents a 2D image */
typedef struct TFSprite{
    //todo: OpenGL texture
    uint32_t width;
    uint32_t height;
} TFSprite;

/* Specifies how a sprite is to be drawn */
typedef struct TFSpriteInstruction{
    TFSprite *spritePtr;
    int depth;
    Vector2D offset;
    float rotation;
    float scale;
} TFSpriteInstruction;

/* 
 * Constructs and returns a TFSpriteInstruction
 * by value
 */
TFSpriteInstruction tfSpriteInstructionMake(
    TFSprite *spritePtr,
    int depth,
    Vector2D offset,
    float rotation,
    float scale
);

/* 
 * Constructs and returns a TFSpriteInstruction
 * by value
 */
#define tfSpriteInstructionMakeSimple( \
    SPRITEPTR, \
    DEPTH, \
    OFFSET \
) \
    tfSpriteInstructionMake( \
        (SPRITEPTR), \
        (DEPTH), \
        (OFFSET), \
        _defaultRotation, \
        _defaultScale \
    )

#endif