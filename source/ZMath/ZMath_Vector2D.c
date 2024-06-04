#include "ZMath_Vector2D.h"

#include <math.h>
#include <stdio.h>

/* Makes a value copy of the given Vector2D */
Vector2D vector2DCopy(Vector2D toCopy){
    return toCopy;
}

/* Copies the Vector2D source into destionation */
void vector2DCopyInto(
    Vector2D *destination, 
    const Vector2D *source
){
    destination->x = source->x;
    destination->y = source->y;
}

/* Flips the given Vector2D halfway around */
Vector2D vector2DNegate(Vector2D vector){
    vector.x = -vector.x;
    vector.y = -vector.y;
    return vector;
}

/* Converts the given Vector2D to a Point2D */
Point2D vector2DToPoint2D(Vector2D vector){
    Point2D toRet;
    toRet.x = vector.x;
    toRet.y = vector.y;
    return toRet;
}

/* 
 * Returns the result of adding the two 
 * given Vectors 
 */
Vector2D vector2DAdd(
    Vector2D vector1,
    Vector2D vector2
){
    /* pass by value */
    vector1.x += vector2.x;
    vector1.y += vector2.y;
    return vector1;
}

/* 
 * Returns the result of subtracting the second
 * Vector2D from the first 
 */
Vector2D vector2DSubtract(
    Vector2D vector1,
    Vector2D vector2
){
    /* pass by value */
    vector1.x -= vector2.x;
    vector1.y -= vector2.y;
    return vector1;
}

/* 
 * Returns the result of multiplying the given Vector2D 
 * by the given scalar 
 */
Vector2D vector2DMultiply(
    Vector2D vector,
    float scalar
){
    /* pass by value */
    vector.x *= scalar;
    vector.y *= scalar;
    return vector;
}

/* 
 * Returns the result of dividing the given Vector2D by 
 * the given scalar 
 */
Vector2D vector2DDivide(Vector2D vector, float scalar){
    /* pass by value */
    vector.x /= scalar;
    vector.y /= scalar;
    return vector;
}

/* Returns the magnitude of the given Vector2D */
float vector2DMagnitude(Vector2D vector){
    return sqrtf((vector.x * vector.x) 
        + (vector.y * vector.y));
}

/*
 * Returns the angle of the given Vector2D in degrees
 */
float vector2DAngle(Vector2D vector){
    return toDegrees(atan2f(-vector.y, vector.x));
}

/* 
 * Constructs the vector which transforms 
 * Point2D A to B 
 */
Vector2D vector2DFromAToB(Point2D a, Point2D b){
    Vector2D toRet;
    toRet.x = b.x - a.x;
    toRet.y = b.y - a.y;
    return toRet;
}

/* Prints the given Vector2D to the given C String */
void printVector2D(
    Vector2D vector, 
    char* charPtr, 
    int arraySize
){
    snprintf(
        charPtr, 
        arraySize, 
        "V(%.3f, %.3f)", 
        vector.x, 
        vector.y
    );
}