#include "Unknown_Literals.h"

#include "Unknown_Object.h"

#define literalsInitCapacity 25

#define stringMapInitCapacity 50

/*
 * Constructs and returns a new UNLiterals by value;
 * the string map pointer is nullable, and if null is
 * passed, the new literals will allocate its own
 * string map and own it
 */
UNLiterals unLiteralsMake(HashMap *stringMapPtr){
    UNLiterals toRet = {0};
    toRet.literals = arrayListMake(UNValue,
        literalsInitCapacity
    );

    /* if null passed, allocate new string map */
    if(!stringMapPtr){
        toRet.stringMapPtr= pgAlloc(
            1,
            sizeof(*(toRet.stringMapPtr))
        );
        *(toRet.stringMapPtr) = hashMapMake(
            UNObjectString*,
            UNValue,
            stringMapInitCapacity,
            _unObjectStringPtrHash,
            _unObjectStringPtrEquals
        );
        toRet.ownsStringMap = true;
    }
    /* otherwise it's a weak ptr */
    else{
        toRet.stringMapPtr = stringMapPtr;
        toRet.ownsStringMap = false;
    }
    return toRet;
}

/*
 * Returns the UNValue at the specified index
 * in the given UNLiterals
 */
UNValue unLiteralsGet(
    UNLiterals *literalsPtr,
    size_t index
){
    return arrayListGet(UNValue,
        &(literalsPtr->literals),
        index
    );
}

/*
 * Pushes the specified UNValue to the back of
 * the specified UNLiterals and returns the index
 * where it was written
 */
size_t unLiteralsPushBack(
    UNLiterals *literalsPtr,
    UNValue value
){
    arrayListPushBack(UNValue,
        &(literalsPtr->literals),
        value
    );
    return literalsPtr->literals.size - 1;
}

/* Used to free objects allocated by the compiler */
static void _unValueFree(UNValue *valuePtr){
    unValueFree(*valuePtr);
}

/*
 * Frees the memory associated with the specified
 * UNLiterals
 */
void unLiteralsFree(UNLiterals *literalsPtr){
    /*
     * free objects on the heap (including compile
     * constant strings)
     */
    arrayListApply(UNValue,
        &(literalsPtr->literals),
        _unValueFree
    );
    arrayListFree(UNValue, &(literalsPtr->literals));
    /*
     * free the hashmap if owned (the map itself
     * doesn't own anything, it just points into the
     * literals arraylist which was already freed)
     */
    if(literalsPtr->ownsStringMap){
        hashMapFree(UNObjectString*, UNValue,
            literalsPtr->stringMapPtr
        );
        pgFree(literalsPtr->stringMapPtr);
    }
    memset(literalsPtr, 0, sizeof(*literalsPtr));
}