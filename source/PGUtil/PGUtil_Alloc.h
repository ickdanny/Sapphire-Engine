#ifndef PGUTIL_ALLOC_H
#define PGUTIL_ALLOC_H

#include <stdlib.h>

#include "PGUtil_Error.h"

/* 
 * Returns a pointer to a newly allocated block which holds
 * the specified number of items, each of the given size.
 */
extern inline void *pgAlloc(size_t numItems, size_t size){
    assertTrue(numItems > 0, "numItems must be > 0");
    assertTrue(size > 0, "size must be > 0");
    void *toRet = calloc(numItems, size);
    assertTrue(toRet, "alloc fail");
    return toRet;
}

/*
 * Returns a pointer to a reallocated block which holds
 * the specified number of items, each of the given size,
 * having the same data as was previously held in the specified
 * pointer.
 */
extern inline void *pgRealloc(
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

/* Frees the given pointer and sets it to NULL */
#define pgFree(PTR_NAME) \
    do{ \
        free(PTR_NAME); \
        (PTR_NAME) = NULL; \
    } while(false)

/* 
 * Frees the given pointer and sets it to the
 * given new value
 */
#define pgFreeAndSwap(PTR_NAME, NEW_VALUE) \
    do{ \
        free(PTR_NAME); \
        (PTR_NAME) = (NEW_VALUE); \
    } while(false)

#endif