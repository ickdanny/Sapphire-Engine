#ifndef UNKNOWN_VALUE_H
#define UNKNOWN_VALUE_H

#include <stdbool.h>

#include "PGUtil.h"
#include "ZMath.h"

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
    un_int,
    un_float,
    un_vector,
    un_point,
    un_object,
} UNValueType;

/* A tagged union of values for Unknown */
typedef struct UNValue{
    UNValueType type;
    union{
        bool boolean;
        int integer;
        float floating;
        Polar vector;
        Point2D point;
        UNObject *object;
    } as;
} UNValue;

/* Constructs a UNValue for the specified bool */
#define unBoolValue(BOOL) \
    ((UNValue){un_bool, {.boolean = (BOOL)}})

/* Constructs a UNValue for the specified int */
#define unIntValue(INT) \
    ((UNValue){un_int, {.integer = (INT)}})

/* Constructs a UNValue for the specified float */
#define unFloatValue(FLOAT) \
    ((UNValue){un_float, {.floating = (FLOAT)}})

/*
 * Constructs a UNValue for the specified polar vector
 */
#define unVectorValue(VECTOR) \
    ((UNValue){un_vector, {.vector = (VECTOR)}})

/* Constructs a UNValue for the specified point */
#define unPointValue(POINT) \
    ((UNValue){un_point, {.point = (POINT)}})

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
 * Returns true if the specified UNValue is an int,
 * false otherwise
 */
#define unIsInt(VALUE) ((VALUE).type == un_int)

/*
 * Returns true if the specified UNValue is a float,
 * false otherwise
 */
#define unIsFloat(VALUE) ((VALUE).type == un_float)

/*
 * Returns true if the specified UNValue is a vector,
 * false otherwise
 */
#define unIsVector(VALUE) ((VALUE).type == un_vector)

/*
 * Returns true if the specified UNValue is a point,
 * false otherwise
 */
#define unIsPoint(VALUE) ((VALUE).type == un_point)

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
 * Unboxes the specified UNValue as an int, error if
 * invalid tag
 */
static inline int unAsInt(UNValue value){
    if(unIsInt(value)){
        return value.as.integer;
    }
    else{
        pgError("bad access for integer");
        return 0;
    }
}

/*
 * Unboxes the specified UNValue as a float, error if
 * invalid tag
 */
static inline float unAsFloat(UNValue value){
    if(unIsFloat(value)){
        return value.as.floating;
    }
    else{
        pgError("bad access for float");
        return 0;
    }
}

/*
 * Unboxes the specified UNValue as a vector, error if
 * invalid tag
 */
static inline Polar unAsVector(UNValue value){
    if(unIsVector(value)){
        return value.as.vector;
    }
    else{
        pgError("bad access for vector");
        return (Polar){0};
    }
}

/*
 * Unboxes the specified UNValue as a point, error if
 * invalid tag
 */
static inline Point2D unAsPoint(UNValue value){
    if(unIsPoint(value)){
        return value.as.point;
    }
    else{
        pgError("bad access for point");
        return (Point2D){0};
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