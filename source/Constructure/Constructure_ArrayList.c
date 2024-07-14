#include "Constructure_ArrayList.h"

/* Creates an arraylist and returns it by value */
ArrayList _arrayListMake(
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
    toRet._capacity = initCapacity;
    toRet._ptr = pgAlloc(initCapacity, elementSize);
    toRet.size = 0u;

    #ifdef _DEBUG
    toRet._typeName = typeName;
    #endif

    return toRet;
}

/*
 * Makes a one level deep copy of the given
 * arraylist and returns it by value
 */
ArrayList _arrayListCopy(
    size_t elementSize,
    const ArrayList *toCopyPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(typeName, toCopyPtr);
    #endif

    ArrayList toRet = {0};
    toRet.size = toCopyPtr->size;
    toRet._capacity = toCopyPtr->_capacity;
    toRet._ptr = pgAlloc(
        toRet._capacity, 
        elementSize
    );
    /* may overflow */
    memcpy(
        toRet._ptr, 
        toCopyPtr->_ptr, 
        toRet._capacity * elementSize
    );

    #ifdef _DEBUG
    /* safe to shallow copy; it is a literal */
    toRet._typeName = toCopyPtr->_typeName;
    #endif

    return toRet;
}

/* 
 * Returns true if the given arraylist is empty,
 * false otherwise
 */
bool arrayListIsEmpty(
    const ArrayList *arrayListPtr
){
    return arrayListPtr->size == 0u;
}

/* Removes all elements of the given arraylist */
void _arrayListClear(
    ArrayList *arrayListPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    memset(
        arrayListPtr->_ptr, 
        0, 
        arrayListPtr->_capacity * elementSize
    );
    arrayListPtr->size = 0u;
}

/* 
 * Grows the given arraylist if it is at capacity;
 * returns false as error code, true otherwise
 */
bool _arrayListGrowIfNeeded(
    ArrayList *arrayListPtr,
    size_t elementSize
){
    enum{ growRatio = 2u };

    if(arrayListPtr->size 
        == arrayListPtr->_capacity
    ){
        arrayListPtr->_capacity *= growRatio;
        ++(arrayListPtr->_capacity);
        arrayListPtr->_ptr = pgRealloc(
            arrayListPtr->_ptr,
            arrayListPtr->_capacity,
            elementSize
        );
        if(!(arrayListPtr->_ptr)){
            return false;
        }
    }
    return true;  
}

/* 
 * Pushes a copy of the specified element onto
 * the back of the given arraylist
 */
void _arrayListPushBackPtr(
    ArrayList *arrayListPtr,
    const void *elementPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    assertTrue(
        _arrayListGrowIfNeeded(
            arrayListPtr, 
            elementSize
        ),
        "failed to grow for pushback; "
        SRC_LOCATION
    );

    /* memcpy safe; elements shouldn't overlap */
    memcpy(
        voidPtrAdd(
            arrayListPtr->_ptr, 
            (arrayListPtr->size) * elementSize
        ),
        elementPtr,
        elementSize
    );
    ++(arrayListPtr->size);
}

/*
 * Removes the back of the given arraylist; 
 * error if empty
 */
void _arrayListPopBack(
    ArrayList *arrayListPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    assertFalse(
        arrayListIsEmpty(arrayListPtr),
        "empty in popback; " SRC_LOCATION
    );

    --(arrayListPtr->size);
}

/* 
 * Erases the element of the given arraylist
 * at the given index
 */
void _arrayListErase(
    ArrayList *arrayListPtr,
    size_t index,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    assertTrue(
        index < arrayListPtr->size,
        "bad index; " SRC_LOCATION
    );

    --(arrayListPtr->size);

    /* move all later elements 1 forward */
    /* length is 0 if we remove back */
    memmove(
        voidPtrAdd(
            arrayListPtr->_ptr, 
            index * elementSize
        ),
        voidPtrAdd(
            arrayListPtr->_ptr,
            (index + 1) * elementSize
        ),
        (arrayListPtr->size - index) * elementSize
    );  
}

/*
 * Returns a pointer to the element of the given
 * arraylist at the given index
 */
void *_arrayListGetPtr(
    ArrayList *arrayListPtr,
    size_t index,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    //todo: temp
    if(index >= arrayListPtr->size){
        printf("uh oh");
    }

    assertTrue(
        index < arrayListPtr->size,
        "bad index; " SRC_LOCATION
    );
    return voidPtrAdd(
        arrayListPtr->_ptr,
        index * elementSize
    );
}

/* 
 * Copies the specified element into the given 
 * arraylist at the given index, replacing the
 * previous element
 */
void _arrayListSetPtr(
    ArrayList *arrayListPtr,
    size_t index,
    const void *elementPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    assertTrue(
        index <= arrayListPtr->size, 
        "bad index; " SRC_LOCATION
    );

    if(index == arrayListPtr->size){
        assertTrue(
            _arrayListGrowIfNeeded(
                arrayListPtr, 
                elementSize
            ),
            "failed to grow for set; "
            SRC_LOCATION
        );
        ++(arrayListPtr->size);
    }

    /* memcpy safe; elements shouldn't overlap */
    memcpy(
        voidPtrAdd(
            arrayListPtr->_ptr, 
            index * elementSize
        ),
        elementPtr,
        elementSize
    );
}

/* 
 * Copies the specified element into the given
 * arraylist at the given index by making room
 * for the element
 */
void _arrayListInsertPtr(
    ArrayList *arrayListPtr,
    size_t index,
    const void *elementPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    assertTrue(
        index <= arrayListPtr->size,
        "bad index; " SRC_LOCATION
    );

    assertTrue(
        _arrayListGrowIfNeeded(
            arrayListPtr, 
            elementSize
        ),
        "failed to grow for insert; "
        SRC_LOCATION
    );

    /* move all later elements 1 back */
    /* length is 0 if we insert as new back */
    memmove(
        voidPtrAdd(
            arrayListPtr->_ptr,
            (index + 1) * elementSize
        ),
        voidPtrAdd(
            arrayListPtr->_ptr,
            index * elementSize
        ),
        (arrayListPtr->size - index) * elementSize
    );

    /* memcpy safe; elements shouldn't overlap */
    memcpy(
        voidPtrAdd(
            arrayListPtr->_ptr, 
            index * elementSize
        ),
        elementPtr,
        elementSize
    );

    ++(arrayListPtr->size);
}

/*
 * Returns a pointer to the front element of 
 * the given arraylist
 */
void *_arrayListFrontPtr(
    ArrayList *arrayListPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    assertFalse(
        arrayListIsEmpty(arrayListPtr),
        "empty in front; " SRC_LOCATION
    );
    return arrayListPtr->_ptr;
}

/*
 * Returns a pointer to the back element of 
 * the given arraylist
 */
void *_arrayListBackPtr(
    ArrayList *arrayListPtr,
    size_t elementSize
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    assertFalse(
        arrayListIsEmpty(arrayListPtr),
        "empty in back; " SRC_LOCATION
    );

    return voidPtrAdd(
        arrayListPtr->_ptr,
        (arrayListPtr->size - 1u) 
            * elementSize
    );
}

/* Frees the given arraylist */
void _arrayListFree(
    ArrayList *arrayListPtr
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    #ifdef _DEBUG
    _arrayListPtrTypeCheck(
        typeName, 
        arrayListPtr
    );
    #endif

    pgFree(arrayListPtr->_ptr);
    arrayListPtr->size = 0u;
    arrayListPtr->_capacity = 0u;
}