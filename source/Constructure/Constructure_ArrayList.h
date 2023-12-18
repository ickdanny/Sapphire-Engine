#ifndef CONSTRUCTURE_ARRAYLIST_H
#define CONSTRUCTURE_ARRAYLIST_H

#include "PGUtil.h"

/* A growable contiguous array on the heap */
typedef struct ArrayList{
    void *_ptr;
    size_t size;
    size_t _capacity;

    #ifdef _DEBUG
    const char *_typeName;
    #endif
} ArrayList;

#ifdef _DEBUG
/*
 * Asserts that the given type matches that of the 
 * given arraylist pointer
 */
#define _arrayListPtrTypeCheck( \
    TYPENAME, \
    ARRAYLISTPTR \
) \
    assertStringEqual( \
        TYPENAME, \
        (ARRAYLISTPTR)->_typeName, \
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
    toRet._capacity = initCapacity;
    toRet._ptr = pgAlloc(initCapacity, elementSize);
    toRet.size = 0;

    #ifdef _DEBUG
    toRet._typeName = typeName;
    #endif

    return toRet;
}

#ifndef _DEBUG
/* 
 * Creates an arraylist of the specified type 
 * and capacity and returns it by value
 */
#define arrayListMake(TYPENAME, INIT_CAPACITY) \
    _arrayListMake(INIT_CAPACITY, sizeof(TYPENAME))
#else
/* 
 * Creates an arraylist of the specified type 
 * and capacity and returns it by value
 */
#define arrayListMake(TYPENAME, INIT_CAPACITY) \
    _arrayListMake( \
        INIT_CAPACITY, \
        sizeof(TYPENAME), \
        #TYPENAME \
    )
#endif

/* 
 * Returns true if the given arraylist is empty,
 * false otherwise
 */
extern inline bool arrayListIsEmpty(
    const ArrayList *arrayListPtr
){
    return arrayListPtr->size == 0u;
}

/* Removes all elements of the given arraylist */
extern inline void _arrayListClear(
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

#ifndef _DEBUG
/* 
 * Removes all elements of the given arraylist
 * of the specified type
 */
#define arrayListClear(TYPENAME, ARRAYLISTPTR) \
    _arrayListClear(ARRAYLISTPTR, sizeof(TYPENAME))
#else
/* 
 * Removes all elements of the given arraylist
 * of the specified type
 */
#define arrayListClear(TYPENAME, ARRAYLISTPTR) \
    _arrayListClear( \
        ARRAYLISTPTR, \
        sizeof(TYPENAME), \
        #TYPENAME \
    )
#endif

/* 
 * Grows the given arraylist if it is at capacity,
 * returns false as error code, true otherwise
 */
extern inline bool _arrayListGrowIfNeeded(
    ArrayList *arrayListPtr,
    size_t elementSize
) {
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
extern inline void _arrayListPushBackPtr(
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

#ifndef _DEBUG
/* 
 * Pushes a copy of the specified element onto
 * the back of the given arraylist of the 
 * specified type
 */
#define arrayListPushBackPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    ELEMENTPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arrayListPushBackPtr( \
            ARRAYLISTPTR, \
            ELEMENTPTR, \
            sizeof(TYPENAME) \
        ) \
    )
#else
/* 
 * Pushes a copy of the specified element onto
 * the back of the given arraylist of the 
 * specified type
 */
#define arrayListPushBackPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    ELEMENTPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arrayListPushBackPtr( \
            ARRAYLISTPTR, \
            ELEMENTPTR, \
            sizeof(TYPENAME), \
            #TYPENAME \
        ) \
    )
#endif

/*
 * Arraylist pushing must be done via macro
 * because it expects values not pointers
 */
#ifndef _DEBUG
/* 
 * Pushes the given value onto the back of 
 * the given arraylist of the specified type
 */
#define arrayListPushBack( \
    TYPENAME, \
    ARRAYLISTPTR, \
    ELEMENT \
) \
    do{ \
        assertTrue( \
            _arrayListGrowIfNeeded( \
                ARRAYLISTPTR, \
                sizeof(ELEMENT) \
            ), \
            "failed to grow for pushback; " \
            SRC_LOCATION \
        ); \
        ((TYPENAME *)((ARRAYLISTPTR)->_ptr))[ \
            (ARRAYLISTPTR)->size \
        ] = ELEMENT; \
        ++((ARRAYLISTPTR)->size); \
    } while(false)
#else
/* 
 * Pushes the given value onto the back of 
 * the given arraylist of the specified type
 */
#define arrayListPushBack( \
    TYPENAME, \
    ARRAYLISTPTR, \
    ELEMENT \
) \
    do{ \
        _arrayListPtrTypeCheck( \
            #TYPENAME, \
            ARRAYLISTPTR \
        ); \
        assertTrue( \
            _arrayListGrowIfNeeded( \
                ARRAYLISTPTR, \
                sizeof(ELEMENT) \
            ), \
            "failed to grow for pushback; " \
            SRC_LOCATION \
        ); \
        ((TYPENAME *)((ARRAYLISTPTR)->_ptr))[ \
            (ARRAYLISTPTR)->size \
        ] = ELEMENT; \
        ++((ARRAYLISTPTR)->size); \
    } while(false)
#endif

/*
 * Removes the back of the given arraylist; 
 * error if empty
 */
extern inline void _arrayListPopBack(
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

#ifndef _DEBUG
/*
 * Removes the back of the given arraylist
 * of the specified type; error if empty
 */
#define arrayListPopBack(TYPENAME, ARRAYLISTPTR) \
    _arrayListPopBack(ARRAYLISTPTR)
#else
/*
 * Removes the back of the given arraylist
 * of the specified type; error if empty
 */
#define arrayListPopBack(TYPENAME, ARRAYLISTPTR) \
    _arrayListPopBack(ARRAYLISTPTR, #TYPENAME)
#endif

/* 
 * Erases the element of the given arraylist
 * at the given index
 */
extern inline void _arrayListErase(
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

#ifndef _DEBUG
/* 
 * Erases the element of the given arraylist
 * of the specified type at the given index
 */
#define arrayListErase( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX \
) \
    _arrayListErase( \
        ARRAYLISTPTR, \
        INDEX, \
        sizeof(TYPENAME) \
    )
#else
/* 
 * Erases the element of the given arraylist
 * of the specified type at the given index
 */
#define arrayListErase( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX \
) \
    _arrayListErase( \
        ARRAYLISTPTR, \
        INDEX, \
        sizeof(TYPENAME), \
        #TYPENAME \
    )
#endif

/*
 * Returns a pointer to the element of the given
 * arraylist at the given index
 */
extern inline void *_arrayListGetPtr(
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
    return voidPtrAdd(
        arrayListPtr->_ptr,
        index * elementSize
    );
}

#ifndef _DEBUG
/*
 * Returns a pointer to the element of the given
 * arraylist of the specified type at the
 * given index
 */
#define arrayListGetPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX \
) \
    ((TYPENAME*)_arrayListGetPtr( \
        ARRAYLISTPTR, \
        INDEX, \
        sizeof(TYPENAME) \
    ))
#else
/*
 * Returns a pointer to the element of the given
 * arraylist of the specified type at the
 * given index
 */
#define arrayListGetPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX \
) \
    ((TYPENAME*)_arrayListGetPtr( \
        ARRAYLISTPTR, \
        INDEX, \
        sizeof(TYPENAME), \
        #TYPENAME \
    ))
#endif

/*
 * Returns the value of the element of the given
 * arraylist of the specified type at the
 * given index
 */
#define arrayListGet( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX \
) \
    ( \
        (TYPENAME) \
        (*arrayListGetPtr( \
            TYPENAME, \
            ARRAYLISTPTR, \
            INDEX \
        )) \
    )

/* 
 * Copies the specified element into the given 
 * arraylist at the given index, replacing the
 * previous element
 */
extern inline void _arrayListSetPtr(
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
        index < arrayListPtr->size, 
        "bad index; " SRC_LOCATION
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
}

#ifndef _DEBUG
/* 
 * Copies the specified element into the given 
 * arraylist of the specified type at the
 * given index, replacing the previous element
 */
#define arrayListSetPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENTPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arrayListSetPtr( \
            ARRAYLISTPTR, \
            INDEX, \
            ELEMENTPTR, \
            sizeof(TYPENAME) \
        ) \
    )
#else
/* 
 * Copies the specified element into the given 
 * arraylist of the specified type at the
 * given index, replacing the previous element
 */
#define arrayListSetPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENTPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arrayListSetPtr( \
            ARRAYLISTPTR, \
            INDEX, \
            ELEMENTPTR, \
            sizeof(TYPENAME), \
            #TYPENAME \
        ) \
    )
#endif

/* 
 * Arraylist setting must be done via macro because
 * it expects values not pointers.
 */
#ifndef _DEBUG
/* 
 * Sets the element at the given index in the given
 * arraylist of the specified type to the 
 * given value, replacing the previous element
 */
#define arrayListSet( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENT \
) \
    do{ \
        assertTrue( \
            INDEX < (ARRAYLISTPTR)->size, \
            "bad index; " SRC_LOCATION \
        ); \
        ((TYPENAME *)((ARRAYLISTPTR)->_ptr))[INDEX] \
            = ELEMENT; \
    } while(false)
#else
/* 
 * Sets the element at the given index in the given
 * arraylist of the specified type to the 
 * given value, replacing the previous element
 */
#define arrayListSet( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENT \
) \
    do{ \
        _arrayListPtrTypeCheck( \
            #TYPENAME, \
            ARRAYLISTPTR \
        ); \
        assertTrue( \
            INDEX < (ARRAYLISTPTR)->size, \
            "bad index; " SRC_LOCATION \
        ); \
        ((TYPENAME *)((ARRAYLISTPTR)->_ptr))[INDEX] \
            = ELEMENT; \
    } while(false)
#endif

/* 
 * Copies the specified element into the given
 * arraylist at the given index by making room
 * for the element
 */
extern inline void _arrayListInsertPtr(
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

#ifndef _DEBUG
/* 
 * Copies the specified element into the given
 * arraylist of the specified type at the 
 * given index by making room for the element
 */
#define arrayListInsertPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENTPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arrayListInsertPtr( \
            ARRAYLISTPTR, \
            INDEX, \
            ELEMENTPTR, \
            sizeof(TYPENAME) \
        ) \
    )
#else
/* 
 * Copies the specified element into the given
 * arraylist of the specified type at the 
 * given index by making room for the element
 */
#define arrayListInsertPtr( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENTPTR \
) \
    _Generic(*ELEMENTPTR, \
        TYPENAME: _arrayListInsertPtr( \
            ARRAYLISTPTR, \
            INDEX, \
            ELEMENTPTR, \
            sizeof(TYPENAME), \
            #TYPENAME \
        ) \
    )
#endif

/*
 * Arraylist insert must be done via macro because
 * it expects values not pointers.
 */
#ifndef _DEBUG
/*
 * Inserts the given element into in the given 
 * arraylist of the specified type at the given
 * index by making room for the element
 */
#define arrayListInsert( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENT \
) \
    do{ \
        assertTrue( \
            INDEX <= (ARRAYLISTPTR)->size, \
            "bad index; " SRC_LOCATION \
        ); \
        assertTrue( \
            _arrayListGrowIfNeeded( \
                (ARRAYLISTPTR), \
                sizeof(ELEMENT) \
            ), \
            "failed to grow for insert; " \
            SRC_LOCATION \
        ); \
        memmove( \
            voidPtrAdd( \
                (ARRAYLISTPTR)->_ptr, \
                (INDEX + 1) * sizeof(ELEMENT) \
            ), \
            voidPtrAdd( \
                (ARRAYLISTPTR)->_ptr, \
                INDEX * sizeof(ELEMENT) \
            ), \
            ((ARRAYLISTPTR)->size - INDEX) \
                * sizeof(ELEMENT) \
        ); \
        ((TYPENAME *)((ARRAYLISTPTR)->_ptr))[INDEX] \
            = ELEMENT; \
        ++((ARRAYLISTPTR)->size); \
    } while(false)
#else
/*
 * Inserts the given element into in the given 
 * arraylist of the specified type at the given
 * index by making room for the element
 */
#define arrayListInsert( \
    TYPENAME, \
    ARRAYLISTPTR, \
    INDEX, \
    ELEMENT \
) \
    do{ \
        _arrayListPtrTypeCheck( \
            #TYPENAME, \
            (ARRAYLISTPTR) \
        ); \
        assertTrue( \
            INDEX <= (ARRAYLISTPTR)->size, \
            "bad index; " SRC_LOCATION \
        ); \
        assertTrue( \
            _arrayListGrowIfNeeded( \
                (ARRAYLISTPTR), \
                sizeof(ELEMENT) \
            ), \
            "failed to grow for insert; " \
            SRC_LOCATION \
        ); \
        memmove( \
            voidPtrAdd( \
                (ARRAYLISTPTR)->_ptr, \
                (INDEX + 1) * sizeof(ELEMENT) \
            ), \
            voidPtrAdd( \
                (ARRAYLISTPTR)->_ptr, \
                INDEX * sizeof(ELEMENT) \
            ), \
            ((ARRAYLISTPTR)->size - INDEX) \
                * sizeof(ELEMENT) \
        ); \
        ((TYPENAME *)((ARRAYLISTPTR)->_ptr))[INDEX] \
            = ELEMENT; \
        ++((ARRAYLISTPTR)->size); \
    } while(false)
#endif

/*
 * Returns a pointer to the front element of 
 * the given arraylist
 */
extern inline void *_arrayListFrontPtr(
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

#ifndef _DEBUG
/*
 * Returns a pointer to the front element of 
 * the given arraylist of the specified type
 */
#define arrayListFrontPtr( \
    TYPENAME, \
    ARRAYLISTPTR \
) \
    ((TYPENAME*)_arrayListFrontPtr( \
        ARRAYLISTPTR \
    ))
#else
/*
 * Returns a pointer to the front element of 
 * the given arraylist of the specified type
 */
#define arrayListFrontPtr( \
    TYPENAME, \
    ARRAYLISTPTR \
) \
    ((TYPENAME*)_arrayListFrontPtr( \
        ARRAYLISTPTR, \
        #TYPENAME \
    ))
#endif

/*
 * Returns the value of the front element of
 * the given arraylist of the specified type
 */
#define arrayListFront( \
    TYPENAME, \
    ARRAYLISTPTR \
) \
    ( \
        (TYPENAME) \
        (*arrayListFrontPtr( \
            TYPENAME, \
            ARRAYLISTPTR \
        )) \
    )

/*
 * Returns a pointer to the back element of 
 * the given arraylist
 */
extern inline void *_arrayListBackPtr(
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

#ifndef _DEBUG
/*
 * Returns a pointer to the back element of 
 * the given arraylist of the specified type
 */
#define arrayListBackPtr( \
    TYPENAME, \
    ARRAYLISTPTR \
) \
    ((TYPENAME*)_arrayListBackPtr( \
        ARRAYLISTPTR, \
        sizeof(TYPENAME) \
    ))
#else
/*
 * Returns a pointer to the back element of 
 * the given arraylist of the specified type
 */
#define arrayListBackPtr( \
    TYPENAME, \
    ARRAYLISTPTR \
) \
    ((TYPENAME*)_arrayListBackPtr( \
        ARRAYLISTPTR, \
        sizeof(TYPENAME), \
        #TYPENAME \
    ))
#endif

/*
 * Returns the value of the back element of
 * the given arraylist of the specified type
 */
#define arrayListBack( \
    TYPENAME, \
    ARRAYLISTPTR \
) \
    ( \
        (TYPENAME) \
        (*arrayListBackPtr( \
            TYPENAME, \
            ARRAYLISTPTR \
        )) \
    )



/* 
 * Apply must be done via macro because
 * it expects values not pointers.
 */
#ifndef _DEBUG
/* 
 * Applies the given function to each element
 * of the given arraylist sequentially from 
 * index 0; the function takes a pointer of
 * the arraylist type.
 */
#define arrayListApply( \
    TYPENAME, \
    ARRAYLISTPTR, \
    FUNC \
) \
    do{ \
        for( \
            size_t u = 0u; \
            u < (ARRAYLISTPTR)->size; \
            ++u \
        ){ \
            FUNC( \
                ( \
                    (TYPENAME*) \
                    ((ARRAYLISTPTR)->_ptr) \
                ) + u \
            ); \
        } \
    } while(false)
#else
/* 
 * Applies the given function to each element
 * of the given arraylist sequentially from 
 * index 0; the function takes a pointer of
 * the arraylist type.
 */
#define arrayListApply( \
    TYPENAME, \
    ARRAYLISTPTR, \
    FUNC \
) \
    do{ \
        _arrayListPtrTypeCheck( \
            #TYPENAME, \
            ARRAYLISTPTR \
        ); \
        for( \
            size_t u = 0u; \
            u < (ARRAYLISTPTR)->size; \
            ++u \
        ){ \
            FUNC( \
                ( \
                    (TYPENAME*) \
                    ((ARRAYLISTPTR)->_ptr) \
                ) + u \
            ); \
        } \
    } while(false)
#endif

/* Frees the given arraylist */
extern inline void _arrayListFree(
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
    arrayListPtr->size = 0;
    arrayListPtr->_capacity = 0;
}

#ifndef _DEBUG
/* 
 * Frees the given arraylist of the 
 * specified type 
 */
#define arrayListFree(TYPENAME, ARRAYLISTPTR) \
    _arrayListFree(ARRAYLISTPTR)
#else
/* 
 * Frees the given arraylist of the 
 * specified type 
 */
#define arrayListFree(TYPENAME, ARRAYLISTPTR) \
    _arrayListFree(ARRAYLISTPTR, #TYPENAME)
#endif

#endif