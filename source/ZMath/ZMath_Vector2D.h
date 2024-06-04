#ifndef ZMATH_VECTOR_H
#define ZMATH_VECTOR_H

#include "ZMath_Point2D.h"
#include "ZMath_Angle.h"

/* A 2D vector stored in Cartesian form */
typedef struct Vector2D{
    float x;
    float y;
} Vector2D;

/* Makes a value copy of the given Vector2D */
Vector2D vector2DCopy(Vector2D toCopy);

/* Copies the Vector2D source into destionation */
void vector2DCopyInto(
    Vector2D *destination, 
    const Vector2D *source
);

/* Flips the given Vector2D halfway around */
Vector2D vector2DNegate(Vector2D vector);

/* Converts the given Vector2D to a Point2D */
Point2D vector2DToPoint2D(Vector2D vector);

/* 
 * Returns the result of adding the two 
 * given Vectors 
 */
Vector2D vector2DAdd(
    Vector2D vector1,
    Vector2D vector2
);

/* 
 * Returns the result of subtracting the second
 * Vector2D from the first 
 */
Vector2D vector2DSubtract(
    Vector2D vector1,
    Vector2D vector2
);

/* 
 * Returns the result of multiplying the given Vector2D 
 * by the given scalar 
 */
Vector2D vector2DMultiply(
    Vector2D vector,
    float scalar
);

/* 
 * Returns the result of dividing the given Vector2D by 
 * the given scalar 
 */
Vector2D vector2DDivide(Vector2D vector, float scalar);

/* Returns the magnitude of the given Vector2D */
float vector2DMagnitude(Vector2D vector);

/* 
 * Returns the angle of the given Vector2D in degrees
 */
float vector2DAngle(Vector2D vector);

/* 
 * Constructs the vector which transforms 
 * Point2D A to B 
 */
Vector2D vector2DFromAToB(Point2D a, Point2D b);

/* Prints the given Vector2D to the given C String */
void printVector2D(
    Vector2D vector, 
    char* charPtr, 
    int arraySize
);

#endif