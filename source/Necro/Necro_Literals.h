#ifndef NECRO_LITERALS_H
#define NECRO_LITERALS_H

#include "Constructure.h"

#include "Necro_Value.h"

/* Holds a collection of literals for a program */
typedef struct NecroLiterals{
    /* an array list of NecroValues */
    ArrayList literals;

    /*
     * ptr hashmap of NecroObjectString* to NecroValue
     * to be copied by the virtual machine for string
     * interning
     */
    HashMap *stringMapPtr;
    bool ownsStringMap;
} NecroLiterals;

/*
 * Constructs and returns a new NecroLiterals by value;
 * the string map pointer is nullable, and if null is
 * passed, the new literals will allocate its own
 * string map and own it
 */
NecroLiterals unLiteralsMake(HashMap *stringMapPtr);

/*
 * Returns the NecroValue at the specified index
 * in the given NecroLiterals
 */
NecroValue necroLiteralsGet(
    NecroLiterals *literalsPtr,
    size_t index
);

/*
 * Pushes the specified NecroValue to the back of
 * the specified NecroLiterals and returns the index
 * where it was written
 */
size_t necroLiteralsPushBack(
    NecroLiterals *literalsPtr,
    NecroValue value
);

/*
 * Frees the memory associated with the specified
 * NecroLiterals
 */
void necroLiteralsFree(NecroLiterals *literalsPtr);

#endif