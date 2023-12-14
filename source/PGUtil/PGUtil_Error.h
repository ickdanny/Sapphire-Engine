#ifndef PGUTIL_ERROR_H
#define PGUTIL_ERROR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * Asserts that the given value is zero, printing
 * the given error message otherwise
 */
#define assertZero assertFalse

/*
 * Asserts that the given value is not zero, printing
 * the given error message otherwise
 */
#define assertNotZero assertTrue

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

/*
 * Asserts that the two given strings are equal under
 * strcmp, printing the given error message otherwise
 */
extern inline void assertStringEqual(
    const char *str1, 
    const char *str2,
    const char *errorMsg
){
    assertFalse(strcmp(str1, str2), errorMsg);
}

#endif