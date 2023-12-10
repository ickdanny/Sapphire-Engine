#ifndef CONSTRUCTURE_STRING

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "PGUtil.h"

#define CONSTRUCTURE_STRING(TYPENAME, PREFIX, CHARTYPE)

typedef char CHARTYPE; //todo remove

typedef struct TYPENAME{
    CHARTYPE *ptr;
    int length; /* does not include null terminator */
} TYPENAME;

extern inline size_t _cLength(const CHARTYPE *cStringPtr){
    size_t charCount = 0;
    while(*cStringPtr != 0){ /* assumes null terminated */
        ++cStringPtr;
        ++charCount;
    }
    return charCount;
}

extern inline CHARTYPE *_cCopy(
    CHARTYPE *destPtr, 
    const CHARTYPE *sourcePtr
){
    CHARTYPE *toRet = destPtr;
    while(*sourcePtr != 0){ /* assumes null terminated */
        *destPtr = *sourcePtr;
        ++destPtr;
        ++sourcePtr;
    }
    return toRet;
}

extern inline TYPENAME MakeFromC(const CHARTYPE *cStringPtr){
    TYPENAME toRet = {0};
    toRet.length = _cLength(cStringPtr);  //todo: what if wchar?
    toRet.ptr = pgAlloc(toRet.length + 1, sizeof(CHARTYPE));
    _cCopy(toRet.ptr, cStringPtr);
    return toRet;
}

//todo: make empty

//todo: more funcs

extern inline void Free(TYPENAME *strPtr){
    pgFree(strPtr->ptr);
    strPtr->length = 0;
}

#endif