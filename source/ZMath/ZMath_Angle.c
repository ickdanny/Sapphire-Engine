#include "ZMath_Angle.h"

#include <math.h>

/* Converts radians to degrees */
float toDegrees(float radians){
    return radians * 180.0f / z_pi;
}

/* Converts degrees to radians */
float toRadians(float degrees){
    return degrees * z_pi / 180.0f;
}

/* Normalizes angles into the range [0.0, 360.0) */
float angleMod(float angle){
    /* this implementation assumes minAngle = 0 */
    if(angle < z_minAngle){
        angle = fmod(angle, z_maxAngle) + z_maxAngle;
    }
    else if(angle >= z_maxAngle){
        angle = fmod(angle, z_maxAngle);
    }
    return angle;
}

/* 
 * Returns the smaller angle between the two 
 * given angles 
 */
float angleSmallerDifference(
    float from, 
    float to
){
    float difference = from - to;
    if(difference > z_halfAngle){
        return -(z_maxAngle - difference);
    }
    if(difference < -z_halfAngle){
        return z_maxAngle + difference;
    }
    return difference;
}

/* 
 * Returns the larger angle between the two 
 * given angles 
 */
float angleLargerDifference(
    float from, 
    float to
){
    float difference = from - to;
    if (difference < z_halfAngle 
        && difference >= z_minAngle
    ){
		return -(z_maxAngle - difference);
	}
	if (difference > -z_halfAngle 
        && difference <= z_minAngle
    ){
		return z_maxAngle + difference;
	}
	return difference;
}

/* Flips the given angle across the y axis */
float angleFlipY(float angle){
    return angleMod(z_halfAngle - angle);
}

/* Flips the given angle across the x axis */
float angleFlipX(float angle){
    return angleMod(-angle);
}

/* Flips the given angle halfway around */
float angleNegate(float angle){
    return angleMod(angle + z_halfAngle);
}

/* Adds the two given angles together */
float angleAdd(float angle1, float angle2){
    return angleMod(angle1 + angle2);
}

/* Subtracts the second angle from the first */
float angleSubtract(float angle1, float angle2){
    return angleMod(angle1 - angle2);
}