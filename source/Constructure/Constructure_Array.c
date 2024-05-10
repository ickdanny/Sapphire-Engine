#include "Constructure_Array.h"

/*
 * Creates an array on the heap and returns
 * it by value
 */
Array _arrayMake(
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
    Array toRet = {0};
    toRet._ptr = pgAlloc(size, elementSize);
    toRet.size = size;

    #ifdef _DEBUG
    toRet._typeName = typeName;
    #endif

    return toRet;
}

/*
 * Makes a one level deep copy of the given array
 * and returns it by value
 */
Array _arrayCopy(
    size_t elementSize,
    const Array *toCopyPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayPtrTypeCheck(typeName, toCopyPtr);
    #endif

    Array toRet = {0};
    toRet.size = toCopyPtr->size;
    toRet._ptr = pgAlloc(toRet.size, elementSize);
    /* may overflow */
    memcpy(
        toRet._ptr, 
        toCopyPtr->_ptr, 
        toRet.size * elementSize
    );

    #ifdef _DEBUG
    /* safe to shallow copy; it is a literal */
    toRet._typeName = toCopyPtr->_typeName;
    #endif

    return toRet;
}

/* Zeroes the data in the given array */
void _arrayClear(
    Array *arrayPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayPtrTypeCheck(typeName, arrayPtr);
    #endif

    memset(
        arrayPtr->_ptr, 
        0, 
        arrayPtr->size * elementSize
    );
}

/* 
 * Returns a pointer to the element of the given array
 * at the given index
 */
void *_arrayGetPtr(
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
    return voidPtrAdd(
        arrayPtr->_ptr, 
        index * elementSize
    );
}

/* 
 * Copies the specified element into the given array 
 * at the given index
 */
void _arraySetPtr(
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

    /* memcpy safe; elements shouldn't overlap */
    memcpy(
        voidPtrAdd(
            arrayPtr->_ptr, 
            index * elementSize
        ),
        elementPtr,
        elementSize
    );
}

/* Frees the given array */
void _arrayFree(
    Array *arrayPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayPtrTypeCheck(typeName, arrayPtr);
    #endif
    
    pgFree(arrayPtr->_ptr);
    arrayPtr->size = 0u;
}