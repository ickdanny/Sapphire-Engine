#ifndef CONSTRUCTURE_STRING_H
#define CONSTRUCTURE_STRING_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "PGUtil.h"

/*
 * Unlike the generic containers in the rest
 * of Constructure, the String types will
 * only accept integral character types and
 * will have a different container type for
 * each - implemented via this type-declare 
 * macro:
 */
#define CONSTRUCTURE_STRING_DECL( \
    TYPENAME, \
    PREFIX, \
    CHARTYPE \
)

typedef char CHARTYPE; //todo remove

typedef struct TYPENAME{
    CHARTYPE *_ptr;

    /* does not include null terminator */
    size_t length; 
} TYPENAME;

/*
 * Counts the number of characters in a null
 * terminated C string of the character type,
 * not including the null terminator itself
 */
extern inline size_t _cStringLength(
    const CHARTYPE *cStringPtr
){
    assertNotNull(
        cStringPtr,
        "null in _cStringLength; "
        SRC_LOCATION
    );

    size_t charCount = 0;

    /* assumes null terminated */
    while(*cStringPtr){ 
        ++cStringPtr;
        ++charCount;
    }
    return charCount;
}

/*
 * Given two null terminated C strings of the
 * character type, copies the source string
 * into the destination string including
 * the null terminator
 */
extern inline CHARTYPE *_cStringCopy(
    CHARTYPE *destPtr, 
    const CHARTYPE *sourcePtr
){
    assertNotNull(
        destPtr,
        "null dest in _cStringLength; "
        SRC_LOCATION
    );
    assertNotNull(
        sourcePtr,
        "null source in _cStringLength; "
        SRC_LOCATION
    );

    CHARTYPE *toRet = destPtr;

    /* assumes null terminated */
    while(*sourcePtr){
        *destPtr = *sourcePtr;
        ++destPtr;
        ++sourcePtr;
    }
    destPtr = 0;
    return toRet;
}

/*
 * Creates a PREFIX copy of the given raw
 * string and returns it by value
 */
extern inline TYPENAME makeFromC(
    const CHARTYPE *cStringPtr
){
    TYPENAME toRet = {0};
    toRet.length = _cStringLength(cStringPtr);
    toRet._ptr = pgAlloc(
        toRet.length + 1, 
        sizeof(CHARTYPE)
    );
    _cStringCopy(toRet._ptr, cStringPtr);
    return toRet;
}

//todo: make empty ctor?

//todo: more funcs
/*
 * compare
 * equals
 * copyInto
 * makeCopy
 * append
 * pushBack
 * popBack
 * assign
 * insert
 * erase
 * charAt
 * back
 * front
 * indexOf(char or string)
 * lastIndexOf(char or string)
 * isEmpty
 * substring
 * beginsWith
 * endsWith
 * tokenize (?)
 * swap
 */

/* Frees the given PREFIX */
extern inline void Free(TYPENAME *strPtr){
    pgFree(strPtr->_ptr);
    strPtr->length = 0;
}

#endif