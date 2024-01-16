#ifndef CONSTRUCTURE_SPARSESET_H
#define CONSTRUCTURE_SPARSESET_H

#include <limits.h> /* for UINT_MAX */

#include "PGUtil.h"

/* A sparse set maps size_t to elements */
typedef struct SparseSet{
    /* UINT_MAX will signal invalid index */
    size_t *_sparsePtr;
    void *_densePtr;
    size_t capacity;
    size_t _size;

    /* 
     * This type will not include a denseToSparse
     * array; one can achieve the same effect via
     * a sparse set of size_t
     */

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
    size_t capacity,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    assertTrue(
        capacity > 0u, 
        "capacity cannot be 0; " SRC_LOCATION
    );
    SparseSet toRet = {0};

    /* init sparse to invalid */
    toRet._sparsePtr = pgAlloc(
        capacity, 
        sizeof(size_t)
    );
    memset(
        toRet._sparsePtr, 
        0xFF, 
        capacity * sizeof(size_t)
    );

    /* allocate dense */
    toRet._densePtr = pgAlloc(
        capacity,
        elementSize
    );

    toRet.capacity = capacity;
    toRet._size = 0u;

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
    toRet.capacity = toCopyPtr->capacity;
    toRet._size = toCopyPtr->_size;
    toRet._sparsePtr = pgAlloc(
        toRet.capacity,
        sizeof(size_t)
    );
    toRet._densePtr = pgAlloc(
        toRet.capacity,
        elementSize
    );

    /* may overflow */
    memcpy(
        toRet._sparsePtr,
        toCopyPtr->_sparsePtr,
        toRet.capacity * sizeof(size_t)
    );
    memcpy(
        toRet._densePtr,
        toCopyPtr->_densePtr,
        toRet.capacity * elementSize
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
        setPtr->capacity * sizeof(size_t)
    );

    /* clear dense */
    memset(
        setPtr->_densePtr,
        0,
        setPtr->capacity * elementSize
    );

    setPtr->_size = 0u;
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
 * Returns true if the given sparse set contains an
 * element associated with the specified index,
 * false otherwise
 */
extern inline bool _sparseSetContains(
    const SparseSet *setPtr,
    size_t sparseIndex
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    return sparseIndex < setPtr->capacity
        && setPtr->_sparsePtr[sparseIndex] 
            != UINT_MAX;
}

#ifndef _DEBUG
/* 
 * Returns true if the given sparse set of the
 * specified type contains an element associated
 * with the specified index, false otherwise
 */
#define sparseSetContains( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX \
) \
    _sparseSetContains(SETPTR, SPARSEINDEX)
#else
/* 
 * Returns true if the given sparse set of the
 * specified type contains an element associated
 * with the specified index, false otherwise
 */
#define sparseSetContains( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX \
) \
    _sparseSetContains(SETPTR, SPARSEINDEX, #TYPENAME)
#endif

/* 
 * Copies the specified value into the element 
 * associated with the given index in the given
 * sparse set
 */
extern inline void _sparseSetSetPtr(
    SparseSet *setPtr,
    size_t sparseIndex,
    void *valuePtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    /* error if bad index*/
    assertTrue(
        sparseIndex < setPtr->capacity,
        "bad index; " SRC_LOCATION
    );

    /* copy the value into dense */
    /* memcpy safe; elements shouldn't overlap */
    memcpy(
        voidPtrAdd(
            setPtr->_densePtr,
            elementSize * setPtr->_size
        ),
        valuePtr, 
        elementSize
    );
    
    /* set sparse */
    setPtr->_sparsePtr[sparseIndex] = setPtr->_size;
    ++(setPtr->_size);
}

#ifndef _DEBUG
/* 
 * Copies the specified value into the element 
 * associated with the given index in the given
 * sparse set of the specified type
 */
#define sparseSetSetPtr( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX, \
    VALUEPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _sparseSetSetPtr( \
            SETPTR, \
            SPARSEINDEX, \
            VALUEPTR, \
            sizeof(TYPENAME) \
        ) \
    )
#else
/* 
 * Copies the specified value into the element 
 * associated with the given index in the given
 * sparse set of the specified type
 */
#define sparseSetSetPtr( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX, \
    VALUEPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _sparseSetSetPtr( \
            SETPTR, \
            SPARSEINDEX, \
            VALUEPTR, \
            sizeof(TYPENAME), \
            #TYPENAME \
        ) \
    )
#endif

/* 
 * Sparse set setting must be done via macro because
 * it expects values not pointers
 */

/*
 * set
 * get
 * getPtr
 * remove
 * iteration somehow
 */

/* Frees the given sparse set */
extern inline void _sparseSetFree(
    SparseSet *setPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    pgFree(setPtr->_sparsePtr);
    pgFree(setPtr->_densePtr);
    setPtr->capacity = 0u;
    setPtr->_size = 0u;
}

#ifndef _DEBUG
/* Frees the given sparse set of the specified type */
#define sparseSetFree(TYPENAME, SETPTR) \
    _sparseSetFree(SETPTR)
#else
/* Frees the given sparse set of the specified type */
#define sparseSetFree(TYPENAME, SETPTR) \
    _sparseSetFree(SETPTR, #TYPENAME)
#endif

#endif