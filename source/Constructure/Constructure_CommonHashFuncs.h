#ifndef CONSTRUCTURE_COMMONHASHFUNCS_H
#define CONSTRUCTURE_COMMONHASHFUNCS_H

#include <stddef.h>
#include <stdbool.h>

/*
 * The following functions are for use with the
 * Constructure HashMap type
 */

/* hash func for ints */
size_t intHash(const void *intPtr);

/* equals func for ints */
bool intEquals(
    const void *intPtr1, 
    const void *intPtr2
);

/* hash func for C strings */
size_t cStringHash(const void *stringPtr);

/* equals func for C strings */
bool cStringEquals(
    const void *stringPtr1,
    const void *stringPtr2
);

#endif