#ifndef CONSTRUCTURE_ARRAYLIST

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "PGUtil.h"

#define CONSTRUCTURE_ARRAYLIST(TYPENAME, PREFIX, ELEMENT)   \
                                                            \
typedef struct TYPENAME{                                    \
    ELEMENT *ptr;                                           \
    int size;                                               \
    int capacity;                                           \
} TYPENAME;                                                 \
                                                            \
extern inline TYPENAME PREFIX##Make(int initCapacity){      \
    assertTrue(initCapacity >= 0, "bad init capacity");     \
    TYPENAME toRet = {0};                                   \
    toRet.capacity = initCapacity;                          \
    toRet.ptr = pgAlloc(initCapacity, sizeof(ELEMENT));     \
    return toRet;                                           \
}                                                           \
                                                            \
extern inline bool _##PREFIX##GrowIfNeeded(                 \
    TYPENAME *listPtr                                       \
){                                                          \
    enum{ growRatio = 2 };                                  \
    if(listPtr->size == listPtr->capacity){                 \
        listPtr->capacity *= growRatio;                     \
        ++listPtr->capacity;                                \
        listPtr->ptr = pgRealloc(                           \
            listPtr->ptr,                                   \
            listPtr->capacity,                              \
            sizeof(ELEMENT)                                 \
        );                                                  \
        if(!(listPtr->ptr)){                                \
            return false;                                   \
        }                                                   \
    }                                                       \
    return true;                                            \
}                                                           \
                                                            \
extern inline bool PREFIX##PushBack(                        \
    TYPENAME *listPtr,                                      \
    const ELEMENT *newElementPtr                            \
){                                                          \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                  \
        return false;                                       \
    }                                                       \
    listPtr->ptr[listPtr->size] = *newElementPtr;           \
    ++(listPtr->size);                                      \
    return true;                                            \
}                                                           \
                                                            \
extern inline bool PREFIX##PushBackValue(                   \
    TYPENAME *listPtr,                                      \
    ELEMENT element                                         \
){                                                          \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                  \
        return false;                                       \
    }                                                       \
    (listPtr->ptr)[listPtr->size] = element;                \
    ++(listPtr->size);                                      \
    return true;                                            \
}                                                           \
                                                            \
extern inline void PREFIX##PopBack(TYPENAME *listPtr){      \
    --(listPtr->size);                                      \
}                                                           \
                                                            \
extern inline void PREFIX##Clear(TYPENAME *listPtr){        \
    listPtr->size = 0;                                      \
}                                                           \
                                                            \
extern inline ELEMENT PREFIX##Get(                          \
    TYPENAME *listPtr,                                      \
    int index                                               \
){                                                          \
    assertTrue(                                             \
        index >= 0 && index < listPtr->size,                \
        "bad index"                                         \
    );                                                      \
    return (listPtr->ptr)[index];                           \
}                                                           \
                                                            \
extern inline ELEMENT PREFIX##Front(TYPENAME *listPtr){     \
    assertTrue(listPtr->size > 0, "list is empty");         \
    return *(listPtr->ptr);                                 \
}                                                           \
                                                            \
extern inline ELEMENT PREFIX##Back(TYPENAME *listPtr){      \
    assertTrue(listPtr->size > 0, "list is empty");         \
    return (listPtr->ptr)[listPtr->size - 1];               \
}                                                           \
                                                            \
extern inline bool PREFIX##IsEmpty(TYPENAME *listPtr){      \
    return listPtr->size == 0;                              \
}                                                           \
                                                            \
extern inline void PREFIX##Erase(                           \
    TYPENAME *listPtr,                                      \
    int index                                               \
){                                                          \
    assertTrue(                                             \
        index >= 0 && index < listPtr->size,                \
        "bad index"                                         \
    );                                                      \
    --(listPtr->size);                                      \
    memmove(                                                \
        listPtr->ptr + index,                               \
        listPtr->ptr + (index + 1),                         \
        (listPtr->size - index) * sizeof(ELEMENT)           \
    );                                                      \
}                                                           \
                                                            \
extern inline bool PREFIX##Insert(                          \
    TYPENAME *listPtr,                                      \
    const ELEMENT *newElementPtr,                           \
    int index                                               \
){                                                          \
    assertTrue(                                             \
        index >= 0 && index <= listPtr->size,               \
        "bad index"                                         \
    );                                                      \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                  \
        return false;                                       \
    }                                                       \
    memmove(                                                \
        listPtr->ptr + (index + 1),                         \
        listPtr->ptr + index,                               \
        (listPtr->size - index) * sizeof(ELEMENT)           \
    );                                                      \
    listPtr->ptr[index] = *newElementPtr;                   \
    ++(listPtr->size);                                      \
    return true;                                            \
}                                                           \
                                                            \
extern inline bool PREFIX##InsertValue(                     \
    TYPENAME *listPtr,                                      \
    ELEMENT element,                                        \
    int index                                               \
){                                                          \
    assertTrue(                                             \
        index >= 0 && index <= listPtr->size,               \
        "bad index"                                         \
    );                                                      \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                  \
        return false;                                       \
    }                                                       \
    memmove(                                                \
        listPtr->ptr + (index + 1),                         \
        listPtr->ptr + index,                               \
        (listPtr->size - index) * sizeof(ELEMENT)           \
    );                                                      \
    listPtr->ptr[index] = element;                          \
    ++(listPtr->size);                                      \
    return true;                                            \
}                                                           \
                                                            \
extern inline void PREFIX##Assign(                          \
    TYPENAME *listPtr,                                      \
    const ELEMENT *elementPtr,                              \
    int index                                               \
){                                                          \
    assertTrue(                                             \
        index >= 0 && index <= listPtr->size,               \
        "bad index"                                         \
    );                                                      \
    (listPtr->ptr)[index] = *elementPtr;                    \
}                                                           \
                                                            \
extern inline void PREFIX##AssignValue(                     \
    TYPENAME *listPtr,                                      \
    ELEMENT element,                                        \
    int index                                               \
){                                                          \
    assertTrue(                                             \
        index >= 0 && index <= listPtr->size,               \
        "bad index"                                         \
    );                                                      \
    (listPtr->ptr)[index] = element;                        \
}                                                           \
                                                            \
extern inline void PREFIX##Free(TYPENAME *listPtr){         \
    pgFree(listPtr->ptr);                                   \
    listPtr->size = 0;                                      \
    listPtr->capacity = 0;                                  \
}                                                           \
                                                            \
extern inline void PREFIX##ForEach(                         \
    TYPENAME *listPtr,                                      \
    void (*funcPtr)(ELEMENT*)                               \
){                                                          \
    for(int i = 0; i < listPtr->size; ++i){                 \
        (*funcPtr)(listPtr->ptr + i);                       \
    }                                                       \
}                                                           \
                                                            \
extern inline void PREFIX##ForEachValue(                    \
    TYPENAME *listPtr,                                      \
    void (*funcPtr)(ELEMENT)                                \
){                                                          \
    for(int i = 0; i < listPtr->size; ++i){                 \
        (*funcPtr)((listPtr->ptr)[i]);                      \
    }                                                       \
}

#endif