#ifndef UNKNOWN_NATIVEFUNCSET_H
#define UNKNOWN_NATIVEFUNCSET_H

#include "Unknown_Object.h"

/*
 * A struct holding a native function and the name it
 * is to be associated with
 */
typedef struct _UNNameFuncPair{
    /* owns its name (heap copied) */
    char *_name;
    UNNativeFunc _func;
} _UNNameFuncPair;

/*
 * Creates and returns a new _UNNameFuncPair by value;
 * makes a heap copy of the name
 */
_UNNameFuncPair _unNameFuncPairMake(
    const char *name,
    UNNativeFunc func
);

/*
 * Frees the memory associated with the specified
 * _UNNameFuncPair
 */
void _unNameFuncPairFree(
    _UNNameFuncPair *nameFuncPairPtr
);

/*
 * Stores a set of mappings between string names and
 * UNNativeFuncs
 */
typedef struct UNNativeFuncSet{
    /* arraylist of type _UNNameFuncPair */
    ArrayList _nameFuncPairs;
} UNNativeFuncSet;

/*
 * Creates and returns a new empty UNNativeFuncSet by
 * value
 */
UNNativeFuncSet unNativeFuncSetMake();

/*
 * Adds the specified native function to the given
 * native function set under the specified name
 */
void unNativeFuncSetAdd(
    UNNativeFuncSet *nativeFuncSetPtr,
    const char *name,
    UNNativeFunc func
);

/*
 * Frees the memory associated with the specified
 * UNNativeFuncSet
 */
void unNativeFuncSetFree(
    UNNativeFuncSet *nativeFuncSetPtr
);

#endif