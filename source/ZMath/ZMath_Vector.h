#ifndef ZMATH_VECTOR_H
#define ZMATH_VECTOR_H

#include "ZMath_Point.h"
#include "ZMath_Angle.h"

/* A 2D vector stored in Cartesian form */
typedef struct Vector{
    float x;
    float y;
} Vector;

/* Makes a value copy of the given Vector */
Vector vectorCopy(Vector toCopy);

/* Copies the Vector source into destionation */
void vectorCopyInto(
    Vector *destination, 
    const Vector *source
);

/* Flips the given Vector halfway around */
Vector vectorNegate(Vector vector);

/* Converts the given Vector to a Point */
Point vectorToPoint(Vector vector);

/* 
 * Returns the result of adding the two 
 * given Vectors 
 */
Vector vectorAdd(Vector vector1, Vector vector2);

/* 
 * Returns the result of subtracting the second Vector 
 * from the first 
 */
Vector vectorSubtract(Vector vector1, Vector vector2);

/* 
 * Returns the result of multiplying the given Vector 
 * by the given scalar 
 */
Vector vectorMultiply(Vector vector, float scalar);

/* 
 * Returns the result of dividing the given Vector by 
 * the given scalar 
 */
Vector vectorDivide(Vector vector, float scalar);

/* Returns the magnitude of the given Vector */
float vectorMagnitude(Vector vector);

/* Returns the angle of the given Vector in degrees */
float vectorAngle(Vector vector);

/* 
 * Constructs the vector which transforms 
 * Point A to B 
 */
Vector vectorFromAToB(Point a, Point b);

/* Prints the given Vector to the given C String */
void printVector(
    Vector vector, 
    char* charPtr, 
    int arraySize
);

#endif