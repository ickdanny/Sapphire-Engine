#include "Necro_Literals.h"

#include "Necro_Object.h"

#define literalsInitCapacity 25

#define stringMapInitCapacity 50

/*
 * Constructs and returns a new NecroLiterals by value;
 * the string map pointer is nullable, and if null is
 * passed, the new literals will allocate its own
 * string map and own it
 */
NecroLiterals necroLiteralsMake(HashMap *stringMapPtr){
    NecroLiterals toRet = {0};
    toRet.literals = arrayListMake(NecroValue,
        literalsInitCapacity
    );

    /* if null passed, allocate new string map */
    if(!stringMapPtr){
        toRet.stringMapPtr= pgAlloc(
            1,
            sizeof(*(toRet.stringMapPtr))
        );
        *(toRet.stringMapPtr) = hashMapMake(
            NecroObjectString*,
            NecroValue,
            stringMapInitCapacity,
            _necroObjectStringPtrHash,
            _necroObjectStringPtrEquals
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
 * Returns the NecroValue at the specified index
 * in the given NecroLiterals
 */
NecroValue necroLiteralsGet(
    NecroLiterals *literalsPtr,
    size_t index
){
    return arrayListGet(NecroValue,
        &(literalsPtr->literals),
        index
    );
}

/*
 * Pushes the specified NecroValue to the back of
 * the specified NecroLiterals and returns the index
 * where it was written
 */
size_t necroLiteralsPushBack(
    NecroLiterals *literalsPtr,
    NecroValue value
){
    arrayListPushBack(NecroValue,
        &(literalsPtr->literals),
        value
    );
    return literalsPtr->literals.size - 1;
}

/* Used to free functions allocated by the compiler */
static void freeFunction(NecroValue *valuePtr){
    if(necroIsFunc(*valuePtr)){
        necroValueFree(*valuePtr);
        memset(valuePtr, 0, sizeof(*valuePtr));
    }
}

/*
 * Used to free non-string objects allocated by the
 * compiler
 */
static void freeNonString(NecroValue *valuePtr){
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
    NecroObjectString **stringPtrPtr
){
    NecroObjectString *stringPtr = *stringPtrPtr;
    /* strings are objects, so this cast is safe */
    NecroObject *objectPtr = (NecroObject*)stringPtr;
    unObjectFree(objectPtr);
}

/*
 * Frees the memory associated with the specified
 * NecroLiterals
 */
void unLiteralsFree(NecroLiterals *literalsPtr){
    /*
     * free objects on the heap (including compile
     * constant strings)
     */
    arrayListApply(NecroValue,
        &(literalsPtr->literals),
        freeFunction
    );
    arrayListApply(NecroValue,
        &(literalsPtr->literals),
        freeNonString
    );
    arrayListFree(NecroValue,
        &(literalsPtr->literals)
    );
    /*
     * free the hashmap if owned (the map itself
     * owns the strings)
     */
    if(literalsPtr->ownsStringMap){
        hashMapKeyApply(NecroObjectString*, NecroValue,
            literalsPtr->stringMapPtr,
            freeString
        );
        hashMapFree(NecroObjectString*, NecroValue,
            literalsPtr->stringMapPtr
        );
        pgFree(literalsPtr->stringMapPtr);
    }
    memset(literalsPtr, 0, sizeof(*literalsPtr));
}