#include "Trifecta_Sprite.h"

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
){
    TFSpriteInstruction toRet = {0};
    toRet.spritePtr = spritePtr;
    toRet.depth = depth;
    toRet.offset = offset;
    toRet.rotation = rotation;
    toRet.scale = scale;
    return toRet;
}