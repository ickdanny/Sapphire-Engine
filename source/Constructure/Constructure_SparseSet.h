#ifndef CONSTRUCTURE_SPARSESET_H
#define CONSTRUCTURE_SPARSESET_H

#include "PGUtil.h"

/* A sparse set maps size_t to elements */
typedef struct SparseSet{
    /* UINT_MAX will signal invalid index */
    size_t *_sparsePtr;
    void *_densePtr;
    size_t size;

    #ifdef _DEBUG
    /* 
     * Should only ever point to a string literal,
     * thus should not be freed
     */
    const char *_typeName;
    #endif
} SparseSet;

#ifdef _DEBUG
/*
 * Asserts that the given type matches that of the 
 * given sparse set pointer
 */
#define _sparseSetPtrTypeCheck(TYPENAME, SETPTR) \
    assertStringEqual( \
        TYPENAME, \
        (SETPTR)->_typeName, \
        "bad sparse set type; " SRC_LOCATION \
    )
#endif

/* Creates a sparse set and returns it by value */
extern inline SparseSet _sparseSetMake(
    size_t size,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    assertTrue(
        size > 0u, 
        "size cannot be 0; " SRC_LOCATION
    );
    SparseSet toRet = {0};

    /* init sparse to invalid */
    toRet._sparsePtr = pgAlloc(
        size, 
        sizeof(size_t)
    );
    memset(
        toRet._sparsePtr, 
        0xFF, 
        size * sizeof(size_t)
    );

    /* allocate dense */
    toRet._densePtr = pgAlloc(
        size,
        elementSize
    );

    toRet.size = size;

    #ifdef _DEBUG
    toRet._typeName = typeName;
    #endif

    return toRet;
}

#ifndef _DEBUG
/* 
 * Creates a sparse set of the specified type
 * and returns it by value
 */
#define sparseSetMake(TYPENAME, SIZE) \
    _sparseSetMake(SIZE, sizeof(TYPENAME))
#else
/* 
 * Creates a sparse set of the specified type
 * and returns it by value
 */
#define sparseSetMake(TYPENAME, SIZE) \
    _sparseSetMake(SIZE, sizeof(TYPENAME), #TYPENAME)
#endif

/*
 * Makes a one level deep copy of the given
 * sparse set and returns it by value
 */
extern inline SparseSet _sparseSetCopy(
    size_t elementSize,
    const SparseSet *toCopyPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, toCopyPtr);
    #endif

    SparseSet toRet = {0};
    toRet.size = toCopyPtr->size;
    toRet._sparsePtr = pgAlloc(
        toRet.size,
        sizeof(size_t)
    );
    toRet._densePtr = pgAlloc(
        toRet.size,
        elementSize
    );

    /* may overflow */
    memcpy(
        toRet._sparsePtr,
        toCopyPtr->_sparsePtr,
        toRet.size * sizeof(size_t)
    );
    memcpy(
        toRet._densePtr,
        toCopyPtr->_densePtr,
        toRet.size * elementSize
    );

    #ifdef _DEBUG
    /* safe to shallow copy; it is a literal */
    toRet._typeName = toCopyPtr->_typeName;
    #endif

    return toRet;
}

#ifndef _DEBUG
/*
 * Makes a one level deep copy of the given
 * sparse set of the specified type and returns
 * it by value
 */
#define sparseSetCopy(TYPENAME, TOCOPYPTR) \
    _sparseSetCopy(sizeof(TYPENAME), TOCOPYPTR)
#else
/*
 * Makes a one level deep copy of the given
 * sparse set of the specified type and returns
 * it by value
 */
#define sparseSetCopy(TYPENAME, TOCOPYPTR) \
    _sparseSetCopy( \
        sizeof(TYPENAME), \
        TOCOPYPTR, \
        #TYPENAME \
    )
#endif

/* 
 * Clears the given sparse set, removing all key
 * associations and zeroing its data
 */
extern inline void _sparseSetClear(
    SparseSet *setPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    /* invalidate sparse keys */
    memset(
        setPtr->_sparsePtr, 
        0xFF, 
        setPtr->size * sizeof(size_t)
    );

    /* clear dense */
    memset(
        setPtr->_densePtr,
        0,
        setPtr->size * elementSize
    );
}

#ifndef _DEBUG
/* 
 * Clears the given sparse set of the specified type,
 * removing all key associations and zeroing its data
 */
#define sparseSetClear(TYPENAME, SETPTR) \
    _sparseSetClear(SETPTR, sizeof(TYPENAME))
#else
/* 
 * Clears the given sparse set of the specified type,
 * removing all key associations and zeroing its data
 */
#define sparseSetClear(TYPENAME, SETPTR) \
    _sparseSetClear( \
        SETPTR, \
        sizeof(TYPENAME), \
        #TYPENAME \
    )
#endif

/*
 * contains
 * set
 * setPtr
 * get
 * getPtr
 * remove
 * iteration somehow
 * free
 */

#endif