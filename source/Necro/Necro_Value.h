#ifndef NECRO_VALUE_H
#define NECRO_VALUE_H

#include <stdbool.h>

#include "PGUtil.h"
#include "ZMath.h"

/* forward declares */
typedef struct NecroObject NecroObject;
typedef struct NecroObjectString NecroObjectString;

/*
 * Enumerates the types of values that Necro
 * supports
 */
typedef enum NecroValueType{
    necro_invalidValue,
    necro_bool,
    necro_int,
    necro_float,
    necro_vector,
    necro_point,
    necro_object,
} NecroValueType;

/* A tagged union of values for Necro */
typedef struct NecroValue{
    NecroValueType type;
    union{
        bool boolean;
        int integer;
        float floating;
        Polar vector;
        Point2D point;
        NecroObject *object;
    } as;
} NecroValue;

/* Constructs a NecroValue for the specified bool */
#define necroBoolValue(BOOL) \
    ((NecroValue){necro_bool, {.boolean = (BOOL)}})

/* Constructs a NecroValue for the specified int */
#define necroIntValue(INT) \
    ((NecroValue){necro_int, {.integer = (INT)}})

/* Constructs a NecroValue for the specified float */
#define necroFloatValue(FLOAT) \
    ((NecroValue){necro_float, {.floating = (FLOAT)}})

/*
 * Constructs a NecroValue for the specified polar
 * vector
 */
#define necroVectorValue(VECTOR) \
    ((NecroValue){necro_vector, {.vector = (VECTOR)}})

/* Constructs a NecroValue for the specified point */
#define necroPointValue(POINT) \
    ((NecroValue){necro_point, {.point = (POINT)}})

/* Constructs a NecroValue for the specified object */
#define necroObjectValue(OBJECT) \
    ((NecroValue){ \
        necro_object, \
        {.object = (NecroObject*)(OBJECT)} \
    })

/*
 * Returns true if the specified NecroValue is a bool,
 * false otherwise
 */
#define necroIsBool(VALUE) ((VALUE).type == necro_bool)

/*
 * Returns true if the specified NecroValue is an int,
 * false otherwise
 */
#define necroIsInt(VALUE) ((VALUE).type == necro_int)

/*
 * Returns true if the specified NecroValue is a float,
 * false otherwise
 */
#define necroIsFloat(VALUE) \
    ((VALUE).type == necro_float)

/*
 * Returns true if the specified NecroValue is a
 * vector, false otherwise
 */
#define necroIsVector(VALUE) \
    ((VALUE).type == necro_vector)

/*
 * Returns true if the specified NecroValue is a point,
 * false otherwise
 */
#define necroIsPoint(VALUE) \
    ((VALUE).type == necro_point)

/*
 * Returns true if the specified NecroValue is an
 * object, false otherwise
 */
#define necroIsObject(VALUE) \
    ((VALUE).type == necro_object)

/*
 * Unboxes the specified NecroValue as a bool, error if
 * invalid tag
 */
static inline bool necroAsBool(NecroValue value){
    if(necroIsBool(value)){
        return value.as.boolean;
    }
    else{
        pgError("bad access for bool; " SRC_LOCATION);
        return false;
    }
}

/*
 * Unboxes the specified NecroValue as an int, error if
 * invalid tag
 */
static inline int necroAsInt(NecroValue value){
    if(necroIsInt(value)){
        return value.as.integer;
    }
    else{
        pgError(
            "bad access for integer;"
            SRC_LOCATION
        );
        return 0;
    }
}

/*
 * Unboxes the specified NecroValue as a float, error
 * if invalid tag
 */
static inline float necroAsFloat(NecroValue value){
    if(necroIsFloat(value)){
        return value.as.floating;
    }
    else{
        pgError("bad access for float; " SRC_LOCATION);
        return 0;
    }
}

/*
 * Unboxes the specified NecroValue as a vector, error
 * if invalid tag
 */
static inline Polar necroAsVector(NecroValue value){
    if(necroIsVector(value)){
        return value.as.vector;
    }
    else{
        pgError(
            "bad access for vector; "
            SRC_LOCATION
        );
        return (Polar){0};
    }
}

/*
 * Unboxes the specified NecroValue as a point, error
 * if invalid tag
 */
static inline Point2D necroAsPoint(NecroValue value){
    if(necroIsPoint(value)){
        return value.as.point;
    }
    else{
        pgError("bad access for point; " SRC_LOCATION);
        return (Point2D){0};
    }
}

/*
 * Unboxes the specified NecroValue as an object, error
 * if invalid tag
 */
static inline NecroObject *necroAsObject(
    NecroValue value
){
    if(necroIsObject(value)){
        return value.as.object;
    }
    else{
        pgError(
            "bad access for object; " SRC_LOCATION
        );
        return 0;
    }
}

/*
 * Returns true if the two specified values are equal,
 * false otherwise
 */
bool necroValueEquals(NecroValue a, NecroValue b);

/* prints the specified value */
void necroValuePrint(NecroValue value);

/*
 * Frees the memory associated with the specified value
 */
void necroValueFree(NecroValue value);

#endif