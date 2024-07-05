#ifndef CONSTRUCTURE_HASHMAP_H
#define CONSTRUCTURE_HASHMAP_H

#include "PGUtil.h"

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
    (slotPtr) && \
    (_dereferenceStatusByte(slotPtr) & 0x01)

/*
 * Evaluates to true if the given slot
 * is a grave, false otherwise
 */
#define _isGrave(slotPtr) \
    (slotPtr) && \
    (_dereferenceStatusByte(slotPtr) & 0x02)

/*
 * Evaluates to true if the given slot
 * is either a grave or is occupied
 */
#define _isTouched(slotPtr) \
    (slotPtr) && \
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

/* Returned by the rehash function */
typedef enum rehashReturnCode {
    _hashMapNoRehash = 0u,
    _hashMapRehash = 1u,
    _hashMapError = 2u
} rehashReturnCode;

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
    size_t (*_hashFunc)(const void *);

    /*
     * A function pointer to the equals function to
     * be used - equals functions must accept two
     * void* and return bool, and moreover must 
     * agree with the given hash function
     */
    bool (*_equalsFunc)(const void *, const void *);

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
 * Returns a void pointer to the key from the given
 * void pointer to a slot
 */
void *_getKeyPtr(void *slotPtr);

/*
 * Returns a void pointer to the value from the
 * given void pointer to a slot and the keysize
 */
void *_getValuePtr(void *slotPtr,size_t keySize);

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
HashMap _hashMapMake(
    size_t initCapacity,
    size_t (*hashFunc)(const void *),
    bool (*equalsFunc)(const void *, const void *),
    size_t slotSize
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
);

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
HashMap _hashMapCopy(
    size_t slotSize,
    const HashMap *toCopyPtr
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
);

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
 * Copies all key/value pairs from the second hashmap
 * into the first if an equal key is not present
 */
void _hashMapAddAllFrom(
    HashMap *hashMapPtr,
    HashMap *toCopyPtr,
    size_t slotSize,
    size_t keySize,
    size_t valueSize
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
);

#ifndef _DEBUG
/*
 * Copies all key/value pairs from the second hashmap
 * into the first with both hashmaps having the
 * specified key and value types if an equal key is
 * not present
 */
#define hashMapAddAllFrom( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    TOCOPYPTR \
) \
    _hashMapAddAllFrom( \
        (HASHMAPPTR), \
        (TOCOPYPTR), \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME), \
        sizeof(VALUETYPENAME) \
    )
#else
/*
 * Copies all key/value pairs from the second hashmap
 * into the first with both hashmaps having the
 * specified key and value types if an equal key is
 * not present
 */
#define hashMapAddAllFrom( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    TOCOPYPTR \
) \
    _hashMapAddAllFrom( \
        (HASHMAPPTR), \
        (TOCOPYPTR), \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME), \
        sizeof(VALUETYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/*
 * Returns true if the given hashmap is empty,
 * false otherwise
 */
bool hashMapIsEmpty(const HashMap *hashMapPtr);

/* Removes all elements of the given hashmap */
void _hashMapClear(
    HashMap *hashMapPtr,
    size_t slotSize
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
);

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
float _hashMapNextTouchedRatio(
    const HashMap *hashMapPtr
);

/*
 * Calculates the load factor of the given
 * hashmap assuming we insert a new element
 */
float _hashMapNextLoadFactor(
    const HashMap *hashMapPtr
);

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
);

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
);

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
);

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
);

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
        &(KEYTYPENAME){KEY} \
    )

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
);

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
    ((VALUETYPENAME*)(_hashMapGetPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME) \
    )))
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
    ((VALUETYPENAME*)(_hashMapGetPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )))
#endif

/*
 * Returns the value associated with the given
 * key in the given hashmap of the specified key
 * and value types; will cause an error if there
 * is no such value
 */
#define hashMapGet( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEY \
) \
    (*(hashMapGetPtr( \
        KEYTYPENAME, \
        VALUETYPENAME, \
        HASHMAPPTR, \
        &(KEYTYPENAME){KEY} \
    )))

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
);

#ifndef _DEBUG
/*
 * Returns a pointer to the key matching the given key
 * in the given hashmap of the specified key and value
 * types or returns NULL if no such value exists
 */
#define hashMapGetKeyPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    ((KEYTYPENAME*)(_hashMapGetKeyPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME) \
    )))
#else
/*
 * Returns a pointer to the key matching the given key
 * in the given hashmap of the specified key and value
 * types or returns NULL if no such value exists
 */
#define hashMapGetKeyPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    ((KEYTYPENAME*)(_hashMapGetKeyPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )))
#endif

/*
 * Returns the key matching the given key in the given
 * hashmap of the specified key and value types; will
 * cause an error if there is no such key
 */
#define hashMapGetKey( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEY \
) \
    (*(hashMapGetKeyPtr( \
        KEYTYPENAME, \
        VALUETYPENAME, \
        HASHMAPPTR, \
        &(KEYTYPENAME){KEY} \
    )))

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
);

#ifndef _DEBUG
/*
 * Copies the specified value into the given
 * hashmap of the specified key and value types
 * and associates it with the given
 * key, possibly replacing its previously
 * associated value; previously created
 * pointers into the hashmap may become
 * invalidated should a rehash operation occur
 */
#define hashMapPutPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR, \
    VALUEPTR \
) \
    _hashMapPutPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        VALUEPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME), \
        sizeof(VALUETYPENAME) \
    )
#else
/*
 * Copies the specified value into the given
 * hashmap of the specified key and value types
 * and associates it with the given
 * key, possibly replacing its previously
 * associated value; previously created
 * pointers into the hashmap may become
 * invalidated should a rehash operation occur
 */
#define hashMapPutPtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR, \
    VALUEPTR \
) \
    _hashMapPutPtr( \
        HASHMAPPTR, \
        KEYPTR, \
        VALUEPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        sizeof(KEYTYPENAME), \
        sizeof(VALUETYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/*
 * Inserts the given value into the given
 * hashmap of the specified key and value types
 * and associates it with the given key, possibly
 * replacing its previously associated value;
 * previously created pointers into the hashmap
 * may become invalidated should a rehash
 * operation occur
 */
#define hashMapPut( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEY, \
    VALUE \
) \
    hashMapPutPtr( \
        KEYTYPENAME, \
        VALUETYPENAME, \
        HASHMAPPTR, \
        &(KEYTYPENAME){KEY}, \
        &(VALUETYPENAME){VALUE} \
    )

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
);

#ifndef _DEBUG
/*
 * Removes the value associated with the given
 * key from the given hashmap of the specified
 * key and value types if such a value exists.
 */
#define hashMapRemovePtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    _hashMapRemovePtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME) \
    )
#else
/*
 * Removes the value associated with the given
 * key from the given hashmap of the specified
 * key and value types if such a value exists.
 */
#define hashMapRemovePtr( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEYPTR \
) \
    _hashMapRemovePtr( \
        HASHMAPPTR, \
        KEYPTR, \
        _slotSize(KEYTYPENAME, VALUETYPENAME), \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

/*
 * Removes the value associated with the given
 * key from the given hashmap of the specified
 * key and value types if such a value exists.
 */
#define hashMapRemove( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    KEY \
) \
    hashMapRemovePtr( \
        KEYTYPENAME, \
        VALUETYPENAME, \
        HASHMAPPTR, \
        &(KEYTYPENAME){KEY} \
    )

/*
 * Apply must be done via macro because
 * it expects pointers of the value type
 */
#ifndef _DEBUG
/*
 * Applies the given function to each value
 * in the given hashmap in no guaranteed
 * order; the function takes a pointer
 * of the value type
 */
#define hashMapApply( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    FUNC \
) \
    do{ \
        size_t slotSize = _slotSize( \
            KEYTYPENAME, \
            VALUETYPENAME \
        ); \
        void *currentSlot \
            = (HASHMAPPTR)->_ptr; \
        for( \
            size_t u = 0u; \
            u < (HASHMAPPTR)->_capacity; \
            ++u \
        ){ \
            if(_isOccupied(currentSlot)){ \
                FUNC( \
                    (VALUETYPENAME*) \
                    (_getValuePtr( \
                        currentSlot, \
                        sizeof(KEYTYPENAME) \
                    )) \
                ); \
            } \
            currentSlot = voidPtrAdd( \
                currentSlot, \
                slotSize \
            ); \
        } \
    } while(false)
#else
/*
 * Applies the given function to each value
 * in the given hashmap in no guaranteed
 * order; the function takes a pointer
 * of the value type
 */
#define hashMapApply( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    FUNC \
) \
    do{ \
        _hashMapPtrTypeCheck( \
            #KEYTYPENAME, \
            #VALUETYPENAME, \
            HASHMAPPTR \
        ); \
        size_t slotSize = _slotSize( \
            KEYTYPENAME, \
            VALUETYPENAME \
        ); \
        void *currentSlot \
            = (HASHMAPPTR)->_ptr; \
        for( \
            size_t u = 0u; \
            u < (HASHMAPPTR)->_capacity; \
            ++u \
        ){ \
            if(_isOccupied(currentSlot)){ \
                FUNC( \
                    (VALUETYPENAME*) \
                    (_getValuePtr( \
                        currentSlot, \
                        sizeof(KEYTYPENAME) \
                    )) \
                ); \
            } \
            currentSlot = voidPtrAdd( \
                currentSlot, \
                slotSize \
            ); \
        } \
    } while(false)
#endif

/*
 * KeyApply must be done via macro because
 * it expects pointers of the key type
 */
#ifndef _DEBUG
/*
 * Applies the given function to each key in the given
 * hashmap in no guaranteed order; the function takes
 * a pointer of the key type
 */
#define hashMapKeyApply( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    FUNC \
) \
    do{ \
        size_t slotSize = _slotSize( \
            KEYTYPENAME, \
            VALUETYPENAME \
        ); \
        void *currentSlot \
            = (HASHMAPPTR)->_ptr; \
        for( \
            size_t u = 0u; \
            u < (HASHMAPPTR)->_capacity; \
            ++u \
        ){ \
            if(_isOccupied(currentSlot)){ \
                FUNC( \
                    (KEYTYPENAME*) \
                    (_getKeyPtr(currentSlot)) \
                ); \
            } \
            currentSlot = voidPtrAdd( \
                currentSlot, \
                slotSize \
            ); \
        } \
    } while(false)
#else
/*
 * Applies the given function to each key in the given
 * hashmap in no guaranteed order; the function takes
 * a pointer of the key type
 */
#define hashMapKeyApply( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    FUNC \
) \
    do{ \
        _hashMapPtrTypeCheck( \
            #KEYTYPENAME, \
            #VALUETYPENAME, \
            HASHMAPPTR \
        ); \
        size_t slotSize = _slotSize( \
            KEYTYPENAME, \
            VALUETYPENAME \
        ); \
        void *currentSlot \
            = (HASHMAPPTR)->_ptr; \
        for( \
            size_t u = 0u; \
            u < (HASHMAPPTR)->_capacity; \
            ++u \
        ){ \
            if(_isOccupied(currentSlot)){ \
                FUNC( \
                    (KEYTYPENAME*) \
                    (_getKeyPtr(currentSlot)) \
                ); \
            } \
            currentSlot = voidPtrAdd( \
                currentSlot, \
                slotSize \
            ); \
        } \
    } while(false)
#endif

/*
 * KeyValueApply must be done via macro because it
 * expects pointers of key and value types
 */
#ifndef _DEBUG
/*
 * Applies the given function to each key/value pair in
 * the given hashmap in no guaranteed order; the
 * function's first parameter is a pointer of the key
 * type and its second parameter is a pointer of the
 * value type
 */
#define hashMapKeyValueApply( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    FUNC \
) \
    do{ \
        size_t slotSize = _slotSize( \
            KEYTYPENAME, \
            VALUETYPENAME \
        ); \
        void *currentSlot \
            = (HASHMAPPTR)->_ptr; \
        for( \
            size_t u = 0u; \
            u < (HASHMAPPTR)->_capacity; \
            ++u \
        ){ \
            if(_isOccupied(currentSlot)){ \
                FUNC( \
                    (KEYTYPENAME*) \
                    (_getKeyPtr(currentSlot)), \
                    (VALUETYPENAME*) \
                    (_getValuePtr( \
                        currentSlot, \
                        sizeof(KEYTYPENAME) \
                    )) \
                ); \
            } \
            currentSlot = voidPtrAdd( \
                currentSlot, \
                slotSize \
            ); \
        } \
    } while(false)
#else
/*
 * Applies the given function to each key/value pair in
 * the given hashmap in no guaranteed order; the
 * function's first parameter is a pointer of the key
 * type and its second parameter is a pointer of the
 * value type
 */
#define hashMapKeyValueApply( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR, \
    FUNC \
) \
    do{ \
        _hashMapPtrTypeCheck( \
            #KEYTYPENAME, \
            #VALUETYPENAME, \
            HASHMAPPTR \
        ); \
        size_t slotSize = _slotSize( \
            KEYTYPENAME, \
            VALUETYPENAME \
        ); \
        void *currentSlot \
            = (HASHMAPPTR)->_ptr; \
        for( \
            size_t u = 0u; \
            u < (HASHMAPPTR)->_capacity; \
            ++u \
        ){ \
            if(_isOccupied(currentSlot)){ \
                FUNC( \
                    (KEYTYPENAME*) \
                    (_getKeyPtr(currentSlot)), \
                    (VALUETYPENAME*) \
                    (_getValuePtr( \
                        currentSlot, \
                        sizeof(KEYTYPENAME) \
                    )) \
                ); \
            } \
            currentSlot = voidPtrAdd( \
                currentSlot, \
                slotSize \
            ); \
        } \
    } while(false)
#endif

/* Frees the given hashmap */
void _hashMapFree(
    HashMap *hashMapPtr
    #ifdef _DEBUG 
    , const char *keyTypeName
    , const char *valueTypeName
    #endif
);

#ifndef _DEBUG
/*
 * Frees the given hashmap of the specified
 * key and value types
 */
#define hashMapFree( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR \
) \
    _hashMapFree(HASHMAPPTR)
#else
/*
 * Frees the given hashmap of the specified
 * key and value types
 */
#define hashMapFree( \
    KEYTYPENAME, \
    VALUETYPENAME, \
    HASHMAPPTR \
) \
    _hashMapFree( \
        HASHMAPPTR, \
        #KEYTYPENAME, \
        #VALUETYPENAME \
    )
#endif

#endif