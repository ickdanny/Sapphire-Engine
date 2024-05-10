#include "Constructure_SparseSet.h"

/* max size_t will signal invalid index */
#define invalidSparseIndex (~((size_t)0u))

/* Creates a sparse set and returns it by value */
SparseSet _sparseSetMake(
    size_t capacity,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    assertTrue(
        capacity > 0u, 
        "capacity cannot be 0; " SRC_LOCATION
    );
    SparseSet toRet = {0};

    /* init sparse to invalid */
    toRet._sparsePtr = pgAlloc(
        capacity, 
        sizeof(size_t)
    );
    memset(
        toRet._sparsePtr, 
        0xFF, 
        capacity * sizeof(size_t)
    );

    /* allocate dense */
    toRet._densePtr = pgAlloc(
        capacity,
        elementSize
    );

    /* allocate reflect */
    toRet._reflectPtr = pgAlloc(
        capacity,
        sizeof(size_t)
    );

    toRet.capacity = capacity;
    toRet._size = 0u;

    #ifdef _DEBUG
    toRet._typeName = typeName;
    #endif

    return toRet;
}

/*
 * Makes a one level deep copy of the given
 * sparse set and returns it by value
 */
SparseSet _sparseSetCopy(
    size_t elementSize,
    const SparseSet *toCopyPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, toCopyPtr);
    #endif

    SparseSet toRet = {0};
    toRet.capacity = toCopyPtr->capacity;
    toRet._size = toCopyPtr->_size;
    toRet._sparsePtr = pgAlloc(
        toRet.capacity,
        sizeof(size_t)
    );
    toRet._densePtr = pgAlloc(
        toRet.capacity,
        elementSize
    );
    toRet._reflectPtr = pgAlloc(
        toRet.capacity,
        sizeof(size_t)
    );

    /* may overflow */
    memcpy(
        toRet._sparsePtr,
        toCopyPtr->_sparsePtr,
        toRet.capacity * sizeof(size_t)
    );
    memcpy(
        toRet._densePtr,
        toCopyPtr->_densePtr,
        toRet.capacity * elementSize
    );
    memcpy(
        toRet._reflectPtr,
        toCopyPtr->_reflectPtr,
        toRet.capacity * sizeof(size_t)
    );

    #ifdef _DEBUG
    /* safe to shallow copy; it is a literal */
    toRet._typeName = toCopyPtr->_typeName;
    #endif

    return toRet;
}

/* 
 * Clears the given sparse set, removing all key
 * associations and zeroing its data
 */
void _sparseSetClear(
    SparseSet *setPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    /* invalidate sparse keys */
    memset(
        setPtr->_sparsePtr, 
        0xFF, 
        setPtr->capacity * sizeof(size_t)
    );

    /* clear dense */
    memset(
        setPtr->_densePtr,
        0,
        setPtr->capacity * elementSize
    );

    /* clear reflect */
    memset(
        setPtr->_reflectPtr,
        0,
        setPtr->capacity * sizeof(size_t)
    );

    setPtr->_size = 0u;
}

/* 
 * Returns true if the given sparse set contains an
 * element associated with the specified index,
 * false otherwise
 */
bool _sparseSetContains(
    const SparseSet *setPtr,
    size_t sparseIndex
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    return sparseIndex < setPtr->capacity
        && setPtr->_sparsePtr[sparseIndex] 
            != invalidSparseIndex;
}

/*
 * Returns a pointer to the element associated with
 * the given index in the given sparse set, or NULL
 * if no such element exists
 */
void *_sparseSetGetPtr(
    SparseSet *setPtr,
    size_t sparseIndex,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    /* error if bad index*/
    assertTrue(
        sparseIndex < setPtr->capacity,
        "bad index; " SRC_LOCATION
    );

    size_t denseIndex 
        = setPtr->_sparsePtr[sparseIndex];
    return denseIndex == invalidSparseIndex
        ? NULL
        : voidPtrAdd(
            setPtr->_densePtr,
            elementSize * denseIndex
        );
}

/* 
 * Copies the specified value into the element 
 * associated with the given index in the given
 * sparse set
 */
void _sparseSetSetPtr(
    SparseSet *setPtr,
    size_t sparseIndex,
    void *valuePtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    /* error if bad index*/
    assertTrue(
        sparseIndex < setPtr->capacity,
        "bad index; " SRC_LOCATION
    );

    /* copy the value into dense */
    /* memcpy safe; elements shouldn't overlap */
    memcpy(
        voidPtrAdd(
            setPtr->_densePtr,
            elementSize * setPtr->_size
        ),
        valuePtr, 
        elementSize
    );
    /* set reflect */
    setPtr->_reflectPtr[setPtr->_size] = sparseIndex;
    
    /* set sparse */
    setPtr->_sparsePtr[sparseIndex] = setPtr->_size;
    ++(setPtr->_size);
}

/*
 * Removes the element associated with the given index
 * from the given sparse set; returns true if
 * successful, false otherwise
 */
bool _sparseSetRemove(
    SparseSet *setPtr,
    size_t sparseIndex,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    /* error if bad sparse index*/
    assertTrue(
        sparseIndex < setPtr->capacity,
        "bad index; " SRC_LOCATION
    );

    size_t denseIndex 
        = setPtr->_sparsePtr[sparseIndex];
    if(denseIndex >= setPtr->_size){
        return false;
    }

    /* invalidate sparse for the element to remove */
    setPtr->_sparsePtr[sparseIndex] 
        = invalidSparseIndex;
    
    --(setPtr->_size);
    /* if not removing end, swap in last element */
    if(setPtr->_size && denseIndex != setPtr->_size){
        size_t sparseIndexOfEnd 
            = setPtr->_reflectPtr[setPtr->_size];
        /* overwrite value */
        memcpy(
            voidPtrAdd(
                setPtr->_densePtr,
                denseIndex * elementSize
            ),
            voidPtrAdd(
                setPtr->_densePtr,
                setPtr->_size * elementSize
            ),
            elementSize
        );
        /* overwrite reflect */
        setPtr->_reflectPtr[denseIndex]
            = sparseIndexOfEnd;
        /* change sparse entry for previous end */
        setPtr->_sparsePtr[sparseIndexOfEnd]
            = denseIndex;
    }
    /* 
     * if remove end, no need to do anything since 
     * size already shrank
     */
    return true;
}

/* Creates an iterator over the given sparse set */
SparseSetItr _sparseSetItr(
    SparseSet *setPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    SparseSetItr toRet = {
        setPtr->_densePtr,
        setPtr->_reflectPtr,
        setPtr->_size,
        0
        #ifdef _DEBUG
        ,typeName
        #endif
    };

    return toRet;
}

/*
 * Returns true if the given iterator has a next
 * element, false otherwise
 */
bool _sparseSetItrHasNext(
    const SparseSetItr *itrPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetItrPtrTypeCheck(typeName, itrPtr);
    #endif

    return itrPtr->_currentIndex < itrPtr->_size;
}

/*
 * Returns a pointer to the next element from the
 * given iterator, or NULL if no such element exists
 */
void *_sparseSetItrNextPtr(
    SparseSetItr *itrPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetItrPtrTypeCheck(typeName, itrPtr);
    #endif

    /* save a pointer to the current index if valid */
    void *toRet = NULL;
    if(itrPtr->_currentIndex < itrPtr->_size){
        toRet = voidPtrAdd(
            itrPtr->_densePtr,
            itrPtr->_currentIndex * elementSize
        );
    }
    /* advance current index */
    ++(itrPtr->_currentIndex);

    return toRet;
}

/*
 * Returns the sparse index of the previous element
 * iterated over by the given iterator; returns
 * the invalid index if no elements have been iterated
 * over
 */
size_t _sparseSetItrIndex(
    const SparseSetItr *itrPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetItrPtrTypeCheck(typeName, itrPtr);
    #endif

    /* 
     * if current index is 0, we have not iterated
     * over anything
     */
    if(!itrPtr->_currentIndex){
        return invalidSparseIndex;
    }
    /* return the previous element of reflect */
    return itrPtr->_reflectPtr[
        itrPtr->_currentIndex - 1
    ];
}

/* Frees the given sparse set */
void _sparseSetFree(
    SparseSet *setPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _sparseSetPtrTypeCheck(typeName, setPtr);
    #endif

    pgFree(setPtr->_sparsePtr);
    pgFree(setPtr->_densePtr);
    pgFree(setPtr->_reflectPtr);

    setPtr->capacity = 0u;
    setPtr->_size = 0u;
}