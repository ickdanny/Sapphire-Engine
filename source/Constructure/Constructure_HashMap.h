#ifndef CONSTRUCTURE_HASHMAP_H
#define CONSTRUCTURE_HASHMAP_H

#include "PGUtil.h"

/* 
 * The maximum ratio of graves to slots in a
 * hashmap before a rehash occurs
 */
#define _maxGraveRatio .75f
/*
 * The maximum ratio of occupied slots in a
 * hashmap before a resize occurs
 */
#define _maxLoadFactor .65f

/* 
 * A growable hash map on the heap; usage
 * requires functions named hashKEYTYPE,
 * hashKEYTYPEPtr, equalsKEYTYPE, and
 * equalsKEYTYPEPtr to be defined
 */
typedef struct HashMap{
    /*
     * Points to an array on the heap with the
     * following layout: statusByte, key, value
     * in which the statusByte is 0 for empty, 1
     * for grave, and 2 for occupied
     */
    void *_ptr;

    /* The number of currently occupied slots */
    size_t size;

    /* The number of allocated slots */
    size_t _capacity;

    /* 
     * The number of graves including currently
     * occupied slots as well as previously
     * removed slots
     */
    size_t _graveCount;

    #ifdef _DEBUG
    /* 
     * Should only ever point to string literals,
     * thus should not be freed
     */
    const char *_keyTypeName;
    const char *_valueTypeName;
    #endif
} HashMap;

/*
 * Calculates the size in bytes of each slot
 * given the key type and value type
 */
#define _slotSize(KEYTYPENAME, VALUETYPENAME) \
    /* +1 for the status byte */ \
    (sizeof(KEYTYPENAME) \
        + sizeof(VALUETYPENAME) + 1)

/*
 * Retrieves the value of the status byte
 * from the given void pointer to a slot
 */
extern inline unsigned char _getStatusByte(
    void *slotPtr
){
    return *((unsigned char *)slotPtr);
}

/*
 * Returns a void pointer to the key from the given
 * void pointer to a slot
 */
extern inline void *_getKeyPtr(void *slotPtr){
    return voidPtrAdd(slotPtr, 1);
}

/*
 * Returns a void pointer to the value from the
 * given void pointer to a slot
 */
extern inline void *_getValuePtr(
    void *slotPtr,
    size_t keySize
){
    return voidPtrAdd(slotPtr, 1 + keySize);
}

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
    do{ \
        assertStringEqual( \
            KEYTYPENAME, \
            (HASHMAPPTR)->_keyTypeName, \
            "bad hashmap key type; " \
            SRC_LOCATION \
        ); \
        assertStringEqual( \
            VALUETYPENAME, \
            (HASHMAPPTR)->_valueTypeName, \
            "bad hashmap value type; " \
            SRC_LOCATION \
        ); \
    } while(false)
#endif

/* Creates a hashmap and returns it by value */
extern inline HashMap _hashMapMake(
    size_t initCapacity,
    size_t slotSize
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
){
    assertTrue(
        initCapacity > 0, 
        "initCapacity cannot be 0; "
        SRC_LOCATION
    );
    HashMap toRet = {0};
    toRet._capacity = initCapacity;
    toRet._ptr = pgAlloc(initCapacity, slotSize);
    toRet.size = 0u;
    toRet._graveCount = 0u;

    #ifdef _DEBUG
    toRet._keyTypeName = keyTypeName;
    toRet._valueTypeName = valueTypeName;
    #endif

    return toRet;
}

#ifndef _DEBUG
/*
 * Creates a hashmap of the specified key and
 * value types and returns it by value
 */
#define hashMapMake( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    INIT_CAPACITY\
) \
    _hashMapMake( \
        INIT_CAPACITY, \
        _slotSize(KEYTYPENAME, VALUETYPENAME) \
    )

#else
/*
 * Creates a hashmap of the specified key and
 * value types and returns it by value
 */
#define hashMapMake( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    INIT_CAPACITY\
) \
    _hashMapMake( \
        INIT_CAPACITY, \
        _slotSize(KEYTYPE, VALUETYPE), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/*
 * Makes a one level deep copy of the given
 * hashmap and returns it by value
 */
extern inline HashMap _hashMapCopy(
    size_t slotSize,
    const HashMap *toCopyPtr
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
){
    #ifdef _DEBUG
    _hashMapPtrTypeCheck(
        keyTypeName, 
        valueTypeName, 
        toCopyPtr
    );
    #endif

    HashMap toRet = {0};
    toRet.size = toCopyPtr->size;
    toRet._graveCount = toCopyPtr->_graveCount;
    toRet._capacity = toCopyPtr->_capacity;
    toRet._ptr = pgAlloc(
        toRet._capacity,
        slotSize
    );
    /* may overflow */
    memcpy(
        toRet._ptr,
        toCopyPtr->_ptr,
        toRet._capacity * slotSize
    );

    #ifdef _DEBUG
    /* safe to shallow copy; they are literals */
    toRet._keyTypeName = toCopyPtr->_keyTypeName;
    toRet._valueTypeName 
        = toCopyPtr->_valueTypeName;
    #endif

    return toRet;
}

#ifndef _DEBUG
/*
 * Makes a one level deep copy of the given
 * hashmap of the specified key and value types
 * and returns it by value
 */
#define hashMapCopy( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    TOCOPYPTR \
) \
    _hashMapCopy( \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        TOCOPYPTR \
    )
#else
/*
 * Makes a one level deep copy of the given
 * hashmap of the specified key and value types
 * and returns it by value
 */
#define hashMapCopy( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    TOCOPYPTR \
) \
    _hashMapCopy( \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        TOCOPYPTR, \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/*
 * Returns true if the given hashmap is empty,
 * false otherwise
 */
extern inline bool hashMapIsEmpty(
    const HashMap *hashMapPtr
){
    return hashMapPtr->size == 0u;
}

/* Removes all elements of the given hashmap */
extern inline void _hashMapClear(
    HashMap *hashMapPtr,
    size_t slotSize
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
){
    #ifdef _DEBUG
    _hashMapPtrTypeCheck(
        keyTypeName, 
        valueTypeName, 
        hashMapPtr
    );
    #endif

    memset(
        hashMapPtr->_ptr,
        0,
        hashMapPtr->_capacity * slotSize
    );
    hashMapPtr->size = 0u;
    hashMapPtr->_graveCount = 0u;
}

#ifndef _DEBUG
/* 
 * Removes all elements of the given hashmap
 * of the specified key and value types
 */
#define hashMapClear( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR \
) \
    _hashMapClear( \
        HASHMAPPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
    )
#else
/* 
 * Removes all elements of the given hashmap
 * of the specified key and value types
 */
#define hashMapClear( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR \
) \
    _hashMapClear( \
        HASHMAPPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/* 
 * Calculates the grave ratio of the given
 * hashmap assuming we insert a new element
 */
extern inline float _hashMapNextGraveRatio(
    const HashMap *hashMapPtr
){
    return 
        (((float)(hashMapPtr->_graveCount)) + 1.0f)
            / ((float)(hashMapPtr->_capacity));
}

/*
 * Calculates the load factor of the given
 * hashmap assuming we insert a new element
 */
extern inline float _hashMapNextLoadFactor(
    const HashMap *hashMapPtr
){
    return (((float)(hashMapPtr->size)) + 1.0f)
        / ((float)(hashMapPtr->_capacity));
}

/*
 * Rehashes the given hashmap if the next insertion
 * into it will put it above the maximum grave ratio, 
 * and also grows it if such an insertion would put
 * it above the maximum load factor; returns false as 
 * error code, true otherwise
 */
extern inline bool _hashMapRehashIfNeeded(
    HashMap *hashMapPtr,
    size_t slotSize
){
    enum{ growRatio = 2u };

    float graveRatio = _hashMapNextGraveRatio(
        hashMapPtr
    );
    if(graveRatio < _maxGraveRatio){
        return true;
    }
    float loadFactor = _hashMapNextLoadFactor(
        hashMapPtr
    );
    if(loadFactor > _maxLoadFactor){
        //TODO resize
    }
    //TODO how to do this? just allocate new stuff? 
}

/*
 * isGrave
 * isOccupied
 * resize
 * rehash
 * hasKeyPtr
 * hasKey
 * putPtr
 * put
 * replacePtr
 * replace
 * getPtr
 * get
 * remove (can remove all graves if new size 0)
 * valueApply
 * free
 */


#undef _maxGraveRatio
#undef _maxLoadFactor

#endif