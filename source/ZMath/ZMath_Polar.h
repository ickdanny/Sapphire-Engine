#ifndef ZMATH_POLAR_H
#define ZMATH_POLAR_H

#include <string.h> /* for memcpy */

#include "ZMath_Vector.h"

typedef struct Polar{
    float magnitude;
    float angle;
    
    Vector asVector;
} Polar;

extern inline Polar polarCopy(Polar toCopy){
    return toCopy;
}

extern inline void polarCopyInto(Polar *destination, const Polar *source){
    memcpy(destination, source, sizeof(*source));
}

extern inline void _polarUpdateVector(Polar *polarPtr){
    float radians = toRadians(polarPtr->angle);
    float magnitude = polarPtr->magnitude;
    polarPtr->asVector.x = magnitude * cosf(radians);
    polarPtr->asVector.y = -magnitude * sinf(radians);
}

extern inline Polar polarFromVector(Vector vector){
    Polar toRet = {0};
    toRet.magnitude = vectorMagnitude(vector);
    toRet.angle = vectorAngle(vector);
    toRet.asVector = vector;
    return toRet;
}

extern inline void polarSetMagnitude(Polar *polarPtr, float magnitude){
    polarPtr->magnitude = magnitude;
    _polarUpdateVector(polarPtr);
}

extern inline void polarSetAngle(Polar *polarPtr, float angle){
    polarPtr->angle = angle;
    _polarUpdateVector(polarPtr);
}

extern inline Polar polarNegate(Polar polar){
    polarSetAngle(&polar, angleAdd(polar.angle, z_halfAngle));
    return polar;
}

extern inline Vector polarToVector(Polar *polarPtr){
    return polarPtr->asVector;
}

extern inline void polarToString(Polar *polarPtr, char* charPtr, int arraySize){
    snprintf(
        charPtr, 
        arraySize, 
        "R(%.3f, %.3f)", 
        polarPtr->magnitude,
        polarPtr->angle
    );
}

#endif