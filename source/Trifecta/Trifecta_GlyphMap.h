#ifndef TRIFECTA_GLYPHMAP
#define TRIFECTA_GLYPHMAP

#include "Constructure.h"

#include "Trifecta_Sprite.h"

/* 
 * Represents a mapping from wide characters to glyphs
 * (sprites)
 */
typedef struct TFGlyphMap{
    /* Maps wide characters to TFSpriteInstruction's */
    HashMap _glyphMap;
    int horizontalSpacing;
    int verticalSpacing;
} TFGlyphMap;

/* Constructs and returns a TFGlyphMap by value */
TFGlyphMap tfGlyphMapMake(
    int horizontalSpacing,
    int verticalSpacing
);

/* 
 * Returns a const ptr to the TFSpriteInstruction
 * associated with the given wide character or NULL
 * if no such association is present in the given
 * TFGlyphMap
 */
const TFSpriteInstruction *tfGlyphMapGet(
    TFGlyphMap *glyphMapPtr,
    wchar_t character
);

/*
 * Associates the given wide character to the
 * specified TFSpriteInstruction in the given
 * TFGlyphMap
 */
void tfGlyphMapPut(
    TFGlyphMap *glyphMapPtr,
    wchar_t character,
    TFSpriteInstruction *spriteInstrPtr
);

/* Frees the specified TFGlyphMap */
void TFGlyphMapFree(TFGlyphMap *glyphMapPtr);

#endif