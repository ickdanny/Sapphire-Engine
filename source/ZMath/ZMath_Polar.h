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

/* Returns the sum of the two given Polars */
Polar polarAdd(Polar left, Polar right);

/*
 * Returns the difference between the two given Polars
 */
Polar polarSubtract(Polar left, Polar right);

/* Returns the scalar multiple of the given polar */
Polar polarMultiply(Polar polar, float scalar);

/*
 * Returns the scalar quotient of the given polar,
 * does not check for division by zero
 */
Polar polarDivide(Polar polar, float scalar);

/* 
 * Returns the result of adding the given Polar
 * to the specified Point2D
 */
Point2D point2DAddPolar(Point2D point, Polar polar);

/* 
 * Returns the result of subtracting the given Polar
 * from the specified Point2D
 */
Point2D point2DSubtractPolar(
    Point2D point,
    Polar polar
);

/* 
 * Constructs the Polar which transforms 
 * Point2D A to B 
 */
Polar polarFromAToB(Point2D a, Point2D b);

/* 
 * Returns the Cartesian representation of the 
 * given Polar 
 */
Vector2D polarToVector(Polar polar);

/* Prints the given Polar to the given C String */
void printPolar(
    Polar polar,
    char* charPtr, 
    int arraySize
);

#endif