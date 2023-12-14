#ifndef CONSTRUCTURE_ARRAY_H
#define CONSTRUCTURE_ARRAY_H

#include <stdlib.h>
#include <string.h>

#include "PGUtil.h"

/* A wrapper for a contiguous array on the heap */
typedef struct _Array{
    void *ptr;
    size_t size;

    #ifdef DEBUG
    const char *typeName;
    #endif
} _Array;

#ifdef DEBUG
/*
 * Asserts that the given type matches that of the 
 * given array
 */
#define _arrayTypeCheck(TYPENAME, ARRAYPTR) \
    assertStringEqual( \
        TYPENAME, \
        ARRAYPTR->typeName, \
        "bad array type" \
    );
#endif

/* Creates an array on the heap */
extern inline _Array _arrayMake(
    size_t size,
    size_t elementSize
    #ifdef DEBUG 
    , const char *typeName 
    #endif
){
    assertTrue(size > 0, "size cannot be 0!");
    _Array toRet = {0};
    toRet.ptr = pgAlloc(size, elementSize);
    toRet.size = size;

    #ifdef DEBUG
    toRet.typeName = typeName;
    #endif

    return toRet;
}

#ifndef DEBUG
/* 
 * Creates an array on the heap of the specified type 
 * and size 
 */
#define arrayMake(TYPENAME, SIZE) \
    _arrayMake(SIZE, sizeof(TYPENAME))
#else
/* 
 * Creates an array on the heap of the specified type 
 * and size 
 */
#define arrayMake(TYPENAME, SIZE) \
    _arrayMake(SIZE, sizeof(TYPENAME), #TYPENAME)
#endif

/* Zeroes the data in the given array */
extern inline void _arrayClear(
    _Array *arrayPtr,
    size_t elementSize
    #ifdef DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef DEBUG
    _arrayTypeCheck(typename, arrayPtr);
    #endif

    memset(arrayPtr->ptr, 0, arrayPtr->size * elementSize);
}

#ifndef DEBUG
/* 
 * Zeroes the data in the given array of the 
 * specified type
 */
#define arrayClear(TYPENAME, ARRAYPTR) \
    _arrayClear(ARRAYPTR, sizeof(TYPENAME))
#else
/* 
 * Zeroes the data in the given array of the 
 * specified type
 */
#define arrayClear(TYPENAME, ARRAYPTR) \
    _arrayClear(ARRAYPTR, sizeof(TYPENAME), #TYPENAME)
#endif

/* 
 * Returns a pointer to the element of the given array
 * at the given index
 */
extern inline void *_arrayGetPtr(
    _Array *arrayPtr,
    size_t index,
    size_t elementSize
    #ifdef DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef DEBUG
    _arrayTypeCheck(typename, arrayPtr);
    #endif

    assertTrue(index < arrayPtr->size, "bad index");
    return voidPtrAdd(arrayPtr->ptr, index * elementSize);
}

#ifndef DEBUG
/*
 * Returns a pointer to the element of the given array
 * of the specified type at the given index
 */
#define arrayGetPtr(TYPENAME, ARRAYPTR, INDEX) \
    ((TYPENAME*)_arrayGetPtr( \
        ARRAYPTR, \
        INDEX, \
        sizeof(TYPENAME)\
    ))
#else
/*
 * Returns a pointer to the element of the given array
 * of the specified type at the given index
 */
#define arrayGetPtr(TYPENAME, ARRAYPTR, INDEX) \
    ((TYPENAME*)_arrayGetPtr( \
        ARRAYPTR, \
        INDEX, \
        sizeof(TYPENAME) \
        #TYPENAME \
    ))
#endif

/*
 * Returns the value of the element of the given array
 * of the specified type at the given index
 */
#define arrayGet(TYPENAME, ARRAYPTR, INDEX) \
    (*arrayGetPtr(TYPENAME, ARRAYPTR, INDEX))

/* 
 * Copies the specified element into the given array 
 * at the given index
 */
extern inline void _arraySetPtr(
    _Array *arrayPtr,
    size_t index,
    const void *elementPtr,
    size_t elementSize
    #ifdef DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef DEBUG
    _arrayTypeCheck(typename, arrayPtr);
    #endif

    assertTrue(index < arrayPtr->size, "bad index");

    /* memcpy over memmove as elements shouldn't overlap */
    memcpy(
        voidPtrAdd(arrayPtr->ptr, index * elementSize),
        elementPtr,
        elementSize
    );
}

#ifndef DEBUG
/* 
 * Copies the specified element into the given array 
 * of the specified type at the given index
 */
#define arraySetPtr(TYPENAME, ARRAYPTR, INDEX, ELEMENTPTR) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arraySetPtr( \
            ARRAYPTR, \
            INDEX, \
            ELEMENTPTR, \
            sizeof(TYPENAME) \
        ), \
    )
#else
/* 
 * Copies the specified element into the given array 
 * of the specified type at the given index
 */
#define arraySetPtr(TYPENAME, ARRAYPTR, INDEX, ELEMENTPTR) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arraySetPtr( \
            ARRAYPTR, \
            INDEX, \
            ELEMENTPTR, \
            sizeof(TYPENAME), \
            #TYPENAME \
        ), \
    )
#endif

#ifndef DEBUG
/* 
 * Sets the element at the given index in the given
 * array of the specified type to the given value
 */
#define arraySet(TYPENAME, ARRAY, INDEX, ELEMENT) \
    do{ \
        assertTrue(INDEX < (ARRAY).size, "bad index"); \
        ((TYPENAME *)((ARRAY).ptr))[INDEX] = ELEMENT; \
    } while(false)
#else
/* 
 * Sets the element at the given index in the given
 * array of the specified type to the given value
 */
#define arraySet(TYPENAME, ARRAY, INDEX, ELEMENT) \
    do{ \
        _arrayTypeCheck(TYPENAME, &(ARRAY)); \
        assertTrue(INDEX < (ARRAY).size, "bad index"); \
        ((TYPENAME *)((ARRAY).ptr))[INDEX] = ELEMENT; \
    } while(false)
#endif

extern inline void PREFIX##ForEach(
    TYPENAME *arrayPtr,
    void (*funcPtr)(ELEMENT*)
){
    for(int i = 0; i < arrayPtr->size; ++i){
        (*funcPtr)(arrayPtr->ptr + i);
    }
}

extern inline void PREFIX##ForEachValue(
    TYPENAME *arrayPtr,
    void (*funcPtr)(ELEMENT)
){
    for(int i = 0; i < arrayPtr->size; ++i){
        (*funcPtr)((arrayPtr->ptr)[i]);
    }
}

extern inline void PREFIX##Free(TYPENAME *arrayPtr){
    pgFree(arrayPtr->ptr);
    arrayPtr->size = 0;
}

/* Frees the given array */
extern inline void _arrayFree(
    _Array *arrayPtr
    #ifdef DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef DEBUG
    _arrayTypeCheck(typename, arrayPtr);
    #endif
    
    pgFree(arrayPtr->ptr);
    arrayPtr->size = 0;
}



#define CONSTRUCTURE_ARRAY(TYPENAME, PREFIX, ELEMENT)           \
                                                                \
typedef struct TYPENAME{                                        \
    ELEMENT *ptr;                                               \
    int size;                                                   \
} TYPENAME;                                                     \
                                                                \
extern inline TYPENAME PREFIX##Make(int size){                  \
    assertTrue(size > 0, "bad init size");                      \
    TYPENAME toRet = {0};                                       \
    toRet.ptr = pgAlloc(size, sizeof(ELEMENT));                 \
    toRet.size = size;                                          \
    return toRet;                                               \
}                                                               \
                                                                \
extern inline void PREFIX##Clear(                               \
    TYPENAME *arrayPtr,                                         \
    ELEMENT clearValue                                          \
){                                                              \
    for(int i = 0; i < arrayPtr->size; ++i){                    \
        *(arrayPtr->ptr + i) = clearValue;                      \
    }                                                           \
}                                                               \
                                                                \
extern inline ELEMENT PREFIX##Get(                              \
    TYPENAME *arrayPtr,                                         \
    int index                                                   \
){                                                              \
    assertTrue(                                                 \
        index > 0 && index < arrayPtr->size,                    \
        "bad index"                                             \
    );                                                          \
    return (arrayPtr->ptr)[index];                              \
}                                                               \
                                                                \
extern inline void PREFIX##Set(                                 \
    TYPENAME *arrayPtr,                                         \
    const ELEMENT *elementPtr,                                  \
    int index                                                   \
){                                                              \
    assertTrue(                                                 \
        index > 0 && index < arrayPtr->size,                    \
        "bad index"                                             \
    );                                                          \
    (arrayPtr->ptr)[index] = *elementPtr;                       \
}                                                               \
                                                                \
extern inline void PREFIX##SetValue(                            \
    TYPENAME *arrayPtr,                                         \
    ELEMENT element,                                            \
    int index                                                   \
){                                                              \
    assertTrue(                                                 \
        index > 0 && index < arrayPtr->size,                    \
        "bad index"                                             \
    );                                                          \
    (arrayPtr->ptr)[index] = element;                           \
}                                                               \
                                                                \
extern inline void PREFIX##ForEach(                             \
    TYPENAME *arrayPtr,                                         \
    void (*funcPtr)(ELEMENT*)                                   \
){                                                              \
    for(int i = 0; i < arrayPtr->size; ++i){                    \
        (*funcPtr)(arrayPtr->ptr + i);                          \
    }                                                           \
}                                                               \
                                                                \
extern inline void PREFIX##ForEachValue(                        \
    TYPENAME *arrayPtr,                                         \
    void (*funcPtr)(ELEMENT)                                    \
){                                                              \
    for(int i = 0; i < arrayPtr->size; ++i){                    \
        (*funcPtr)((arrayPtr->ptr)[i]);                         \
    }                                                           \
}                                                               \
                                                                \
extern inline void PREFIX##Free(TYPENAME *arrayPtr){            \
    pgFree(arrayPtr->ptr);                                      \
    arrayPtr->size = 0;                                         \
}

#endif