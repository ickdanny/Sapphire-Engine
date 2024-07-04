#ifndef UNKNOWN_LITERALS_H
#define UNKNOWN_LITERALS_H

#include "Constructure.h"

#include "Unknown_Value.h"

/* Holds a collection of literals for a program */
typedef struct UNLiterals{
    /* an array list of UNValues */
    ArrayList literals;

    /*
     * hashmap of UNObjectString* to UNValue to be
     * copied by the virtual machine for string
     * interning
     */
    HashMap stringMap;
} UNLiterals;

/* Constructs and returns a new UNLiterals by value */
UNLiterals unLiteralsMake();

/*
 * Returns the UNValue at the specified index
 * in the given UNLiterals
 */
UNValue unLiteralsGet(
    UNLiterals *literalsPtr,
    size_t index
);

/*
 * Pushes the specified UNValue to the back of
 * the specified UNLiterals and returns the index
 * where it was written
 */
size_t unLiteralsPushBack(
    UNLiterals *literalsPtr,
    UNValue value
);

/*
 * Frees the memory associated with the specified
 * UNLiterals
 */
void unLiteralsFree(UNLiterals *literalsPtr);

#endif