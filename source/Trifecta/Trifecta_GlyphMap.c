#include "Trifecta_GlyphMap.h"

/* 
 * Uses the integral value of the given wchar_t 
 * as its hash
 */
size_t wcharHash(const void *wcharPtr){
    return *((wchar_t*)wcharPtr);
}

/* 
 * Returns true if the two given wchar_t's are equal,
 * false otherwise
 */
bool wcharEquals(
    const void *wcharPtr1, 
    const void *wcharPtr2
){
    return *((wchar_t*)wcharPtr1)
        == *((wchar_t*)wcharPtr2);
}

#define _keyType wchar_t
#define _valueType TFSpriteInstruction*

/* Constructs and returns a TFGlyphMap by value */
TFGlyphMap tfGlyphMapMake(
    int horizontalSpacing,
    int verticalSpacing
){
    TFGlyphMap toRet = {0};
    toRet._glyphMap = hashMapMake(_keyType, _valueType,
        100,
        wcharHash,
        wcharEquals
    );
    toRet.horizontalSpacing = horizontalSpacing;
    toRet.verticalSpacing = verticalSpacing;
    return toRet;
}

/* 
 * Returns a const ptr to the TFspriteInstruction
 * associated with the given wide character or NULL
 * if no such association is present in the given
 * TFGlyphMap
 */
const TFSpriteInstruction *tfGlyphMapGet(
    TFGlyphMap *glyphMapPtr,
    wchar_t character
){
    /* if present, return associated sprite */
    if(hashMapHasKey(_keyType, _valueType,
        &(glyphMapPtr->_glyphMap),
        character
    )){
        return hashMapGet(_keyType, _valueType,
            &(glyphMapPtr->_glyphMap),
            character
        );
    }
    /* otherwise return NULL */
    else{
        return NULL;
    }
}

/*
 * Associates the given wide character to the
 * specified TFSpriteInstruction in the given
 * TFGlyphMap
 */
void tfGlyphMapPut(
    TFGlyphMap *glyphMapPtr,
    wchar_t character,
    TFSpriteInstruction *spriteInstrPtr
){
    hashMapPut(_keyType, _valueType,
        &(glyphMapPtr->_glyphMap),
        character,
        spriteInstrPtr
    );
}

/* Frees the specified TFGlyphMap */
void TFGlyphMapFree(TFGlyphMap *glyphMapPtr){
    hashMapFree(_keyType, _valueType,
        &(glyphMapPtr->_glyphMap)
    );
}

#undef _keyType
#undef _valueType