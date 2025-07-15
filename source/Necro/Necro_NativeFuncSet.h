#ifndef NECRO_NATIVEFUNCSET_H
#define NECRO_NATIVEFUNCSET_H

#include "Necro_Object.h"

/*
 * A struct holding a native function and the name it
 * is to be associated with
 */
typedef struct _NecroNameNativeFuncPair{
    /* owns its name (heap copied) */
    char *_name;
    NecroNativeFunc _func;
} _NecroNameNativeFuncPair;

/*
 * Creates and returns a new _NecroNameNativeFuncPair
 * by value; makes a heap copy of the name
 */
_NecroNameNativeFuncPair _necroNameNativeFuncPairMake(
    const char *name,
    NecroNativeFunc func
);

/*
 * Frees the memory associated with the specified
 * _NecroNameNativeFuncPair
 */
void _necroNameNativeFuncPairFree(
    _NecroNameNativeFuncPair *nameNativeFuncPairPtr
);

/*
 * Stores a set of mappings between string names and
 * NecroNativeFuncs
 */
typedef struct NecroNativeFuncSet{
    /* arraylist of type _NecroNameNativeFuncPair */
    ArrayList _nameNativeFuncPairs;
} NecroNativeFuncSet;

/*
 * Creates and returns a new empty NecroNativeFuncSet
 * by value
 */
NecroNativeFuncSet necroNativeFuncSetMake();

/*
 * Adds the specified native function to the given
 * native function set under the specified name
 */
void necroNativeFuncSetAdd(
    NecroNativeFuncSet *nativeFuncSetPtr,
    const char *name,
    NecroNativeFunc func
);

/*
 * Frees the memory associated with the specified
 * NecroNativeFuncSet
 */
void necroNativeFuncSetFree(
    NecroNativeFuncSet *nativeFuncSetPtr
);

#endif