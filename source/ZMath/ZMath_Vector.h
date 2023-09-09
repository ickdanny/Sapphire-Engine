#ifndef ZMATH_VECTOR_H
#define ZMATH_VECTOR_H

#include <math.h>
#include "ZMath_Point.h"
#include "ZMath_Angle.h"

typedef struct Vector{
    float x;
    float y;
} Vector;

extern inline Vector vectorCopy(Vector toCopy){
    return toCopy;
}

extern inline void vectorCopyInto(Vector *destination, Vector source){
    destination->x = source.x;
    destination->y = source.y;
}

extern inline Vector vectorNegate(Vector vector){
    vector.x = -vector.x;
    vector.y = -vector.y;
    return vector;
}

extern inline Point vectorToPoint(Vector vector){
    Point toRet;
    toRet.x = vector.x;
    toRet.y = vector.y;
    return toRet;
}

extern inline Vector vectorAdd(Vector vector1, Vector vector2){
    vector1.x += vector2.x;
    vector1.y += vector2.y;
    return vector1;
}

extern inline Vector vectorSubtract(Vector vector1, Vector vector2){
    vector1.x -= vector2.x;
    vector1.y -= vector2.y;
    return vector1;
}

extern inline Vector vectorMultiply(Vector vector, float scalar){
    vector.x *= scalar;
    vector.y *= scalar;
    return vector;
}

extern inline Vector vectorDivide(Vector vector, float scalar){
    vector.x /= scalar;
    vector.y /= scalar;
    return vector;
}

extern inline float vectorMagnitude(Vector vector){
    return sqrtf((vector.x * vector.x) + (vector.y * vector.y));
}

extern inline float vectorAngle(Vector vector){
    return toDegrees(atan2f(-vector.y, vector.x));
}

extern inline Vector vectorFromAToB(Point a, Point b){
    Vector toRet;
    toRet.x = b.x - a.x;
    toRet.y = b.y - a.y;
    return toRet;
}

extern inline void vectorToString(Vector vector, char* charPtr, int arraySize){
    snprintf(charPtr, arraySize, "V(%.3f, %.3f)", vector.x, vector.y);
}

#endif