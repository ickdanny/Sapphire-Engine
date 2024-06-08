#include "PGUtil_Error.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prints the given error message but does not exit */
void pgWarning(const char *warningMsg){
    fprintf(stderr, "%s\n", warningMsg);
}

/* Prints the given error message and exits */
void pgError(const char *errorMsg){
    fprintf(stderr, "%s\n", errorMsg);
    exit(1);
}

/* 
 * Asserts that the given bool is true, printing the
 * given error message otherwise
 */
void assertTrue(bool assertion, const char *errorMsg){
    if(!assertion){
        pgError(errorMsg);
    }
}

/*
 * Asserts that the given bool is false, printing the 
 * given error message otherwise 
 */
void assertFalse(
    bool assertion, 
    const char *errorMsg
){
    if(assertion){
        pgError(errorMsg);
    }
}

/*
 * Asserts that the two given strings are equal under
 * strcmp, printing the given error message otherwise
 */
void assertStringEqual(
    const char *str1, 
    const char *str2,
    const char *errorMsg
){
    assertFalse(strcmp(str1, str2), errorMsg);
}