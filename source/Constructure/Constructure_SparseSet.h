#ifndef CONSTRUCTURE_SPARSESET_H
#define CONSTRUCTURE_SPARSESET_H

#include <limits.h> /* for UINT_MAX */

#include "PGUtil.h"

/* UINT_MAX will signal invalid index */
#define invalidSparseIndex UINT_MAX

/* A sparse set maps size_t to elements */
typedef struct SparseSet{
    size_t *_sparsePtr;
    void *_densePtr;
    /* need a mapping of dense to sparse for remove */
    size_t *_reflectPtr;
    size_t capacity;
    size_t _size;

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

    /* allocate reflect */
    toRet._reflectPtr = pgAlloc(
        capacity,
        sizeof(size_t)
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
    toRet._reflectPtr = pgAlloc(
        toRet.capacity,
        sizeof(size_t)
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
    memcpy(
        toRet._reflectPtr,
        toCopyPtr->_reflectPtr,
        toRet.capacity * sizeof(size_t)
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

    /* clear reflect */
    memset(
        setPtr->_reflectPtr,
        0,
        setPtr->capacity * sizeof(size_t)
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
            != invalidSparseIndex;
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
 * Returns a pointer to the element associated with
 * the given index in the given sparse set, or NULL
 * if no such element exists
 */
extern inline void *_sparseSetGetPtr(
    SparseSet *setPtr,
    size_t sparseIndex,
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

    size_t denseIndex 
        = setPtr->_sparsePtr[sparseIndex];
    return denseIndex == invalidSparseIndex
        ? NULL
        : voidPtrAdd(
            setPtr->_densePtr,
            elementSize * denseIndex
        );
}

#ifndef _DEBUG
/*
 * Returns a pointer to the element associated with
 * the given index in the given sparse set of the
 * specified type, or NULL if no such element exists
 */
#define sparseSetGetPtr( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX \
) \
    ((TYPENAME*)_sparseSetGetPtr( \
        SETPTR, \
        SPARSEINDEX, \
        sizeof(TYPENAME) \
    ))
#else
/*
 * Returns a pointer to the element associated with
 * the given index in the given sparse set of the
 * specified type, or NULL if no such element exists
 */
#define sparseSetGetPtr( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX \
) \
    ((TYPENAME*)_sparseSetGetPtr( \
        SETPTR, \
        SPARSEINDEX, \
        sizeof(TYPENAME), \
        #TYPENAME \
    ))
#endif

/*
 * Returns the value of the element associated with
 * the given index in the given sparse set of the
 * specified type
 */
#define sparseSetGet(TYPENAME, SETPTR, SPARSEINDEX) \
    ( \
        (TYPENAME) \
        (*sparseSetGetPtr( \
            TYPENAME, \
            SETPTR, \
            SPARSEINDEX \
        )) \
    )

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
    /* set reflect */
    setPtr->_reflectPtr[setPtr->_size] = sparseIndex;
    
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
#ifndef _DEBUG
/*
 * Sets the element associated with the given index
 * in the given sparse set of the specified type to
 * the given value
 */
#define sparseSetSet( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX, \
    VALUE\
) \
    do{ \
        assertTrue( \
            SPARSEINDEX < setPtr->capacity, \
            "bad index; " SRC_LOCATION \
        ) \
        ((TYPENAME *)((SETPTR)->_densePtr))[ \
            (SETPTR)->_size \
        ] = VALUE; \
        (SETPTR)->_reflectPtr[(SETPTR)->_size] \
            = (SPARSEINDEX) \
        (SETPTR)->_sparsePtr[SPARSEINDEX] \
            = (SETPTR)->_size; \
        ++((SETPTR)->_size); \
    } while(false)
#else
/*
 * Sets the element associated with the given index
 * in the given sparse set of the specified type to
 * the given value
 */
#define sparseSetSet( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX, \
    VALUE\
) \
    do{ \
        _sparseSetPtrTypeCheck(#TYPENAME, SETPTR); \
        assertTrue( \
            SPARSEINDEX < setPtr->capacity, \
            "bad index; " SRC_LOCATION \
        ) \
        ((TYPENAME *)((SETPTR)->_densePtr))[ \
            (SETPTR)->_size \
        ] = VALUE; \
        (SETPTR)->_reflectPtr[(SETPTR)->_size] \
            = (SPARSEINDEX) \
        (SETPTR)->_sparsePtr[SPARSEINDEX] \
            = (SETPTR)->_size; \
        ++((SETPTR)->_size); \
    } while(false)
#endif

/*
 * Removes the element associated with the given index
 * from the given sparse set; returns true if
 * successful, false otherwise
 */
extern inline bool _sparseSetRemove(
    SparseSet *setPtr,
    size_t sparseIndex,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    /* error if bad sparse index*/
    assertTrue(
        sparseIndex < setPtr->capacity,
        "bad index; " SRC_LOCATION
    );

    size_t denseIndex 
        = setPtr->_sparsePtr[sparseIndex];
    if(denseIndex >= setPtr->_size){
        return false;
    }

    /* invalidate sparse for the element to remove */
    setPtr->_sparsePtr[sparseIndex] 
        = invalidSparseIndex;
    
    --(setPtr->_size);
    /* if not removing end, swap in last element */
    if(setPtr->_size && denseIndex != setPtr->_size){
        size_t sparseIndexOfEnd 
            = setPtr->_reflectPtr[setPtr->_size];
        /* overwrite value */
        memcpy(
            voidPtrAdd(
                setPtr->_densePtr,
                denseIndex
            ),
            voidPtrAdd(
                setPtr->_densePtr,
                setPtr->_size
            ),
            elementSize
        );
        /* overwrite reflect */
        setPtr->_reflectPtr[denseIndex]
            = sparseIndexOfEnd;
        /* change sparse entry for previous end */
        setPtr->_sparsePtr[sparseIndexOfEnd]
            = denseIndex;
    }
    /* 
     * if remove end, no need to do anything since 
     * size already shrank
     */
    return true;
}

#ifndef _DEBUG
/*
 * Removes the element associated with the given index
 * from the given sparse set of the specified type; 
 * returns true if successful, false otherwise
 */
#define sparseSetRemove( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX \
) \
    _sparseSetRemove( \
        SETPTR, \
        SPARSEINDEX, \
        sizeof(TYPENAME) \
    )
#else
/*
 * Removes the element associated with the given index
 * from the given sparse set of the specified type; 
 * returns true if successful, false otherwise
 */
#define sparseSetRemove( \
    TYPENAME, \
    SETPTR, \
    SPARSEINDEX \
) \
    _sparseSetRemove( \
        SETPTR, \
        SPARSEINDEX, \
        sizeof(TYPENAME), \
        #TYPENAME \
    )
#endif

/*
 * Provides functionality for iterating over the
 * elements of a sparse set
 */
typedef struct SparseSetItr{
    /* Itr owns none of these; no need to free */
    const void *_densePtr;
    const size_t *_reflectPtr;
    const size_t _size;
    size_t _currentIndex;

    #ifdef _DEBUG
    const char *_typeName;
    #endif
} SparseSetItr;

/* Creates an iterator over the given sparse set */
extern inline SparseSetItr _sparseSetItr(
    SparseSet *setPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    SparseSetItr toRet = {
        setPtr->_densePtr,
        setPtr->_reflectPtr,
        setPtr->_size,
        0
        #ifdef _DEBUG
        ,typeName
        #endif
    };

    return toRet;
}
#ifndef _DEBUG
/*
 * Creates an iterator over the given sparse set
 * of the specified type
 */
#define sparseSetItr(TYPENAME, SETPTR) \
    _sparseSetItr(SETPTR)
#else
/*
 * Creates an iterator over the given sparse set
 * of the specified type
 */
#define sparseSetItr(TYPENAME, SETPTR) \
    _sparseSetItr(SETPTR, #TYPENAME)
#endif

//todo: iterator functionality
/*
 * Returns true if the given iterator has a next
 * element, false otherwise
 */
extern inline bool _sparseSetItrHasNext(
    const SparseSetItr *itrPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    //todo need a version to check the itr
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    return itrPtr->_currentIndex < itrPtr->_size;
}

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

#undef invalidSparseIndex

#endif