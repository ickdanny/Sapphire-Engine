#include "Unknown_NativeFuncSet.h"

#define nameNativeFuncSetInitCapacity 20

/*
 * Creates and returns a new _UNNameNativeFuncPair by
 * value; makes a heap copy of the name
 */
_UNNameNativeFuncPair _unNameNativeFuncPairMake(
    const char *name,
    UNNativeFunc func
){
    assertNotNull(
        name,
        "null passed to name native func pair ctor; "
        SRC_LOCATION
    );
    int length = strlen(name);

    _UNNameNativeFuncPair toRet = {0};

    toRet._func = func;
    toRet._name = pgAlloc(length + 1, sizeof(char));
    strncpy(toRet._name, name, length);

    return toRet;
}

/*
 * Frees the memory associated with the specified
 * _UNNameNativeFuncPair
 */
void _unNameNativeFuncPairFree(
    _UNNameNativeFuncPair *nameNativeFuncPairPtr
){
    assertNotNull(
        nameNativeFuncPairPtr,
        "null passed to name func pair free; "
        SRC_LOCATION
    );
    pgFree(nameNativeFuncPairPtr->_name);
    nameNativeFuncPairPtr->_name = NULL;
    nameNativeFuncPairPtr->_func = NULL;
}

/*
 * Creates and returns a new empty UNNativeFuncSet by
 * value
 */
UNNativeFuncSet unNativeFuncSetMake(){
    UNNativeFuncSet toRet = {0};
    toRet._nameNativeFuncPairs = arrayListMake(
        _UNNameNativeFuncPair,
        nameNativeFuncSetInitCapacity
    );
    return toRet;
}

/*
 * Adds the specified native function to the given
 * native function set under the specified name
 */
void unNativeFuncSetAdd(
    UNNativeFuncSet *nativeFuncSetPtr,
    const char *name,
    UNNativeFunc func
){
    assertNotNull(
        nativeFuncSetPtr,
        "null set passed to nativeFuncSet add; "
        SRC_LOCATION
    );
    assertNotNull(
        name,
        "null name passed to nativeFuncSet add; "
        SRC_LOCATION
    );
    arrayListPushBack(
        _UNNameNativeFuncPair,
        &(nativeFuncSetPtr->_nameNativeFuncPairs),
        _unNameNativeFuncPairMake(
            name,
            func
        )
    );
}

/*
 * Frees the memory associated with the specified
 * UNNativeFuncSet
 */
void unNativeFuncSetFree(
    UNNativeFuncSet *nativeFuncSetPtr
){
    assertNotNull(
        nativeFuncSetPtr,
        "null passed to nativeFuncSet free; "
        SRC_LOCATION
    );
    /* free each individual func pair */
    arrayListApply(_UNNameNativeFuncPair,
        &(nativeFuncSetPtr->_nameNativeFuncPairs),
        _unNameNativeFuncPairFree
    );
    /* then free the array list */
    arrayListFree(_UNNameNativeFuncPair,
        &(nativeFuncSetPtr->_nameNativeFuncPairs)
    );
    memset(
        nativeFuncSetPtr,
        0,
        sizeof(*nativeFuncSetPtr)
    );
}