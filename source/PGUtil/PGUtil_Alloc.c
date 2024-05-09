#include "PGUtil_Alloc.h"

#include <stdlib.h>

#include "PGUtil_Error.h"

/* 
 * Returns a pointer to a newly allocated block which 
 * holds the specified number of items, each of the 
 * given size
 */
void *pgAlloc(size_t numItems, size_t size){
    assertTrue(numItems > 0, "numItems must be > 0");
    assertTrue(size > 0, "size must be > 0");
    void *toRet = calloc(numItems, size);
    assertTrue(toRet, "alloc fail");
    return toRet;
}

/*
 * Returns a pointer to a reallocated block which 
 * holds the specified number of items, each of the 
 * given size, having the same data as was previously 
 * held in the specified pointer
 */
void *pgRealloc(
    void *ptr, 
    size_t numItems, 
    size_t size
){
    assertTrue(numItems > 0, "numItems must be > 0");
    assertTrue(size > 0, "size must be > 0");
    /* possible multiplication overflow */
    void *toRet = realloc(ptr, numItems * size);
    assertTrue(toRet, "realloc fail");
    return toRet;
}