#ifndef PGUTIL_H
#define PGUTIL_H

#include "PGUtil_Alloc.h"
#include "PGUtil_Error.h"

/* Various other utility functions */

/* 
 * Does pointer arithmetic on void pointers by
 * casting to char
 */
extern inline void *voidPtrAdd(
    void *voidPtr, 
    int offset
){
    return ((char *)voidPtr) + offset;
}

#endif