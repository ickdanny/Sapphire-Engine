#include "Unknown_Literals.h"

#define literalsInitCapacity 8

/* Constructs and returns a new UNLiterals by value */
UNLiterals unLiteralsMake(){
    UNLiterals toRet = {0};
    toRet.literals = arrayListMake(UNValue,
        literalsInitCapacity
    );
    return toRet;
}

/*
 * Returns the todo: UNValue at the specified index
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
 * Pushes the specified todo: UNValue to the back of
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
    /* free objects on the heap */
    arrayListApply(UNValue,
        &(literalsPtr->literals),
        _unValueFree
    );
    arrayListFree(UNValue, &(literalsPtr->literals));
    memset(literalsPtr, 0, sizeof(*literalsPtr));
}