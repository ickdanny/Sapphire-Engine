#ifndef ZMATH_VECTOR_H
#define ZMATH_VECTOR_H

#include <math.h>
#include "Point.h"

typedef struct Vector{
    float x;
    float y;
} Vector;

inline Vector vectorCopy(Vector toCopy){
    return toCopy;
}

inline void vectorCopyInto(Vector *destination, Vector source){
    destination->x = source.x;
    destination->y = source.y;
}

inline Vector vectorNegate(Vector vector){
    vector.x = -vector.x;
    vector.y = -vector.y;
    return vector;
}

inline Point vectorToPoint(Vector vector){
    Point toRet;
    toRet.x = vector.x;
    toRet.y = vector.y;
    return toRet;
}

inline Vector vectorAdd(Vector vector1, Vector vector2){
    vector1.x += vector2.x;
    vector1.y += vector2.y;
    return vector1;
}

inline Vector vectorSubtract(Vector vector1, Vector vector2){
    vector1.x -= vector2.x;
    vector1.y -= vector2.y;
    return vector1;
}

inline Vector vectorMultiply(Vector vector, float scalar){
    vector.x *= scalar;
    vector.y *= scalar;
    return vector;
}

inline Vector vectorDivide(Vector vector, float scalar){
    vector.x /= scalar;
    vector.y /= scalar;
    return vector;
}

inline float vectorMagnitude(Vector vector){
    return sqrtf((vector.x * vector.x) + (vector.y * vector.y));
}

inline float vectorAngle(Vector vector){
    //todo
}

inline Vector vectorFromAToB(Point a, Point b){
    Vector toRet;
    toRet.x = b.x - a.x;
    toRet.y = b.y - a.y;
    return toRet;
}

#endif