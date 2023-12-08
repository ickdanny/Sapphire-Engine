#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define CONSTRUCTURE_STRING(TYPENAME, PREFIX, CHARTYPE)

typedef char CHARTYPE; //todo remove

typedef struct TYPENAME{
    CHARTYPE *ptr;
    int length; //not including null terminator
} TYPENAME;

extern inline size_t _cLength(const CHARTYPE *cStringPtr){
    size_t charCount = 0;
    while(*cStringPtr != 0){ //assumes sentinel value = 0
        ++cStringPtr;
        ++charCount;
    }
    return charCount;
}

extern inline TYPENAME *_cCopy(
    CHARTYPE *destPtr, 
    const CHARTYPE *sourcePtr
){
    CHARTYPE *toRet = destPtr;
    while(*sourcePtr != 0){ //assumes sentinel value = 0
        *destPtr = *sourcePtr;
        ++destPtr;
        ++sourcePtr;
    }
    return toRet;
}

extern inline TYPENAME Make(const CHARTYPE *cStringPtr){
    TYPENAME toRet = {0};
    toRet.length = _cLength(cStringPtr);  //todo: what if wchar?
    toRet.ptr = calloc(toRet.length + 1, sizeof(CHARTYPE));
    assert(toRet.ptr && "calloc check");
    _cCopy(toRet.ptr, cStringPtr);
    return toRet;
}

//todo: more funcs

extern inline void Free(TYPENAME *strPtr){
    free(strPtr->ptr);
    strPtr->ptr = NULL;
    strPtr->length = 0;
}