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

/*
 * A wrapper for a continguous null terminated
 * fixed width character array on the heap
 */
typedef struct TYPENAME{
    CHARTYPE *_ptr;

    /* does not include null terminator */
    size_t length; 

    size_t _capacity;
} TYPENAME;

/*
 * Counts the number of characters in a null
 * terminated C string of the character type,
 * not including the null terminator itself
 */
extern inline size_t _cLength(
    const CHARTYPE *cStringPtr
){
    assertNotNull(
        cStringPtr,
        "null in _cLength; "
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
 * Creates a string copy of the given null
 * terminated C string and returns it by value
 */
extern inline TYPENAME makeC(
    const CHARTYPE *cStringPtr
){
    TYPENAME toRet = {0};
    toRet.length = _cLength(cStringPtr);
    toRet._capacity = toRet.length + 1;
    toRet._ptr = (CHARTYPE *)pgAlloc(
        toRet._capacity, 
        sizeof(CHARTYPE)
    );

    /* 
     * use memcpy instead of a custom
     * _cStringCopy since the lengths of strings
     * are already stored
     */
    memcpy(
        toRet._ptr, 
        cStringPtr, 
        toRet._capacity * sizeof(CHARTYPE)
    );
    return toRet;
}

/*
 * Creates and allocates space for an
 * empty string
 */
extern inline TYPENAME makeEmpty(){
    #define stringInitCapacity 16u;

    TYPENAME toRet = {0};
    toRet.length = 0;
    toRet._capacity = stringInitCapacity;
    toRet._ptr = (CHARTYPE *)pgAlloc(
        toRet._capacity, 
        sizeof(CHARTYPE)
    );
    return toRet;

    #undef stringInitCapacity
}

/*
 * Makes a copy of the given string and returns
 * it by value
 */
extern inline TYPENAME copy(
    const TYPENAME *toCopyPtr
){
    TYPENAME toRet = {0};
    toRet.length = toCopyPtr->length;

    /* 
     * allocate just enough space for 
     * the copy 
     */
    toRet._capacity = toRet.length + 1;
    toRet._ptr = (CHARTYPE *)pgAlloc(
        toRet._capacity, 
        sizeof(CHARTYPE)
    );

    /* 
     * use memcpy instead of a custom
     * _cStringCopy since the lengths of strings
     * are already stored
     */
    memcpy(
        toRet._ptr, 
        toCopyPtr->_ptr, 
        toRet._capacity * sizeof(CHARTYPE)
    );

    return toRet;
}

/*
 * Copies the contents of the source string
 * into the destination string
 */
extern inline void copyInto(
    TYPENAME *destPtr,
    const TYPENAME *srcPtr
){
    /* allocate more space if needed */
    if(destPtr->_capacity <= srcPtr->length){
        destPtr->_capacity = srcPtr->length + 1;
        destPtr->_ptr = (CHARTYPE *)pgRealloc(
            destPtr->_ptr,
            destPtr->_capacity,
            sizeof(CHARTYPE)
        );
    }

    /* 
     * use memcpy instead of a custom
     * _cStringCopy since the lengths of strings
     * are already stored
     */
    memcpy(
        destPtr->_ptr, 
        srcPtr->_ptr, 
        destPtr->_capacity * sizeof(CHARTYPE)
    );
}

/*
 * Reallocates more space for the given string 
 * if its capacity does not fit its nominal length 
 */
extern inline void _reallocToFitNewLength(
    TYPENAME *stringPtr
){
    if(stringPtr->length >= stringPtr->_capacity){
        stringPtr->_capacity 
            = stringPtr->length + 1;
        stringPtr->_ptr = (CHARTYPE *)pgRealloc(
            stringPtr->_ptr,
            stringPtr->_capacity,
            sizeof(CHARTYPE)
        );
    }
}

/* 
 * Prepends the given null terminated C string
 * onto the front of the given string
 */
extern inline void prependC(
    TYPENAME *stringPtr,
    const CHARTYPE *toPrependPtr
){
    size_t toPrependLength = _cLength(
        toPrependPtr
    );
    if(!toPrependLength){
        return;
    }

    size_t originalLength = stringPtr->length;
    stringPtr->length += toPrependLength;

    /* allocate more space if needed */
    _reallocToFitNewLength(stringPtr);

    memmove(
        stringPtr->_ptr + toPrependLength,
        stringPtr->_ptr,
        /* +1 to copy null */
        originalLength + 1
    );
    /* use memcpy; length is known */
    memcpy(
        stringPtr->_ptr, 
        toPrependPtr,
        /* do not +1; do not copy null*/
        toPrependLength * sizeof(CHARTYPE)
    );
}

/* 
 * Appends the second given string onto the
 * front of the first
 */
extern inline void prepend(
    TYPENAME *stringPtr,
    TYPENAME *toPrependPtr
){
    if(!(toPrependPtr->length)){
        return;
    }

    size_t originalLength = stringPtr->length;
    stringPtr->length += toPrependPtr->length;

    /* allocate more space if needed */
    _reallocToFitNewLength(stringPtr);

    memmove(
        stringPtr->_ptr + toPrependPtr->length,
        stringPtr->_ptr,
        /* +1 to copy null */
        originalLength + 1
    );
    /* use memcpy; length is known */
    memcpy(
        stringPtr->_ptr, 
        toPrependPtr,
        /* do not +1; do not copy null*/
        toPrependPtr->length * sizeof(CHARTYPE)
    );
}

/*
 * Appends the given null terminated C string
 * onto the back of the given string
 */
extern inline void appendC(
    TYPENAME *stringPtr,
    const CHARTYPE *toAppendPtr
){
    size_t toAppendLength = _cLength(
        toAppendPtr
    );
    if(!toAppendLength){
        return;
    }

    size_t originalLength = stringPtr->length;
    stringPtr->length += toAppendLength;

    /* allocate more space if needed */
    _reallocToFitNewLength(stringPtr);

    /* use memcpy; length is known */
    memcpy(
        stringPtr->_ptr + originalLength,
        toAppendPtr,
        /* +1 to copy null */
        (toAppendLength + 1) * sizeof(CHARTYPE)
    );
}

/*
 * Appends the second given string onto the back
 * of the first
 */
extern inline void append(
    TYPENAME *stringPtr,
    TYPENAME *toAppendPtr
){
    if(!(toAppendPtr->length)){
        return;
    }

    size_t originalLength = stringPtr->length;
    stringPtr->length += toAppendPtr->length;

    /* allocate more space if needed */
    _reallocToFitNewLength(stringPtr);

    /* use memcpy; length is known */
    memcpy(
        stringPtr->_ptr + originalLength,
        toAppendPtr->_ptr,
        /* +1 to copy null */
        (toAppendPtr->length + 1) 
            * sizeof(CHARTYPE)
    );
}

/*
 * Compares the two given null terminated C
 * strings of the character type and returns 
 * >0 if the first is greater than the second, 
 * <0 if the second is greater than the first,
 * or 0 if the two are equal.
 */
extern inline int _cCompare(
    const CHARTYPE *cStringPtr1,
    const CHARTYPE *cStringPtr2
){
    assertNotNull(
        cStringPtr1,
        "null str1 in _cCompare; "
        SRC_LOCATION
    );
    assertNotNull(
        cStringPtr2,
        "null str2 in _cCompare; "
        SRC_LOCATION
    );

    /* assumes null terminated */
    while(
        *cStringPtr1 
        && (*cStringPtr1 == *cStringPtr2)
    ){
        ++cStringPtr1;
        ++cStringPtr2;
    }

    /*
     * Since 1 > 2 is disjoint from 2 > 1,
     * only operand can be 1; if 1 = 2, then
     * returns 0
     */
    return (*cStringPtr1 > *cStringPtr2) 
        - (*cStringPtr2 > *cStringPtr1);
}

/*
 * Compares the two given strings and returns 
 * >0 if the first is greater than the second, 
 * <0 if the second is greater than the first,
 * or 0 if the two are equal.
 */
extern inline int compare(
    const TYPENAME *stringPtr1,
    const TYPENAME *stringPtr2
){
    return _cCompare(
        stringPtr1->_ptr,
        stringPtr2->_ptr
    );
}

/*
 * Returns true if the two given strings are
 * equal, false otherwise
 */
extern inline bool equals(
    const TYPENAME *stringPtr1,
    const TYPENAME *stringPtr2
){
    /* if lengths unequal, clearly unequal */
    if(stringPtr1->length 
        != stringPtr2->length
    ){
        return false;
    }

    /* 
     * logical NOT on cStringCompare to
     * map 0 to true (1) and !0 to false (0)
     */
    return !_cCompare(
        stringPtr1->_ptr,
        stringPtr2->_ptr
    );
}

/*
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
    strPtr->_capacity = 0;
}

#endif