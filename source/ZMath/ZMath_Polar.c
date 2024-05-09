#include "ZMath_Polar.h"

#include <stdio.h>
#include <string.h> /* for memcpy */

/* Makes a value copy of the given Polar */
Polar polarCopy(Polar toCopy){
    return toCopy;
}

/* Copies the Polar source into destination */
void polarCopyInto(
    Polar *destination, 
    const Polar *source
){
    memcpy(destination, source, sizeof(*source));
}

/* 
 * Updates the internal Cartesian representation of 
 * the given Polar 
 */
void _polarUpdateVector(Polar *polarPtr){
    float radians = toRadians(polarPtr->angle);
    float magnitude = polarPtr->magnitude;
    polarPtr->asVector.x = magnitude * cosf(radians);
    polarPtr->asVector.y = -magnitude * sinf(radians);
}

/* 
 * Constructs a new Polar from a given 
 * Cartesian Vector 
 */
Polar polarFromVector(Vector vector){
    Polar toRet = {0};
    toRet.magnitude = vectorMagnitude(vector);
    toRet.angle = vectorAngle(vector);
    toRet.asVector = vector;
    return toRet;
}

/* Sets the magnitude of the given Polar */
void polarSetMagnitude(
    Polar *polarPtr, 
    float magnitude
){
    polarPtr->magnitude = magnitude;
    _polarUpdateVector(polarPtr);
}

/* Sets the angle of the given Polar in degrees*/
void polarSetAngle(Polar *polarPtr, float angle){
    polarPtr->angle = angle;
    _polarUpdateVector(polarPtr);
}

/* Flips the given Polar halfway around */
Polar polarNegate(Polar polar){
    polarSetAngle(
        &polar, 
        angleAdd(polar.angle, z_halfAngle)
    );
    return polar;
}

/* 
 * Returns the Cartesian representation of the 
 * given Polar 
 */
Vector polarToVector(Polar *polarPtr){
    return polarPtr->asVector;
}

/* Prints the given Polar to the given C String */
void printPolar(
    Polar *polarPtr, 
    char* charPtr, 
    int arraySize
){
    snprintf(
        charPtr, 
        arraySize, 
        "R(%.3f, %.3f)", 
        polarPtr->magnitude,
        polarPtr->angle
    );
}