#ifndef CONSTRUCTURE_HASHMAP_H
#define CONSTRUCTURE_HASHMAP_H

#include "PGUtil.h"

/* A growable hash map on the heap */
typedef struct HashMap{
    void *_ptr;
    size_t _capacity;

    #ifdef _DEBUG
    /* 
     * Should only ever point to string literals,
     * thus should not be freed
     */
    const char *_keyTypeName;
    const char *_valueTypeName;
    #endif
} HashMap;

#ifdef _DEBUG
/*
 * Asserts that the given type matches that of the 
 * given hashmap pointer
 */
#define _hashMapPtrTypeCheck( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR \
) \
    assertStringEqual( \
        KEYTYPENAME, \
        (HASHMAPPTR)->_keyTypeName, \
        "bad hashmap key type; " SRC_LOCATION \
    ) \
    assertStringEqual( \
        VALUETYPENAME, \
        (HASHMAPPTR)->_valueTypeName, \
        "bad hashmap value type; " SRC_LOCATION \
    )
#endif

#endif