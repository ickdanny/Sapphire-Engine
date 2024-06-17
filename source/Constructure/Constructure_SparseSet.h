#ifndef CONSTRUCTURE_SPARSESET_H
#define CONSTRUCTURE_SPARSESET_H

#include "PGUtil.h"

/* A sparse set maps size_t to elements */
typedef struct SparseSet{
    size_t *_sparsePtr;

    //todo: make this dynamically growable
    void *_densePtr;
    /* need a mapping of dense to sparse for remove */
    size_t *_reflectPtr;
    /* The number of sparse indices supported */
    size_t sparseCapacity;
    /* Internal capacity of dense and reflect arrays */
    size_t _denseCapacity;
    /* Current number of elements*/
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
/*
 * Asserts that the given type matches that of the 
 * given sparse set iterator pointer
 */
#define _sparseSetItrPtrTypeCheck(TYPENAME, ITRPTR) \
    assertStringEqual( \
        TYPENAME, \
        (ITRPTR)->_typeName, \
        "bad sparse set itr type; " SRC_LOCATION \
    )
#endif

/* Creates a sparse set and returns it by value */
SparseSet _sparseSetMake(
    size_t sparseCapacity,
    size_t initDenseCapacity,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

#ifndef _DEBUG
/* 
 * Creates a sparse set of the specified type
 * and returns it by value
 */
#define sparseSetMake( \
    TYPENAME, \
    SPARSECAPACITY, \
    INITDENSECAPACITY \
) \
    _sparseSetMake( \
        SPARSECAPACITY, \
        INITDENSECAPACITY, \
        sizeof(TYPENAME) \
    )
#else
/* 
 * Creates a sparse set of the specified type
 * and returns it by value
 */
#define sparseSetMake( \
    TYPENAME, \
    SPARSECAPACITY, \
    INITDENSECAPACITY \
) \
    _sparseSetMake( \
        SPARSECAPACITY, \
        INITDENSECAPACITY, \
        sizeof(TYPENAME), \
        #TYPENAME \
    )
#endif

/*
 * Makes a one level deep copy of the given
 * sparse set and returns it by value
 */
SparseSet _sparseSetCopy(
    size_t elementSize,
    const SparseSet *toCopyPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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
void _sparseSetClear(
    SparseSet *setPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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
bool _sparseSetContains(
    const SparseSet *setPtr,
    size_t sparseIndex
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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
void *_sparseSetGetPtr(
    SparseSet *setPtr,
    size_t sparseIndex,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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
 * Grows the given sparse set if it is at capacity;
 * returns false as error code, true otherwise
 */
bool _sparseSetGrowIfNeeded(
    SparseSet *setPtr,
    size_t elementSize
);

/* 
 * Copies the specified value into the element 
 * associated with the given index in the given
 * sparse set
 */
void _sparseSetSetPtr(
    SparseSet *setPtr,
    size_t sparseIndex,
    void *valuePtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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
    _Generic(*VALUEPTR, \
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
            SPARSEINDEX < (SETPTR)->sparseCapacity, \
            "bad index; " SRC_LOCATION \
        ); \
        assertTrue( \
            _sparseSetGrowIfNeeded( \
                SETPTR, \
                sizeof(TYPENAME) \
            ), \
            "sparse set failed to grow; " \
            SRC_LOCATION \
        ); \
        ((TYPENAME *)((SETPTR)->_densePtr))[ \
            (SETPTR)->_size \
        ] = VALUE; \
        (SETPTR)->_reflectPtr[(SETPTR)->_size] \
            = (SPARSEINDEX); \
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
            SPARSEINDEX < (SETPTR)->sparseCapacity, \
            "bad index; " SRC_LOCATION \
        ); \
        assertTrue( \
            _sparseSetGrowIfNeeded( \
                SETPTR, \
                sizeof(TYPENAME) \
            ), \
            "sparse set failed to grow; " \
            SRC_LOCATION \
        ); \
        ((TYPENAME *)((SETPTR)->_densePtr))[ \
            (SETPTR)->_size \
        ] = VALUE; \
        (SETPTR)->_reflectPtr[(SETPTR)->_size] \
            = (SPARSEINDEX); \
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
bool _sparseSetRemove(
    SparseSet *setPtr,
    size_t sparseIndex,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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
    void *_densePtr;
    size_t *_reflectPtr;
    size_t _size;
    size_t _currentIndex;

    #ifdef _DEBUG
    const char *_typeName;
    #endif
} SparseSetItr;

/* Creates an iterator over the given sparse set */
SparseSetItr _sparseSetItr(
    SparseSet *setPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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

/*
 * Returns true if the given iterator has a next
 * element, false otherwise
 */
bool _sparseSetItrHasNext(
    const SparseSetItr *itrPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

#ifndef _DEBUG
/*
 * Returns true if the given iterator of the specified
 * type has a next element, false otherwise
 */
#define sparseSetItrHasNext(TYPENAME, ITRPTR) \
    _sparseSetItrHasNext(ITRPTR)
#else
/*
 * Returns true if the given iterator of the specified
 * type has a next element, false otherwise
 */
#define sparseSetItrHasNext(TYPENAME, ITRPTR) \
    _sparseSetItrHasNext(ITRPTR, #TYPENAME)
#endif

/*
 * Returns a pointer to the next element from the
 * given iterator, or NULL if no such element exists
 */
void *_sparseSetItrNextPtr(
    SparseSetItr *itrPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

#ifndef _DEBUG
/*
 * Returns a pointer to the next element from the
 * given iterator of the specified type, or NULL if no
 * such element exists
 */
#define sparseSetItrNextPtr(TYPENAME, ITRPTR) \
    ((TYPENAME*)_sparseSetItrNextPtr( \
        ITRPTR, \
        sizeof(TYPENAME) \
    ))
#else
/*
 * Returns a pointer to the next element from the
 * given iterator of the specified type, or NULL if no
 * such element exists
 */
#define sparseSetItrNextPtr(TYPENAME, ITRPTR) \
    ((TYPENAME*)_sparseSetItrNextPtr( \
        ITRPTR, \
        sizeof(TYPENAME), \
        #TYPENAME \
    ))
#endif

/*
 * Returns the value of the next element from the
 * given iterator of the specified type
 */
#define sparseSetItrNext(TYPENAME, ITRPTR) \
    ( \
        (TYPENAME) \
        (*sparseSetItrNextPtr( \
            TYPENAME, \
            ITRPTR \
        )) \
    )

/*
 * Returns the sparse index of the previous element
 * iterated over by the given iterator; returns
 * the invalid index if no elements have been iterated
 * over
 */
size_t _sparseSetItrIndex(
    const SparseSetItr *itrPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

#ifndef _DEBUG
/*
 * Returns the sparse index of the previous element
 * iterated over by the given iterator of the
 * specified type; returns the invalid index if no 
 * elements have been iterated over
 */
#define sparseSetItrIndex(TYPENAME, ITRPTR) \
    _sparseSetItrIndex(ITRPTR)
#else
/*
 * Returns the sparse index of the previous element
 * iterated over by the given iterator of the
 * specified type; returns the invalid index if no 
 * elements have been iterated over
 */
#define sparseSetItrIndex(TYPENAME, ITRPTR) \
    _sparseSetItrIndex(ITRPTR, #TYPENAME)
#endif

/* Frees the given sparse set */
void _sparseSetFree(
    SparseSet *setPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

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