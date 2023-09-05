#ifndef ZMATH_POLAR_H
#define ZMATH_POLAR_H

#include "ZMath_Vector.h"

typedef struct Polar{
    float magnitude;
    float angle;
    
    Vector asVector;
} Polar;

inline void _polarUpdateVector(Polar *polarPtr){
    float radians = toRadians(polarPtr->angle);
    float magnitude = polarPtr->magnitude;
    polarPtr->asVector.x = magnitude * cosf(radians);
    polarPtr->asVector.y = -magnitude * sinf(radians);
}

inline Polar polarFromVector(Vector vector){
    Polar toRet;
    toRet.magnitude = vectorMagnitude(vector);
    toRet.angle = vectorAngle(vector);
    toRet.asVector = vector;
}

inline void polarSetMagnitude(Polar *polarPtr, float magnitude){
    polarPtr->magnitude = magnitude;
    _polarUpdateVector(polarPtr);
}

inline void polarSetAngle(Polar *polarPtr, float angle){
    polarPtr->angle = angle;
    _polarUpdateVector(polarPtr);
}

inline Polar polarNegate(Polar polar){
    polarSetAngle(&polar, angleAdd(polar.angle, halfAngle));
    return polar;
}

inline Vector polarToVector(Polar *polarPtr){
    return polarPtr->asVector;
}

#endif