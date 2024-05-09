#include "ZMath_Vector.h"

#include <math.h>
#include <stdio.h>

/* Makes a value copy of the given Vector */
Vector vectorCopy(Vector toCopy){
    return toCopy;
}

/* Copies the Vector source into destionation */
void vectorCopyInto(
    Vector *destination, 
    const Vector *source
){
    destination->x = source->x;
    destination->y = source->y;
}

/* Flips the given Vector halfway around */
Vector vectorNegate(Vector vector){
    vector.x = -vector.x;
    vector.y = -vector.y;
    return vector;
}

/* Converts the given Vector to a Point */
Point vectorToPoint(Vector vector){
    Point toRet;
    toRet.x = vector.x;
    toRet.y = vector.y;
    return toRet;
}

/* 
 * Returns the result of adding the two 
 * given Vectors 
 */
Vector vectorAdd(Vector vector1, Vector vector2){
    /* pass by value */
    vector1.x += vector2.x;
    vector1.y += vector2.y;
    return vector1;
}

/* 
 * Returns the result of subtracting the second Vector 
 * from the first 
 */
Vector vectorSubtract(Vector vector1, Vector vector2){
    /* pass by value */
    vector1.x -= vector2.x;
    vector1.y -= vector2.y;
    return vector1;
}

/* 
 * Returns the result of multiplying the given Vector 
 * by the given scalar 
 */
Vector vectorMultiply(Vector vector, float scalar){
    /* pass by value */
    vector.x *= scalar;
    vector.y *= scalar;
    return vector;
}

/* 
 * Returns the result of dividing the given Vector by 
 * the given scalar 
 */
Vector vectorDivide(Vector vector, float scalar){
    /* pass by value */
    vector.x /= scalar;
    vector.y /= scalar;
    return vector;
}

/* Returns the magnitude of the given Vector */
float vectorMagnitude(Vector vector){
    return sqrtf((vector.x * vector.x) 
        + (vector.y * vector.y));
}

/* Returns the angle of the given Vector in degrees */
float vectorAngle(Vector vector){
    return toDegrees(atan2f(-vector.y, vector.x));
}

/* 
 * Constructs the vector which transforms 
 * Point A to B 
 */
Vector vectorFromAToB(Point a, Point b){
    Vector toRet;
    toRet.x = b.x - a.x;
    toRet.y = b.y - a.y;
    return toRet;
}

/* Prints the given Vector to the given C String */
void printVector(
    Vector vector, 
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