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

/* Returns the sum of the two given Polars */
Polar polarAdd(Polar left, Polar right){
    Vector2D leftVec = polarToVector(left);
    Vector2D rightVec = polarToVector(right);
    Vector2D sumVec = vector2DAdd(leftVec, rightVec);
    return polarFromVector(sumVec);
}

/*
 * Returns the difference between the two given Polars
 */
Polar polarSubtract(Polar left, Polar right){
    Vector2D leftVec = polarToVector(left);
    Vector2D rightVec = polarToVector(right);
    Vector2D diffVec = vector2DSubtract(
        leftVec,
        rightVec
    );
    return polarFromVector(diffVec);
}

/* Returns the scalar multiple of the given polar */
Polar polarMultiply(Polar polar, float scalar){
    polar.magnitude *= scalar;
    return polar;
}

/*
 * Returns the scalar quotient of the given polar,
 * does not check for division by zero
 */
Polar polarDivide(Polar polar, float scalar){
    polar.magnitude /= scalar;
    return polar;
}

/* 
 * Returns the result of adding the given Polar
 * to the specified Point2D
 */
Point2D point2DAddPolar(Point2D point, Polar polar){
    return point2DAddVector2D(
        point,
        polarToVector(polar)
    );
}

/* 
 * Returns the result of subtracting the given Polar
 * from the specified Point2D
 */
Point2D point2DSubtractPolar(
    Point2D point,
    Polar polar
){
    return point2DSubtractVector2D(
        point,
        polarToVector(polar)
    );
}

/* 
 * Constructs the Polar which transforms 
 * Point2D A to B 
 */
Polar polarFromAToB(Point2D a, Point2D b){
    return polarFromVector(
        vector2DFromAToB(a, b)
    );
}

/* 
 * Returns the Cartesian representation of the 
 * given Polar 
 */
Vector2D polarToVector(Polar polar){
    Vector2D toRet = {0};
    float radians = toRadians(polar.angle);
    toRet.x = polar.magnitude * cosf(radians);
    toRet.y = -polar.magnitude * sinf(radians);
    return toRet;
}

/* Prints the given Polar to the given C String */
void printPolar(
    Polar polar,
    char* charPtr, 
    int arraySize
){
    snprintf(
        charPtr, 
        arraySize, 
        "<%.3f, %.3f>", 
        polar.magnitude,
        polar.angle
    );
}