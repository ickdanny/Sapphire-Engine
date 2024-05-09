#include "PGUtil.h"

/* 
 * Does pointer arithmetic on void pointers by
 * casting to char
 */
void *voidPtrAdd(void *voidPtr, int offset){
    return ((char *)voidPtr) + offset;
}