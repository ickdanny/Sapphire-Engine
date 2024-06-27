#ifndef UNKNOWN_LITERALS_H
#define UNKNOWN_LITERALS_H

#include "Constructure.h"

/*
 * Enumerates the types of values that Unknown
 * supports
 */
typedef enum UNValueType{
    un_invalidValue,
    un_bool,
    un_number, //todo: double for now
} UNValueType;

/* A tagged union of values for Unknown */
typedef struct UNValue{
    UNValueType type;
    union{
        bool boolean;
        double number;
    } as;
} UNValue;

/* Constructs a UNValue for the specified bool */
#define unBoolValue(BOOL) \
    ((UNValue){un_bool, {.boolean = (BOOL)}})

/* Constructs a UNValue for the specified number */
#define unNumberValue(NUMBER) \
    ((UNValue){un_number, {.number = (NUMBER)}})

/*
 * Unboxes the specified UNValue as a bool, error if
 * invalid tag
 */
#define unAsBool(UNVALUE) \
    ((UNVALUE.type == un_bool) \
        ? (UNVALUE).as.boolean \
        : (pgError("bad access for bool"), false))

/*
 * Unboxes the specified UNValue as a number, error if
 * invalid tag
 */
#define unAsNumber(UNVALUE) \
    ((UNVALUE.type == un_number) \
        ? (UNVALUE).as.number \
        : (pgError("bad access for number"), 0.0))

/* todo: prints the specified value */
void unValuePrint(UNValue value);

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