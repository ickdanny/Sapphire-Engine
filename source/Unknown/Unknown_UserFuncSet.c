#include "Unknown_UserFuncSet.h"

#define nameUserFuncSetInitCapacity 20

/*
 * Creates and returns a new _UNNameUserFuncPair by
 * value; makes a heap copy of the name; takes
 * ownership of the function provided as a parameter
 */
_UNNameUserFuncPair _unNameUserFuncPairMake(
    const char *name,
    UNObjectFunc *funcPtr
){
    assertNotNull(
        name,
        "null passed to name user func pair ctor; "
        SRC_LOCATION
    );
    assertNotNull(
        funcPtr,
        "null passed to name user func pair ctor; "
        SRC_LOCATION
    );
    int length = strlen(name);

    _UNNameUserFuncPair toRet = {0};

    toRet._funcPtr = funcPtr;
    toRet._name = pgAlloc(length + 1, sizeof(char));
    strncpy(toRet._name, name, length);

    return toRet;
}

/*
 * Frees the memory associated with the specified
 * _UNNameUserFuncPair
 */
void _unNameUserFuncPairFree(
    _UNNameUserFuncPair *nameUserFuncPairPtr
){
    assertNotNull(
        nameUserFuncPairPtr,
        "null passed to name func pair free; "
        SRC_LOCATION
    );
    pgFree(nameUserFuncPairPtr->_name);
    nameUserFuncPairPtr->_name = NULL;
    /* the pair holds ownership over the function */
    unObjectFree(
        (UNObject*)(nameUserFuncPairPtr->_funcPtr)
    );
    nameUserFuncPairPtr->_funcPtr = NULL;
}

/*
 * Creates and returns a new empty UNUserFuncSet by
 * value
 */
UNUserFuncSet unUserFuncSetMake(){
    UNUserFuncSet toRet = {0};
    toRet._nameUserFuncPairs = arrayListMake(
        _UNNameUserFuncPair,
        nameUserFuncSetInitCapacity
    );
    return toRet;
}

/*
 * Adds the specified user function to the given
 * user function set under the specified name; the set
 * takes ownership of the function provided as a
 * parameter
 */
void unUserFuncSetAdd(
    UNUserFuncSet *userFuncSetPtr,
    const char *name,
    UNObjectFunc *funcPtr
){
    assertNotNull(
        userFuncSetPtr,
        "null set passed to userFuncSet add; "
        SRC_LOCATION
    );
    assertNotNull(
        name,
        "null name passed to userFuncSet add; "
        SRC_LOCATION
    );
    assertNotNull(
        funcPtr,
        "null funcPtr passed to userFuncSet add; "
        SRC_LOCATION
    );
    arrayListPushBack(
        _UNNameUserFuncPair,
        &(userFuncSetPtr->_nameUserFuncPairs),
        _unNameUserFuncPairMake(
            name,
            funcPtr
        )
    );
}

/*
 * Frees the memory associated with the specified
 * UNUserFuncSet
 */
void unUserFuncSetFree(
    UNUserFuncSet *userFuncSetPtr
){
    assertNotNull(
        userFuncSetPtr,
        "null passed to userFuncSet free; "
        SRC_LOCATION
    );
    /* free each individual func pair */
    arrayListApply(_UNNameUserFuncPair,
        &(userFuncSetPtr->_nameUserFuncPairs),
        _unNameUserFuncPairFree
    );
    /* then free the array list */
    arrayListFree(_UNNameUserFuncPair,
        &(userFuncSetPtr->_nameUserFuncPairs)
    );
    memset(
        userFuncSetPtr,
        0,
        sizeof(*userFuncSetPtr)
    );
}