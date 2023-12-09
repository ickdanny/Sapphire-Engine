#ifndef ZMATH_POLAR_H
#define ZMATH_POLAR_H

#include <string.h> /* for memcpy */

#include "ZMath_Vector.h"

/* A 2D vector stored in polar form */
typedef struct Polar{
    float magnitude;
    float angle;
    
    Vector asVector;
} Polar;

/* Makes a value copy of the given Polar */
extern inline Polar polarCopy(Polar toCopy){
    return toCopy;
}

/* Copies the Polar source into destination */
extern inline void polarCopyInto(Polar *destination, const Polar *source){
    memcpy(destination, source, sizeof(*source));
}

/* Updates the internal Cartesian representation of the given Polar */
extern inline void _polarUpdateVector(Polar *polarPtr){
    float radians = toRadians(polarPtr->angle);
    float magnitude = polarPtr->magnitude;
    polarPtr->asVector.x = magnitude * cosf(radians);
    polarPtr->asVector.y = -magnitude * sinf(radians);
}

/* Constructs a new Polar from a given Cartesian Vector */
extern inline Polar polarFromVector(Vector vector){
    Polar toRet = {0};
    toRet.magnitude = vectorMagnitude(vector);
    toRet.angle = vectorAngle(vector);
    toRet.asVector = vector;
    return toRet;
}

/* Sets the magnitude of the given Polar */
extern inline void polarSetMagnitude(
    Polar *polarPtr, 
    float magnitude
){
    polarPtr->magnitude = magnitude;
    _polarUpdateVector(polarPtr);
}

/* Sets the angle of the given Polar in degrees*/
extern inline void polarSetAngle(Polar *polarPtr, float angle){
    polarPtr->angle = angle;
    _polarUpdateVector(polarPtr);
}

/* Flips the given Polar halfway around */
extern inline Polar polarNegate(Polar polar){
    polarSetAngle(&polar, angleAdd(polar.angle, z_halfAngle));
    return polar;
}

/* Returns the Cartesian representation of the given Polar */
extern inline Vector polarToVector(Polar *polarPtr){
    return polarPtr->asVector;
}

/* Prints the given Polar to the given C String */
extern inline void printPolar(
    Polar *polarPtr, 
    char* charPtr, 
    int arraySize
){
    snprintf(
        charPtr, 
        arraySize, 
        "R(%.3f, %.3f)", 
        polarPtr->magnitude,
        polarPtr->angle
    );
}

#endif