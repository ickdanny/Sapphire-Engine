#ifndef PGUTIL_ALLOC_H
#define PGUTIL_ALLOC_H

#include <stdlib.h>
#include "PGUtil_Error.h"

extern inline void *pgAlloc(size_t numItems, size_t size){
    assertTrue(numItems > 0, "numItems must be > 0");
    assertTrue(size > 0, "size must be > 0");
    void *toRet = calloc(numItems, size);
    assertTrue(toRet, "alloc fail");
    return toRet;
}

extern inline void *pgRealloc(
    void *ptr, 
    size_t numItems, 
    size_t size
){
    assertTrue(numItems > 0, "numItems must be > 0");
    assertTrue(size > 0, "size must be > 0");
    void *toRet = realloc(ptr, numItems * size);
    assertTrue(toRet, "realloc fail");
    return toRet;
}

#define pgFree(PTR_NAME) \
    free(PTR_NAME);      \
    PTR_NAME = NULL;

#endif