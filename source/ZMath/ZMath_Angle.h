#ifndef ZMATH_ANGLE_H
#define ZMATH_ANGLE_H

#include <math.h>

#include "ZMath_Constants.h"

/* The min value of angle representations inclusive */
#define z_minAngle 0.0f

/* The max value of angle representations exclusive */
#define z_maxAngle 360.0f

/* The range of possible values of angles */
#define z_angleRange ((z_maxAngle) - (z_minAngle))

/* The value of the angle halfway from min and max */
#define z_halfAngle ((z_minAngle) + ((z_angleRange) \
    / 2.0f))

/* Converts radians to degrees */
float toDegrees(float radians);

/* Converts degrees to radians */
float toRadians(float degrees);

/* Normalizes angles into the range [0.0, 360.0) */
float angleMod(float angle);

/* 
 * Returns the smaller angle between the two 
 * given angles 
 */
float angleSmallerDifference(float from, float to);

/* 
 * Returns the larger angle between the two 
 * given angles 
 */
float angleLargerDifference(float from, float to);

/* Flips the given angle across the y axis */
float angleFlipY(float angle);

/* Flips the given angle across the x axis */
float angleFlipX(float angle);

/* Flips the given angle halfway around */
float angleNegate(float angle);

/* Adds the two given angles together */
float angleAdd(float angle1, float angle2);

/* Subtracts the second angle from the first */
float angleSubtract(float angle1, float angle2);

#endif