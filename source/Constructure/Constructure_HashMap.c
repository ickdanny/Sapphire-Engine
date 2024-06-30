#include "Constructure_HashMap.h"

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

/*
 * Returns a void pointer to the key from the given
 * void pointer to a slot
 */
void *_getKeyPtr(void *slotPtr){
    return voidPtrAdd(slotPtr, 1);
}

/*
 * Returns a void pointer to the value from the
 * given void pointer to a slot and the keysize
 */
void *_getValuePtr(void *slotPtr,size_t keySize){
    return voidPtrAdd(slotPtr, 1 + keySize);
}

/* Creates a hashmap and returns it by value */
HashMap _hashMapMake(
    size_t initCapacity,
    size_t (*hashFunc)(const void *),
    bool (*equalsFunc)(const void *, const void *),
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

/*
 * Makes a one level deep copy of the given
 * hashmap and returns it by value
 */
HashMap _hashMapCopy(
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

/*
 * Returns true if the given hashmap is empty,
 * false otherwise
 */
bool hashMapIsEmpty(const HashMap *hashMapPtr){
    return hashMapPtr->size == 0u;
}

/* Removes all elements of the given hashmap */
void _hashMapClear(
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

/* 
 * Calculates the touched ratio of the given
 * hashmap assuming we insert a new element
 */
float _hashMapNextTouchedRatio(
    const HashMap *hashMapPtr
){
    return (((float)(hashMapPtr->_touchedCount)) 
        + 1.0f)
            / ((float)(hashMapPtr->_capacity));
}

/*
 * Calculates the load factor of the given
 * hashmap assuming we insert a new element
 */
float _hashMapNextLoadFactor(
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
void *_findEmptySlotToInsert(
    size_t hash,
    void *slotArrayPtr,
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
rehashReturnCode _hashMapRehashIfNeeded(
    HashMap *hashMapPtr,
    size_t slotSize
){
    enum{ growRatio = 2u };

    float graveRatio = _hashMapNextTouchedRatio(
        hashMapPtr
    );
    /* if grave ratio is low, do nothing */
    if(graveRatio < _maxTouchedRatio){
        return _hashMapNoRehash;
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
        return _hashMapError;
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

    return _hashMapRehash;
}

/*
 * Returns a pointer to the slot occupied by the
 * specified key given a hashmap and the slot size;
 * if no such slot exists, returns NULL; an optional
 * output void ** can be provided for writing the
 * first unoccupied slot found in the search
 */
void *_findSlotOfKey(
    const HashMap *hashMapPtr,
    const void *keyPtr,
    size_t slotSize,
    void **firstEmptySlotPtrPtr
){
    /* quadratic probe sequence */
    size_t increment = 1u;
    size_t hash = hashMapPtr->_hashFunc(keyPtr);
    size_t index = hash % hashMapPtr->_capacity;

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
        index %= hashMapPtr->_capacity;
        ++increment;
    }
    /* 
     * finding an untouched slot means there is
     * no such element; return NULL in this case
     */
    if(!firstEmptySlotPtr){
        firstEmptySlotPtr = slotPtr;
    }
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
bool _hashMapHasKeyPtr(
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
        hashMapPtr,
        keyPtr, 
        slotSize,
        NULL
    );
}

/*
 * Returns a pointer to the value associated
 * with the given key in the given hashmap or
 * returns NULL if no such value exists
 */
void *_hashMapGetPtr(
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
        hashMapPtr,
        keyPtr,
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

/*
 * Returns a pointer to the key matching the given key
 * in the given hashmap or returns NULL if no such key
 * exists
 */
void *_hashMapGetKeyPtr(
    HashMap *hashMapPtr,
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

    void *slotPtr = _findSlotOfKey(
        hashMapPtr,
        keyPtr,
        slotSize,
        NULL
    );
    if(slotPtr){
        return _getKeyPtr(slotPtr);
    }
    else{
        return NULL;
    }
}

/*
 * Copies the specified value into the given
 * hashmap and associates it with the given
 * key, possibly replacing its previously
 * associated value; previously created
 * pointers into the hashmap may become
 * invalidated should a rehash operation occur
 */
void _hashMapPutPtr(
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
        switch(_hashMapRehashIfNeeded(
            hashMapPtr, 
            slotSize
        )){
            /* case 2a: rehash */
            case _hashMapRehash:
                /* need to find new slot */
                firstEmptySlotPtr = 
                    _findEmptySlotToInsert(
                        hashMapPtr->_hashFunc(
                            keyPtr
                        ),
                        hashMapPtr->_ptr,
                        hashMapPtr->_capacity,
                        slotSize
                    );
                /* FALL THROUGH */
            /* case 2b: no rehash */
            case _hashMapNoRehash:
                /* place key in empty slot */
                memcpy(
                    _getKeyPtr(firstEmptySlotPtr),
                    keyPtr,
                    keySize
                );
                /* place value in empty slot */
                memcpy(
                    _getValuePtr(
                        firstEmptySlotPtr,
                        keySize
                    ),
                    valuePtr,
                    valueSize
                );
                /* set empty slot to occupied */
                _setOccupied(firstEmptySlotPtr);
                /* increase size and touch count */
                ++hashMapPtr->size;
                ++hashMapPtr->_touchedCount;
                break;
            /* case 2c: error */
            case _hashMapError:
                pgError(
                    "failed to rehash; "
                    SRC_LOCATION
                );
                break;
        }
    }
}

/*
 * Removes the value associated with the given
 * key from the given hashmap if such a value
 * exists.
 */
void _hashMapRemovePtr(
    HashMap *hashMapPtr,
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

    void *slotPtr = _findSlotOfKey(
        hashMapPtr,
        keyPtr,
        slotSize,
        NULL
    );
    if(slotPtr){
        --hashMapPtr->size;
        /* 
         * if the last element was removed,
         * clear all the graves
         */
        if(!(hashMapPtr->size)){
            memset(
                hashMapPtr->_ptr,
                0,
                slotSize 
                    * hashMapPtr->_capacity
            );
        }
        /* otherwise, just remove element */
        else{
            memset(slotPtr, 0, slotSize);
            _setGrave(slotPtr);
        }
    }
}

/* Frees the given hashmap */
void _hashMapFree(
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