#ifndef UNKNOWN_LITERALS_H
#define UNKNOWN_LITERALS_H

#include "Constructure.h"

#include "Unknown_Value.h"

/* Holds a collection of literals for a program */
typedef struct UNLiterals{
    /* an array list of UNValues */
    ArrayList literals;

    /*
     * ptr hashmap of UNObjectString* to UNValue to be
     * copied by the virtual machine for string
     * interning
     */
    HashMap *stringMapPtr;
    bool ownsStringMap;
} UNLiterals;

/*
 * Constructs and returns a new UNLiterals by value;
 * the string map pointer is nullable, and if null is
 * passed, the new literals will allocate its own
 * string map and own it
 */
UNLiterals unLiteralsMake(HashMap *stringMapPtr);

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