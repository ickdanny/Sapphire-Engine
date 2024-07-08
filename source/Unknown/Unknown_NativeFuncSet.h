#ifndef UNKNOWN_NATIVEFUNCSET_H
#define UNKNOWN_NATIVEFUNCSET_H

#include "Unknown_Object.h"

/*
 * A struct holding a native function and the name it
 * is to be associated with
 */
typedef struct _UNNameNativeFuncPair{
    /* owns its name (heap copied) */
    char *_name;
    UNNativeFunc _func;
} _UNNameNativeFuncPair;

/*
 * Creates and returns a new _UNNameNativeFuncPair by
 * value; makes a heap copy of the name
 */
_UNNameNativeFuncPair _unNameNativeFuncPairMake(
    const char *name,
    UNNativeFunc func
);

/*
 * Frees the memory associated with the specified
 * _UNNameNativeFuncPair
 */
void _unNameNativeFuncPairFree(
    _UNNameNativeFuncPair *nameNativeFuncPairPtr
);

/*
 * Stores a set of mappings between string names and
 * UNNativeFuncs
 */
typedef struct UNNativeFuncSet{
    /* arraylist of type _UNNameNativeFuncPair */
    ArrayList _nameNativeFuncPairs;
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