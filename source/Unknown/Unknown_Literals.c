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

/* Used to free functions allocated by the compiler */
static void freeFunction(UNValue *valuePtr){
    if(unIsFunc(*valuePtr)){
        unValueFree(*valuePtr);
        memset(valuePtr, 0, sizeof(*valuePtr));
    }
}

/*
 * Used to free non-string objects allocated by the
 * compiler
 */
static void freeNonString(UNValue *valuePtr){
    if(unIsObject(*valuePtr)
        && !unIsString(*valuePtr)
    ){
        unValueFree(*valuePtr);
        memset(valuePtr, 0, sizeof(*valuePtr));
    }
}

/*
 * Used to free string objects allocated by the
 * compiler
 */
static void freeString(
    UNObjectString **stringPtrPtr
){
    UNObjectString *stringPtr = *stringPtrPtr;
    /* strings are objects, so this cast is safe */
    UNObject *objectPtr = (UNObject*)stringPtr;
    unObjectFree(objectPtr);
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
        freeFunction
    );
    arrayListApply(UNValue,
        &(literalsPtr->literals),
        freeNonString
    );
    arrayListFree(UNValue, &(literalsPtr->literals));
    /*
     * free the hashmap if owned (the map itself
     * owns the strings)
     */
    if(literalsPtr->ownsStringMap){
        hashMapKeyApply(UNObjectString*, UNValue,
            literalsPtr->stringMapPtr,
            freeString
        );
        hashMapFree(UNObjectString*, UNValue,
            literalsPtr->stringMapPtr
        );
        pgFree(literalsPtr->stringMapPtr);
    }
    memset(literalsPtr, 0, sizeof(*literalsPtr));
}