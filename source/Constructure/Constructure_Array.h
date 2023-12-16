#ifndef CONSTRUCTURE_ARRAY_H
#define CONSTRUCTURE_ARRAY_H

#include "PGUtil.h"

/* A wrapper for a contiguous array on the heap */
typedef struct Array{
    void *ptr;
    size_t size;

    #ifdef _DEBUG
    const char *typeName;
    #endif
} Array;

#ifdef _DEBUG
/*
 * Asserts that the given type matches that of the 
 * given array pointer
 */
#define _arrayPtrTypeCheck(TYPENAME, ARRAYPTR) \
    assertStringEqual( \
        TYPENAME, \
        (ARRAYPTR)->typeName, \
        "bad array type; " SRC_LOCATION \
    )
/*
 * Asserts that the given type matches that of the 
 * given array
 */
#define _arrayTypeCheck(TYPENAME, ARRAY) \
    assertStringEqual( \
        TYPENAME, \
        (ARRAY).typeName, \
        "bad array type; " SRC_LOCATION \
    )
#endif

/*
 * Creates an array on the heap and returns
 * it by value
 */
extern inline Array _arrayMake(
    size_t size,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    assertTrue(
        size > 0, 
        "size cannot be 0; " SRC_LOCATION
    );
    Array toRet = {0};
    toRet.ptr = pgAlloc(size, elementSize);
    toRet.size = size;

    #ifdef _DEBUG
    toRet.typeName = typeName;
    #endif

    return toRet;
}

#ifndef _DEBUG
/* 
 * Creates an array on the heap of the specified type 
 * and size and returns it by value
 */
#define arrayMake(TYPENAME, SIZE) \
    _arrayMake(SIZE, sizeof(TYPENAME))
#else
/* 
 * Creates an array on the heap of the specified type 
 * and size and returns it by value
 */
#define arrayMake(TYPENAME, SIZE) \
    _arrayMake(SIZE, sizeof(TYPENAME), #TYPENAME)
#endif

/* Zeroes the data in the given array */
extern inline void _arrayClear(
    Array *arrayPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayPtrTypeCheck(typeName, arrayPtr);
    #endif

    memset(arrayPtr->ptr, 0, arrayPtr->size * elementSize);
}

#ifndef _DEBUG
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
    Array *arrayPtr,
    size_t index,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayPtrTypeCheck(typeName, arrayPtr);
    #endif

    assertTrue(
        index < arrayPtr->size, 
        "bad index; " SRC_LOCATION
    );
    return voidPtrAdd(arrayPtr->ptr, index * elementSize);
}

#ifndef _DEBUG
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
        sizeof(TYPENAME), \
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
    Array *arrayPtr,
    size_t index,
    const void *elementPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayPtrTypeCheck(typeName, arrayPtr);
    #endif

    assertTrue(
        index < arrayPtr->size, 
        "bad index; " SRC_LOCATION
    );

    /* memcpy over memmove as elements shouldn't overlap */
    memcpy(
        voidPtrAdd(arrayPtr->ptr, index * elementSize),
        elementPtr,
        elementSize
    );
}

#ifndef _DEBUG
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
        ) \
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
        ) \
    )
#endif

/* 
 * Array setting must be done via macro because
 * it expects values not pointers.
 */
#ifndef _DEBUG
/* 
 * Sets the element at the given index in the given
 * array of the specified type to the given value
 */
#define arraySet(TYPENAME, ARRAY, INDEX, ELEMENT) \
    do{ \
        assertTrue( \
            INDEX < (ARRAY).size, \
            "bad index; " SRC_LOCATION \
        ); \
        ((TYPENAME *)((ARRAY).ptr))[INDEX] = ELEMENT; \
    } while(false)
#else
/* 
 * Sets the element at the given index in the given
 * array of the specified type to the given value
 */
#define arraySet(TYPENAME, ARRAY, INDEX, ELEMENT) \
    do{ \
        _arrayTypeCheck(#TYPENAME, ARRAY); \
        assertTrue( \
            INDEX < (ARRAY).size, \
            "bad index; " SRC_LOCATION \
        ); \
        ((TYPENAME *)((ARRAY).ptr))[INDEX] = ELEMENT; \
    } while(false)
#endif

/* 
 * Apply must be done via macro because
 * it expects values not pointers.
 */
#ifndef _DEBUG
/* 
 * Applies the given function to each element
 * of the given array sequentially from index 0;
 * the function takes a pointer of the array type.
 */
#define arrayApply(TYPENAME, ARRAY, FUNC) \
    do{ \
        for(int i = 0; i < (ARRAY).size; ++i){ \
            FUNC(((TYPENAME*)(ARRAY.ptr)) + i); \
        } \
    } while(false)
#else
/* 
 * Applies the given function to each element
 * of the given array sequentially from index 0;
 * the function takes a pointer of the array type.
 */
#define arrayApply(TYPENAME, ARRAY, FUNC) \
    do{ \
        _arrayTypeCheck(#TYPENAME, ARRAY); \
        for(int i = 0; i < (ARRAY).size; ++i){ \
            FUNC(((TYPENAME*)(ARRAY.ptr)) + i); \
        } \
    } while(false)
#endif

/* Frees the given array */
extern inline void _arrayFree(
    Array *arrayPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayPtrTypeCheck(typeName, arrayPtr);
    #endif
    
    pgFree(arrayPtr->ptr);
    arrayPtr->size = 0;
}

#ifndef _DEBUG
/* Frees the given array of the specified type */
#define arrayFree(TYPENAME, ARRAYPTR) \
    _arrayFree(ARRAYPTR)
#else
/* Frees the given array of the specified type */
#define arrayFree(TYPENAME, ARRAYPTR) \
    _arrayFree(ARRAYPTR, #TYPENAME)
#endif

#endif