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
 * Constructs a new Polar from a given 
 * Cartesian Vector2D 
 */
Polar polarFromVector(Vector2D vector){
    Polar toRet = {0};
    toRet.magnitude = vector2DMagnitude(vector);
    toRet.angle = vector2DAngle(vector);
    return toRet;
}

/* Sets the magnitude of the given Polar */
void polarSetMagnitude(
    Polar *polarPtr, 
    float magnitude
){
    polarPtr->magnitude = magnitude;
}

/* Sets the angle of the given Polar in degrees*/
void polarSetAngle(Polar *polarPtr, float angle){
    polarPtr->angle = angle;
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
Vector2D polarToVector(Polar *polarPtr){
    Vector2D toRet = {0};
    float radians = toRadians(polarPtr->angle);
    toRet.x = polarPtr->magnitude * cosf(radians);
    toRet.y = -polarPtr->magnitude * sinf(radians);
    return toRet;
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