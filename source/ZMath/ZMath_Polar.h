#ifndef ZMATH_POLAR_H
#define ZMATH_POLAR_H

#include "ZMath_Vector2D.h"

/* A 2D vector stored in polar form */
typedef struct Polar{
    float magnitude;
    float angle; /* stored in degrees */
} Polar;

/* Makes a value copy of the given Polar */
Polar polarCopy(Polar toCopy);

/* Copies the Polar source into destination */
void polarCopyInto(
    Polar *destination,
    const Polar *source
);

/* 
 * Constructs a new Polar from a given 
 * Cartesian Vector2D 
 */
Polar polarFromVector(Vector2D vector);

/* Sets the magnitude of the given Polar */
void polarSetMagnitude(
    Polar *polarPtr, 
    float magnitude
);

/* Sets the angle of the given Polar in degrees*/
void polarSetAngle(Polar *polarPtr, float angle);

/* Flips the given Polar halfway around */
Polar polarNegate(Polar polar);

/* 
 * Returns the Cartesian representation of the 
 * given Polar 
 */
Vector2D polarToVector(Polar *polarPtr);

/* Prints the given Polar to the given C String */
void printPolar(
    Polar *polarPtr, 
    char* charPtr, 
    int arraySize
);

#endif