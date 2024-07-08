#ifndef UNKNOWN_USERFUNCSET_H
#define UNKNOWN_USERFUNCSET_H

#include "Unknown_Object.h"

/*
 * A struct holding a user function and the name it
 * is to be associated with
 */
typedef struct _UNNameUserFuncPair{
    /* owns its name (heap copied) */
    char *_name;
    /* owns the pointer to its function also */
    UNObjectFunc *_funcPtr;
} _UNNameUserFuncPair;

/*
 * Creates and returns a new _UNNameUserFuncPair by
 * value; makes a heap copy of the name; takes
 * ownership of the function provided as a parameter
 */
_UNNameUserFuncPair _unNameUserFuncPairMake(
    const char *name,
    UNObjectFunc *funcPtr
);

/*
 * Frees the memory associated with the specified
 * _UNNameUserFuncPair
 */
void _unNameUserFuncPairFree(
    _UNNameUserFuncPair *nameUserFuncPairPtr
);

/*
 * Stores a set of mappings between string names and
 * UNObjectFunc pointers
 */
typedef struct UNUserFuncSet{
    /* arraylist of type _UNNameUserFuncPair */
    ArrayList _nameUserFuncPairs;
} UNUserFuncSet;

/*
 * Creates and returns a new empty UNUserFuncSet by
 * value
 */
UNUserFuncSet unUserFuncSetMake();

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
);

/*
 * Frees the memory associated with the specified
 * UNUserFuncSet
 */
void unUserFuncSetFree(
    UNUserFuncSet *userFuncSetPtr
);

#endif