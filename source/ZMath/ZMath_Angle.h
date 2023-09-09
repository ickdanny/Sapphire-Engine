#ifndef ZMATH_ANGLE_H
#define ZMATH_ANGLE_H

#include "ZMath_Constants.h"

const float z_minAngle = 0.0f;
const float z_halfAngle = 180.0f;
const float z_maxAngle = 360.0f;

extern inline float toDegrees(float radians){
    return radians * 180.0f / z_pi;
}

extern inline float toRadians(float degrees){
    return degrees * z_pi / 180.0f;
}

extern inline float angleMod(float angle){
    if(angle < z_minAngle){
        angle += z_maxAngle;
    }
    else if(angle >= z_maxAngle){
        angle -= z_maxAngle;
    }
    return angle;
}

extern inline float angleSmallerDifference(float from, float to){
    float difference = from - to;
    if(difference > z_halfAngle){
        return -(z_maxAngle - difference);
    }
    if(difference < -z_halfAngle){
        return z_maxAngle + difference;
    }
    return difference;
}

extern inline float angleLargerDifference(float from, float to){
    float difference = from - to;
    if (difference < z_halfAngle && difference >= z_minAngle) {
		return -(z_maxAngle - difference);
	}
	if (difference > -z_halfAngle && difference <= z_minAngle) {
		return z_maxAngle + difference;
	}
	return difference;
}

extern inline float angleFlipY(float angle){
    return angleMod(z_halfAngle - angle);
}

extern inline float angleFlipX(float angle){
    return angleMod(-angle);
}

extern inline float angleNegate(float angle){
    return angleMod(angle + z_halfAngle);
}

extern inline float angleAdd(float angle1, float angle2){
    return angleMod(angle1 + angle2);
}

extern inline float angleSubtract(float angle1, float angle2){
    return angleMod(angle1 - angle2);
}

#endif