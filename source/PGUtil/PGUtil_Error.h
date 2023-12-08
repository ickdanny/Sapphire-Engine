#ifndef PGUTIL_ERROR_H
#define PGUTIL_ERROR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern inline void assertTrue(bool assertion, const char *errorMsg){
    if(!assertion){
        fprintf(stderr, "%s\n", errorMsg);
        exit(1);
    }
}

extern inline void assertFalse(bool assertion, const char *errorMsg){
    if(assertion){
        fprintf(stderr, "%s\n", errorMsg);
        exit(1);
    }
}

#endif