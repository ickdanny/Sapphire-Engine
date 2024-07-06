#include "Unknown_NativeFuncSet.h"

#define nameFuncSetInitCapacity 20

/*
 * Creates and returns a new _UNNameFuncPair by value;
 * makes a heap copy of the name
 */
_UNNameFuncPair _unNameFuncPairMake(
    const char *name,
    UNNativeFunc func
){
    assertNotNull(
        name,
        "null passed to name func pair ctor; "
        SRC_LOCATION
    );
    int length = strlen(name);

    _UNNameFuncPair toRet = {0};

    toRet._func = func;
    toRet._name = pgAlloc(length + 1, sizeof(char));
    strncpy(toRet._name, name, length);

    return toRet;
}

/*
 * Frees the memory associated with the specified
 * _UNNameFuncPair
 */
void _unNameFuncPairFree(
    _UNNameFuncPair *nameFuncPairPtr
){
    assertNotNull(
        nameFuncPairPtr,
        "null passed to name func pair free; "
        SRC_LOCATION
    );
    pgFree(nameFuncPairPtr->_name);
    nameFuncPairPtr->_name = NULL;
    nameFuncPairPtr->_func = NULL;
}

/*
 * Creates and returns a new empty UNNativeFuncSet by
 * value
 */
UNNativeFuncSet unNativeFuncSetMake(){
    UNNativeFuncSet toRet = {0};
    toRet._nameFuncPairs = arrayListMake(
        _UNNameFuncPair,
        nameFuncSetInitCapacity
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
        _UNNameFuncPair,
        &(nativeFuncSetPtr->_nameFuncPairs),
        _unNameFuncPairMake(
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
    arrayListFree(_UNNameFuncPair,
        &(nativeFuncSetPtr->_nameFuncPairs)
    );
    memset(
        nativeFuncSetPtr,
        0,
        sizeof(*nativeFuncSetPtr)
    );
}