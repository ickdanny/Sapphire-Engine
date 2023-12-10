#ifndef PGUTIL_ERROR_H
#define PGUTIL_ERROR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* 
 * Asserts that the given bool is true, printing the given
 * error message otherwise
 */
extern inline void assertTrue(bool assertion, const char *errorMsg){
    if(!assertion){
        fprintf(stderr, "%s\n", errorMsg);
        exit(1);
    }
}

/*
 * Asserts that the given bool is false, printing the given 
 * error message otherwise 
 */
extern inline void assertFalse(bool assertion, const char *errorMsg){
    if(assertion){
        fprintf(stderr, "%s\n", errorMsg);
        exit(1);
    }
}

/* 
 * Asserts that the given pointer is not null, printing
 * the given error message otherwise
 */
#define assertNotNull assertTrue

/*
 * Asserts that the given pointer is null, printing
 * the given error message otherwise
 */
#define assertNull assertFalse

#endif