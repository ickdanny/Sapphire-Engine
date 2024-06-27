#ifndef UNKNOWN_LITERALS_H
#define UNKNOWN_LITERALS_H

#include "Constructure.h"

#include "Unknown_Value.h"

/* Holds a collection of literals for a program */
typedef struct UNLiterals{
    //todo: an array list of UNValue for now
    ArrayList literals;
} UNLiterals;

/* Constructs and returns a new UNLiterals by value */
UNLiterals unLiteralsMake();

/*
 * Returns the todo: UNValue at the specified index
 * in the given UNLiterals
 */
UNValue unLiteralsGet(
    UNLiterals *literalsPtr,
    size_t index
);

/*
 * Pushes the specified todo: UNValue to the back of
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