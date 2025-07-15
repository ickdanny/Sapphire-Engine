#include "Necro_NativeFuncSet.h"

#define nameNativeFuncSetInitCapacity 20

/*
 * Creates and returns a new _NecroNameNativeFuncPair
 * by value; makes a heap copy of the name
 */
_NecroNameNativeFuncPair _necroNameNativeFuncPairMake(
    const char *name,
    NecroNativeFunc func
){
    assertNotNull(
        name,
        "null passed to name native func pair ctor; "
        SRC_LOCATION
    );
    int length = strlen(name);

    _NecroNameNativeFuncPair toRet = {0};

    toRet._func = func;
    toRet._name = pgAlloc(length + 1, sizeof(char));
    strncpy(toRet._name, name, length);

    return toRet;
}

/*
 * Frees the memory associated with the specified
 * _NecroNameNativeFuncPair
 */
void _necroNameNativeFuncPairFree(
    _NecroNameNativeFuncPair *nameNativeFuncPairPtr
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
 * Creates and returns a new empty NecroNativeFuncSet
 * by value
 */
NecroNativeFuncSet necroNativeFuncSetMake(){
    NecroNativeFuncSet toRet = {0};
    toRet._nameNativeFuncPairs = arrayListMake(
        _NecroNameNativeFuncPair,
        nameNativeFuncSetInitCapacity
    );
    return toRet;
}

/*
 * Adds the specified native function to the given
 * native function set under the specified name
 */
void necroNativeFuncSetAdd(
    NecroNativeFuncSet *nativeFuncSetPtr,
    const char *name,
    NecroNativeFunc func
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
        _NecroNameNativeFuncPair,
        &(nativeFuncSetPtr->_nameNativeFuncPairs),
        _necroNameNativeFuncPairMake(
            name,
            func
        )
    );
}

/*
 * Frees the memory associated with the specified
 * NecroNativeFuncSet
 */
void necroNativeFuncSetFree(
    NecroNativeFuncSet *nativeFuncSetPtr
){
    assertNotNull(
        nativeFuncSetPtr,
        "null passed to nativeFuncSet free; "
        SRC_LOCATION
    );
    /* free each individual func pair */
    arrayListApply(_NecroNameNativeFuncPair,
        &(nativeFuncSetPtr->_nameNativeFuncPairs),
        _necroNameNativeFuncPairFree
    );
    /* then free the array list */
    arrayListFree(_NecroNameNativeFuncPair,
        &(nativeFuncSetPtr->_nameNativeFuncPairs)
    );
    memset(
        nativeFuncSetPtr,
        0,
        sizeof(*nativeFuncSetPtr)
    );
}