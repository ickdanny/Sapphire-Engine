#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define CONSTRUCTURE_ARRAYLIST(TYPENAME, PREFIX, ELEMENT)                       \
                                                                                \
typedef struct TYPENAME{                                                        \
    ELEMENT *ptr;                                                               \
    int size;                                                                   \
    int capacity;                                                               \
} TYPENAME;                                                                     \
                                                                                \
extern inline TYPENAME PREFIX##Make(int initCapacity){                          \
    assert(initCapacity >= 0);                                                  \
    TYPENAME toRet = {0};                                                       \
    toRet.capacity = initCapacity;                                              \
    toRet.ptr = calloc(initCapacity, sizeof(ELEMENT));                          \
    return toRet;                                                               \
}                                                                               \
                                                                                \
extern inline bool _##PREFIX##GrowIfNeeded(TYPENAME *listPtr){                  \
    enum{ growRatio = 2 };                                                      \
    if(listPtr->size == listPtr->capacity){                                     \
        listPtr->capacity *= growRatio;                                         \
        ++listPtr->capacity;                                                    \
        listPtr->ptr = realloc(                                                 \
            listPtr->ptr,                                                       \
            sizeof(ELEMENT) * listPtr->capacity                                 \
        );                                                                      \
        if(!(listPtr->ptr)){                                                    \
            return false;                                                       \
        }                                                                       \
    }                                                                           \
    return true;                                                                \
}                                                                               \
                                                                                \
extern inline bool PREFIX##PushBack(                                            \
    TYPENAME *listPtr,                                                          \
    const ELEMENT *newElementPtr                                                \
){                                                                              \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                                      \
        return false;                                                           \
    }                                                                           \
    listPtr->ptr[listPtr->size] = *newElementPtr;                               \
    ++(listPtr->size);                                                          \
    return true;                                                                \
}                                                                               \
                                                                                \
extern inline bool PREFIX##PushBackValue(TYPENAME *listPtr, ELEMENT element){   \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                                      \
        return false;                                                           \
    }                                                                           \
    (listPtr->ptr)[listPtr->size] = element;                                    \
    ++(listPtr->size);                                                          \
    return true;                                                                \
}                                                                               \
                                                                                \
extern inline void PREFIX##PopBack(TYPENAME *listPtr){                          \
    --(listPtr->size);                                                          \
}                                                                               \
                                                                                \
extern inline void PREFIX##Clear(TYPENAME *listPtr){                            \
    listPtr->size = 0;                                                          \
}                                                                               \
                                                                                \
extern inline ELEMENT PREFIX##Get(TYPENAME *listPtr, int index){                \
    assert(index >= 0 && index < listPtr->size);                                \
    return (listPtr->ptr)[index];                                               \
}                                                                               \
                                                                                \
extern inline ELEMENT PREFIX##Front(TYPENAME *listPtr){                         \
    assert(listPtr->size > 0);                                                  \
    return *(listPtr->ptr);                                                     \
}                                                                               \
                                                                                \
extern inline ELEMENT PREFIX##Back(TYPENAME *listPtr){                          \
    assert(listPtr->size > 0);                                                  \
    return (listPtr->ptr)[listPtr->size - 1];                                   \
}                                                                               \
                                                                                \
extern inline bool PREFIX##IsEmpty(TYPENAME *listPtr){                          \
    return listPtr->size == 0;                                                  \
}                                                                               \
                                                                                \
extern inline void PREFIX##Erase(TYPENAME *listPtr, int index){                 \
    assert(index >= 0 && index < listPtr->size);                                \
    --(listPtr->size);                                                          \
    memmove(                                                                    \
        listPtr->ptr + index,                                                   \
        listPtr->ptr + (index + 1),                                             \
        (listPtr->size - index) * sizeof(ELEMENT)                               \
    );                                                                          \
}                                                                               \
                                                                                \
extern inline bool PREFIX##Insert(                                              \
    TYPENAME *listPtr,                                                          \
    const ELEMENT *newElementPtr,                                               \
    int index                                                                   \
){                                                                              \
    assert(index >= 0 && index <= listPtr->size);                               \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                                      \
        return false;                                                           \
    }                                                                           \
    memmove(                                                                    \
        listPtr->ptr + (index + 1),                                             \
        listPtr->ptr + index,                                                   \
        (listPtr->size - index) * sizeof(ELEMENT)                               \
    );                                                                          \
    listPtr->ptr[index] = *newElementPtr;                                       \
    ++(listPtr->size);                                                          \
    return true;                                                                \
}                                                                               \
                                                                                \
extern inline bool PREFIX##InsertValue(                                         \
    TYPENAME *listPtr,                                                          \
    ELEMENT element,                                                            \
    int index                                                                   \
){                                                                              \
    assert(index >= 0 && index <= listPtr->size);                               \
    if(!_##PREFIX##GrowIfNeeded(listPtr)){                                      \
        return false;                                                           \
    }                                                                           \
    memmove(                                                                    \
        listPtr->ptr + (index + 1),                                             \
        listPtr->ptr + index,                                                   \
        (listPtr->size - index) * sizeof(ELEMENT)                               \
    );                                                                          \
    listPtr->ptr[index] = element;                                              \
    ++(listPtr->size);                                                          \
    return true;                                                                \
}                                                                               \
                                                                                \
extern inline void PREFIX##Assign(                                              \
    TYPENAME *listPtr,                                                          \
    const ELEMENT *elementPtr,                                                  \
    int index                                                                   \
){                                                                              \
    assert(index >= 0 && index < listPtr->size);                                \
    (listPtr->ptr)[index] = *elementPtr;                                        \
}                                                                               \
                                                                                \
extern inline void PREFIX##AssignValue(                                         \
    TYPENAME *listPtr,                                                          \
    ELEMENT element,                                                            \
    int index                                                                   \
){                                                                              \
    assert(index >= 0 && index < listPtr->size);                                \
    (listPtr->ptr)[index] = element;                                            \
}                                                                               \
                                                                                \
extern inline void PREFIX##Free(TYPENAME *listPtr){                             \
    free(listPtr->ptr);                                                         \
    listPtr->ptr = NULL;                                                        \
    listPtr->size = 0;                                                          \
    listPtr->capacity = 0;                                                      \
}                                                                               \
                                                                                \
extern inline void PREFIX##ForEach(                                             \
    TYPENAME *listPtr,                                                          \
    void (*funcPtr)(ELEMENT*)                                                   \
){                                                                              \
    for(int i = 0; i < listPtr->size; ++i){                                     \
        (*funcPtr)(listPtr->ptr + i);                                           \
    }                                                                           \
}                                                                               \
                                                                                \
extern inline void PREFIX##ForEachValue(                                        \
    TYPENAME *listPtr,                                                          \
    void (*funcPtr)(ELEMENT)                                                    \
){                                                                              \
    for(int i = 0; i < listPtr->size; ++i){                                     \
        (*funcPtr)((listPtr->ptr)[i]);                                          \
    }                                                                           \
}