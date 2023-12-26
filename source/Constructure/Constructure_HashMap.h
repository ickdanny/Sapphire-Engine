#ifndef CONSTRUCTURE_HASHMAP_H
#define CONSTRUCTURE_HASHMAP_H

#include "PGUtil.h"

/* 
 * The maximum ratio of touched slots in a
 * hashmap before a rehash occurs
 */
#define _maxTouchedRatio .75f
/*
 * The maximum ratio of occupied slots in a
 * hashmap before a resize occurs
 */
#define _maxLoadFactor .60f

/* A growable hash map on the heap */
typedef struct HashMap{
    /*
     * Points to an array on the heap with the
     * following layout: statusByte, key, value;
     * Status bytes have bit1 set to 1 if the
     * slot is occupied, and bit2 set to 1 if the
     * slot is a grave
     */
    void *_ptr;

    /* The number of currently occupied slots */
    size_t size;

    /* The number of allocated slots */
    size_t _capacity;

    /* 
     * The number of touched slots including 
     * currently occupied slots as well as graves
     */
    size_t _touchedCount;

    /* 
     * A function pointer to the hash function to
     * be used - hash functions must accept void*
     * and return size_t, and moreover must
     * agree with the given equals function
     */
    size_t (*_hashFunc)(void *);

    /*
     * A function pointer to the equals function to
     * be used - equals functions must accept void*
     * and return bool, and moreover must agree
     * with the given hash function
     */
    bool (*_equalsFunc)(void *, void *);

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
 * Dereferences the the status byte of
 * the given slot
 */
#define _dereferenceStatusByte(slotPtr) \
    (*((unsigned char *)(slotPtr)))

/*
 * Evaluates to true if the given slot
 * is occupied, false otherwise
 */
#define _isOccupied(slotPtr) \
    (_dereferenceStatusByte(slotPtr) & 0x01)

/*
 * Evaluates to true if the given slot
 * is a grave, false otherwise
 */
#define _isGrave(slotPtr) \
    (_dereferenceStatusByte(slotPtr) & 0x02)

/*
 * Evaluates to true if the given slot
 * is either a grave or is occupied
 */
#define _isTouched(slotPtr) \
    (_dereferenceStatusByte(slotPtr) & 0x03)

/*
 * Sets the status byte at the given slot to
 * occupied
 */
#define _setOccupied(slotPtr) \
    (_dereferenceStatusByte(slotPtr) = 0x01)

/*
 * Sets the status byte at the given slot to
 * grave
 */
#define _setGrave(slotPtr) \
    (_dereferenceStatusByte(slotPtr) = 0x02)

/*
 * Returns a void pointer to the key from the given
 * void pointer to a slot
 */
extern inline void *_getKeyPtr(void *slotPtr){
    return voidPtrAdd(slotPtr, 1);
}

/*
 * Returns a void pointer to the value from the
 * given void pointer to a slot and the keysize
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
    size_t (*hashFunc)(void *),
    bool (*equalsFunc)(void *),
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
    toRet._touchedCount = 0u;
    toRet._hashFunc = hashFunc;
    toRet._equalsFunc = equalsFunc;

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
    INIT_CAPACITY, \
    HASHFUNC, \
    EQUALSFUNC \
) \
    _hashMapMake( \
        INIT_CAPACITY, \
        HASHFUNC, \
        EQUALSFUNC, \
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
    INIT_CAPACITY, \
    HASHFUNC, \
    EQUALSFUNC \
) \
    _hashMapMake( \
        INIT_CAPACITY, \
        HASHFUNC, \
        EQUALSFUNC, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
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
    toRet._touchedCount 
        = toCopyPtr->_touchedCount;
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
    toRet._hashFunc = toCopyPtr->_hashFunc;
    toRet._equalsFunc = toCopyPtr->_equalsFunc;

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
    hashMapPtr->_touchedCount = 0u;
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
        _slotSize(KEYTYPENAME, VALUETYPENAME) \
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
 * Calculates the touched ratio of the given
 * hashmap assuming we insert a new element
 */
extern inline float _hashMapNextTouchedRatio(
    const HashMap *hashMapPtr
){
    return 
        (((float)(hashMapPtr->_touchedCount)) 
            + 1.0f)
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
 * Returns a pointer to the first empty slot 
 * to insert given a hash, a slot array, 
 * the length of that slot array, and the slot 
 * size
 */
extern inline void *_findEmptySlotToInsert(
    size_t hash,
    const void *slotArrayPtr,
    size_t slotArrayLength,
    size_t slotSize
){
    /* quadratic probe sequence */
    size_t increment = 1u;
    size_t index = hash % slotArrayLength;

    /* 
     * keep probing until we find unoccupied slot;
     * for insertion, do not care about graves
     */
    while(_isOccupied(
        voidPtrAdd(slotArrayPtr, index * slotSize)
    )){
        index += increment;
        index %= slotArrayLength;
        ++increment;
    }
    /* index now points to unoccupied slot*/
    return voidPtrAdd(
        slotArrayPtr, 
        index * slotSize
    );
}

/*
 * Rehashes the given hashmap if the next insertion
 * into it will put it above the maximum grave ratio, 
 * and also grows it if possible if such an insertion 
 * would put it above the maximum load factor; 
 * returns false as error code, true otherwise
 */
extern inline bool _hashMapRehashIfNeeded(
    HashMap *hashMapPtr,
    size_t slotSize
){
    enum{ growRatio = 2u };

    float graveRatio = _hashMapNextTouchedRatio(
        hashMapPtr
    );
    /* if grave ratio is low, do nothing */
    if(graveRatio < _maxTouchedRatio){
        return true;
    }
    /* at this point, grave ratio is high */
    float loadFactor = _hashMapNextLoadFactor(
        hashMapPtr
    );
    /* if load factor is high, grow the array */
    size_t newCapacity = 
        loadFactor > _maxLoadFactor
            ? (growRatio * hashMapPtr->_capacity)
            : hashMapPtr->_capacity;
    /* 
     * allocate new array; also important to
     * zero it
     */
    void *newPtr = pgAlloc(newCapacity, slotSize);
    /* return false if allocation failed */
    if(!newPtr){
        return false;
    }
    /* insert each K/V pair into new array */
    void *oldSlotPtr = hashMapPtr->_ptr;
    void *newSlotPtr = NULL;
    size_t hash;
    for(
        size_t oldIndex = 0u; 
        oldIndex < hashMapPtr->_capacity;
        ++oldIndex
    ){
        if(_isOccupied(oldSlotPtr)){
            /* 
             * recalculate hash from the stored 
             * value of the key
             */
            hash = hashMapPtr->_hashFunc(
                _getKeyPtr(oldSlotPtr)
            );
            /*
             * find proper slot in the new 
             * array to insert
             */
            newSlotPtr = _findEmptySlotToInsert(
                hash,
                newPtr,
                newCapacity,
                slotSize
            );
            /*
             * can memcpy the whole slot since
             * the status byte will be occupied
             */
            memcpy(
                newSlotPtr, 
                oldSlotPtr, 
                slotSize
            );
        }
        /* move to next slot in old array*/
        oldSlotPtr = voidPtrAdd(
            oldSlotPtr, 
            slotSize
        );
    }
    /* free old array and swap for new */
    pgFreeAndSwap(hashMapPtr->_ptr, newPtr);
    /* set new capacity */
    hashMapPtr->_capacity = newCapacity;
    /* 
     * the new slot array has no graves, so the
     * touched count should be set to the size
     */
    hashMapPtr->_touchedCount = hashMapPtr->size;
    /* note that size should stay the same */

    return true;
}

/*
 * Returns a pointer to the slot occupied by the
 * specified key given a hashmap and the slot size;
 * if no such slot exists, returns NULL; an optional
 * output void ** can be provided for writing the
 * first unoccupied slot found in the search
 */
extern inline void *_findSlotOfKey(
    const HashMap *hashMapPtr,
    const void *keyPtr,
    size_t slotSize,
    void **firstEmptySlotPtrPtr
){
    /* quadratic probe sequence */
    size_t increment = 1u;
    size_t hash = hashMapPtr->_hashFunc(keyPtr);
    size_t index = hash % hashMapPtr->size;

    /*
     * keep probing until we find a slot which
     * equals the keyPtr
     */
    void *firstEmptySlotPtr = NULL;
    void *slotPtr;
    while(_isTouched(
        slotPtr = voidPtrAdd(
            hashMapPtr->_ptr, 
            index * slotSize
        )
    )){
        /* check for equality if occupied */
        if(_isOccupied(slotPtr)){
            /* test to see if match found */
            if(hashMapPtr->_equalsFunc(
                keyPtr, 
                _getKeyPtr(slotPtr))
            ){
                /* if so, return that pointer */
                if(firstEmptySlotPtrPtr){
                    *firstEmptySlotPtrPtr 
                        = firstEmptySlotPtr;
                }
                return slotPtr;
            }
        }
        /* 
         * otherwise, if this is the first empty
         * slot, store it
         */
        else if(!firstEmptySlotPtr){
            firstEmptySlotPtr = slotPtr;
        }
        /* keep looking */
        index += increment;
        index %= hashMapPtr->size;
        ++increment;
    }
    /* 
     * finding an untouched slot means there is
     * no such element; return NULL in this case
     */
    if(firstEmptySlotPtrPtr){
        *firstEmptySlotPtrPtr 
            = firstEmptySlotPtr;
    }
    return NULL;
}

/* 
 * Returns true if the given hashmap has an entry
 * for the given key, false otherwise
 */
extern inline bool _hashMapHasKeyPtr(
    const HashMap *hashMapPtr,
    const void *keyPtr,
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

    /* cast the pointer to bool as return */
    return _findSlotOfKey(
        keyPtr, 
        hashMapPtr, 
        slotSize,
        NULL
    );
}

#ifndef _DEBUG
/* 
 * Returns true if the given hashmap of the
 * specified key and value types has an entry
 * for the given key, false otherwise
 */
#define hashMapHasKeyPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    _hashMapHasKeyPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME) \
    )
#else
/* 
 * Returns true if the given hashmap of the
 * specified key and value types has an entry
 * for the given key, false otherwise
 */
#define hashMapHasKeyPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    _hashMapHasKeyPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/*
 * Returns true if the given hashmap of the
 * specified key and value types has an entry
 * for the given key, false otherwise
 */
#define hashMapHasKey( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEY \
) \
    hashMapHasKeyPtr( \
        KEYTYPENAME, \
        VALUETYPENAME, \
        HASHMAPPTR, \
        &(KEY) /* equals takes void* anyway */ \
    )

/*
 * Returns a pointer to the value associated
 * with the given key in the given hashmap or
 * returns NULL if no such value exists
 */
extern inline void *_hashMapGetPtr(
    HashMap *hashMapPtr,
    const void *keyPtr,
    size_t slotSize,
    size_t keySize
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

    void *slotPtr = _findSlotOfKey(
        keyPtr,
        hashMapPtr,
        slotSize,
        NULL
    );
    if(slotPtr){
        return _getValuePtr(slotPtr, keySize);
    }
    else{
        return NULL;
    }
}

#ifndef _DEBUG
/*
 * Returns a pointer to the value associated
 * with the given key in the given hashmap of
 * the specified key and value types or
 * returns NULL if no such value exists
 */
#define hashMapGetPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    _hashMapGetPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME) \
    )
#else
/*
 * Returns a pointer to the value associated
 * with the given key in the given hashmap of
 * the specified key and value types or
 * returns NULL if no such value exists
 */
#define hashMapGetPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    _hashMapGetPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/*
 * Copies the specified value into the given
 * hashmap and associates it with the given
 * key, possibly replacing its previously
 * associated value; previously created
 * pointers into the hashmap may become
 * invalidated should a rehash operation occur
 */
extern inline void _hashMapPutPtr(
    HashMap *hashMapPtr,
    const void *keyPtr,
    const void *valuePtr,
    size_t slotSize,
    size_t keySize,
    size_t valueSize
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

    void *firstEmptySlotPtr = NULL;
    void *slotPtr = _findSlotOfKey(
        hashMapPtr,
        keyPtr,
        slotSize,
        &firstEmptySlotPtr
    );
    /* case 1: replace */
    if(_isOccupied(slotPtr)){
        /* case 1a: replace value in place */
        if(!firstEmptySlotPtr){
            memcpy(
                _getValuePtr(slotPtr, keySize),
                valuePtr,
                valueSize
            );
        }
        /* case 1b: replace in earlier slot */
        else{
            /* set previous slot to grave */
            memset(slotPtr, 0, slotSize);
            _setGrave(slotPtr);
            /* place key in earlier slot */
            memcpy(
                _getKeyPtr(firstEmptySlotPtr),
                keyPtr,
                keySize
            );
            /* place value in earlier slot */
            memcpy(
                _getValuePtr(
                    firstEmptySlotPtr,
                    keySize
                ),
                valuePtr,
                valueSize
            );
            /* set earlier slot to occupied */
            _setOccupied(firstEmptySlotPtr);
        }
    }
    /* case 2: insert */
    else{
        /* case 2a: no rehash */
        //TODO HOW TO CHECK IF REHASH OCCURRED?
    }
}

/*
 * putPtr
 * put
 * remove (can remove all graves if new size 0)
 * valueApply
 */

/* Frees the given hashmap */
extern inline void _hashMapFree(
    HashMap *hashMapPtr
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

    pgFree(hashMapPtr->_ptr);
    hashMapPtr->size = 0u;
    hashMapPtr->_capacity = 0u;
    hashMapPtr->_touchedCount = 0u;
    hashMapPtr->_hashFunc = NULL;
    hashMapPtr->_equalsFunc = NULL;
}

#undef _maxGraveRatio
#undef _maxLoadFactor

#endif