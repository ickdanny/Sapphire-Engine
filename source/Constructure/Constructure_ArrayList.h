#ifndef CONSTRUCTURE_ARRAYLIST_H
#define CONSTRUCTURE_ARRAYLIST_H

#include "PGUtil.h"

/* A growable contiguous array on the heap */
typedef struct ArrayList{
    void *ptr;
    size_t size;
    size_t capacity;

    #ifdef _DEBUG
    const char *typeName;
    #endif
} ArrayList;

#ifdef _DEBUG
/*
 * Asserts that the given type matches that of the 
 * given arraylist pointer
 */
#define _arrayListPtrTypeCheck(TYPENAME, ARRAYLISTPTR) \
    assertStringEqual( \
        TYPENAME, \
        (ARRAYLISTPTR)->typeName, \
        "bad arraylist type; " SRC_LOCATION \
    )
/*
 * Asserts that the given type matches that of the 
 * given arraylist
 */
#define _arrayListTypeCheck(TYPENAME, ARRAYLIST) \
    assertStringEqual( \
        TYPENAME, \
        (ARRAYLIST).typeName, \
        "bad arraylist type; " SRC_LOCATION \
    )
#endif

/* Creates an arraylist and returns it by value */
extern inline ArrayList _arrayListMake(
    size_t initCapacity,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    assertTrue(
        initCapacity > 0, 
        "initCapacity cannot be 0; "
        SRC_LOCATION
    );
    ArrayList toRet = {0};
    toRet.capacity = initCapacity;
    toRet.ptr = pgAlloc(initCapacity, elementSize);
    toRet.size = 0;

    #ifdef _DEBUG
    toRet.typeName = typeName;
    #endif

    return toRet;
}

#ifndef _DEBUG
/* 
 * Creates an arraylist of the specified type 
 * and capacity and returns it by value
 */
#define arrayListMake(TYPENAME, INIT_CAPACITY) \
    _arrayMake(INIT_CAPACITY, sizeof(TYPENAME))
#else
/* 
 * Creates an arraylist of the specified type 
 * and capacity and returns it by value
 */
#define arrayListMake(TYPENAME, INIT_CAPACITY) \
    _arrayMake( \
        INIT_CAPACITY, \
        sizeof(TYPENAME), \
        #TYPENAME \
    )
#endif

#endif

/*                                              \
                                                            \
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
*/