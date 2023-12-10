#ifndef CONSTRUCTURE_ARRAY

#include <stdlib.h>
#include <string.h>

#include "PGUtil.h"

#define CONSTRUCTURE_ARRAY(TYPENAME, PREFIX, ELEMENT)           \
                                                                \
typedef struct TYPENAME{                                        \
    ELEMENT *ptr;                                               \
    int size;                                                   \
} TYPENAME;                                                     \
                                                                \
extern inline TYPENAME PREFIX##Make(int size){                  \
    assertTrue(size > 0, "bad init size");                      \
    TYPENAME toRet = {0};                                       \
    toRet.ptr = pgAlloc(size, sizeof(ELEMENT));                 \
    toRet.size = size;                                          \
    return toRet;                                               \
}                                                               \
                                                                \
extern inline void PREFIX##Clear(                               \
    TYPENAME *arrayPtr,                                         \
    ELEMENT clearValue                                          \
){                                                              \
    for(int i = 0; i < arrayPtr->size; ++i){                    \
        *(arrayPtr->ptr + i) = clearValue;                      \
    }                                                           \
}                                                               \
                                                                \
extern inline ELEMENT PREFIX##Get(                              \
    TYPENAME *arrayPtr,                                         \
    int index                                                   \
){                                                              \
    assertTrue(                                                 \
        index > 0 && index < arrayPtr->size,                    \
        "bad index"                                             \
    );                                                          \
    return (arrayPtr->ptr)[index];                              \
}                                                               \
                                                                \
extern inline void PREFIX##Set(                                 \
    TYPENAME *arrayPtr,                                         \
    const ELEMENT *elementPtr,                                  \
    int index                                                   \
){                                                              \
    assertTrue(                                                 \
        index > 0 && index < arrayPtr->size,                    \
        "bad index"                                             \
    );                                                          \
    (arrayPtr->ptr)[index] = *elementPtr;                       \
}                                                               \
                                                                \
extern inline void PREFIX##SetValue(                            \
    TYPENAME *arrayPtr,                                         \
    ELEMENT element,                                            \
    int index                                                   \
){                                                              \
    assertTrue(                                                 \
        index > 0 && index < arrayPtr->size,                    \
        "bad index"                                             \
    );                                                          \
    (arrayPtr->ptr)[index] = element;                           \
}                                                               \
                                                                \
extern inline void PREFIX##ForEach(                             \
    TYPENAME *arrayPtr,                                         \
    void (*funcPtr)(ELEMENT*)                                   \
){                                                              \
    for(int i = 0; i < arrayPtr->size; ++i){                    \
        (*funcPtr)(arrayPtr->ptr + i);                          \
    }                                                           \
}                                                               \
                                                                \
extern inline void PREFIX##ForEachValue(                        \
    TYPENAME *arrayPtr,                                         \
    void (*funcPtr)(ELEMENT)                                    \
){                                                              \
    for(int i = 0; i < arrayPtr->size; ++i){                    \
        (*funcPtr)((arrayPtr->ptr)[i]);                         \
    }                                                           \
}                                                               \
                                                                \
extern inline void PREFIX##Free(TYPENAME *arrayPtr){            \
    pgFree(arrayPtr->ptr);                                      \
    arrayPtr->size = 0;                                         \
}

#endif