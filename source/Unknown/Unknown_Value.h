#ifndef UNKNOWN_VALUE_H
#define UNKNOWN_VALUE_H

#include <stdbool.h>

#include "PGUtil.h"

/* forward declares */
typedef struct UNObject UNObject;
typedef struct UNObjectString UNObjectString;

/*
 * Enumerates the types of values that Unknown
 * supports
 */
typedef enum UNValueType{
    un_invalidValue,
    un_bool,
    un_number, //todo: double for now
    un_object,
} UNValueType;

/* A tagged union of values for Unknown */
typedef struct UNValue{
    UNValueType type;
    union{
        bool boolean;
        double number;
        UNObject *object;
    } as;
} UNValue;

/* Constructs a UNValue for the specified bool */
#define unBoolValue(BOOL) \
    ((UNValue){un_bool, {.boolean = (BOOL)}})

/* Constructs a UNValue for the specified number */
#define unNumberValue(NUMBER) \
    ((UNValue){un_number, {.number = (NUMBER)}})

/* Constructs a UNValue for the specified object */
#define unObjectValue(OBJECT) \
    ((UNValue){ \
        un_object, \
        {.object = (UNObject*)(OBJECT)} \
    })

/*
 * Returns true if the specified UNValue is a bool,
 * false otherwise
 */
#define unIsBool(VALUE) ((VALUE).type == un_bool)

/*
 * Returns true if the specified UNValue is a number,
 * false otherwise
 */
#define unIsNumber(VALUE) ((VALUE).type == un_number)

/*
 * Returns true if the specified UNValue is an object,
 * false otherwise
 */
#define unIsObject(VALUE) ((VALUE).type == un_object)

/*
 * Unboxes the specified UNValue as a bool, error if
 * invalid tag
 */
static inline bool unAsBool(UNValue value){
    if(unIsBool(value)){
        return value.as.boolean;
    }
    else{
        pgError("bad access for bool");
        return false;
    }
}

/*
 * Unboxes the specified UNValue as a number, error if
 * invalid tag
 */
static inline double unAsNumber(UNValue value){
    if(unIsNumber(value)){
        return value.as.number;
    }
    else{
        pgError("bad access for number");
        return 0;
    }
}

/*
 * Unboxes the specified UNValue as an object, error
 * if invalid tag
 */
static inline UNObject *unAsObject(UNValue value){
    if(unIsObject(value)){
        return value.as.object;
    }
    else{
        pgError("bad access for object");
        return 0;
    }
}

/*
 * Returns true if the two specified values are equal,
 * false otherwise
 */
bool unValueEquals(UNValue a, UNValue b);

/* prints the specified value */
void unValuePrint(UNValue value);

/*
 * Frees the memory associated with the specified value
 */
void unValueFree(UNValue value);

#endif