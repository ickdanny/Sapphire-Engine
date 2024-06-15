#include "Constructure_CommonHashFuncs.h"

#include <string.h>

/* hash func for ints */
size_t intHash(const void *intPtr){
    return *((int*)intPtr);
}

/* equals func for ints */
bool intEquals(
    const void *intPtr1, 
    const void *intPtr2
){
    return *((int*)intPtr1) == *((int*)intPtr2);
}

/* hash func for C strings */
size_t cStringHash(const void *stringPtr){
    static size_t prime = 53;
    static size_t modulo = 1294967281;

    const char *charPtr = stringPtr;
    size_t hash = 0;
    while(*charPtr != '\0'){
        hash += *charPtr;
        ++hash;
        hash *= prime;
        hash %= modulo;
        ++charPtr;
    }
    return hash;
}

/* equals func for C strings */
bool cStringEquals(
    const void *stringPtr1,
    const void *stringPtr2
){
    char *string1 = *((char**)stringPtr1);
    char *string2 = *((char**)stringPtr2);
    return strcmp(string1, string2) == 0;
}