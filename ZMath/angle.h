#ifndef ZMATH_ANGLE_H
#define ZMATH_ANGLE_H

#include "constants.h"

const float minAngle = 0.0f;
const float halfAngle = 180.0f;
const float maxAngle = 360.0f;

inline float toDegrees(float radians){
    return radians * 180.0f / z_pi;
}

inline float toRadians(float degrees){
    return degrees * z_pi / 180.0f;
}

inline float angleMod(float angle){
    if(angle < minAngle){
        angle += maxAngle;
    }
    else if(angle >= maxAngle){
        angle -= maxAngle;
    }
    return angle;
}

inline float angleSmallerDifference(float from, float to){
    float difference = from - to;
    if(difference > halfAngle){
        return -(maxAngle - difference);
    }
    if(difference < -halfAngle){
        return maxAngle + difference;
    }
    return difference;
}

inline float angleLargerDifference(float from, float to){
    float difference = from - to;
    if (difference < halfAngle && difference >= minAngle) {
		return -(maxAngle - difference);
	}
	if (difference > -halfAngle && difference <= minAngle) {
		return maxAngle + difference;
	}
	return difference;
}

inline float angleFlipY(float angle){
    return angleMod(halfAngle - angle);
}

inline float angleFlipX(float angle){
    return angleMod(-angle);
}

inline float angleNegate(float angle){
    return angleMod(angle + halfAngle);
}

inline float angleAdd(float angle1, float angle2){
    return angleMod(angle1 + angle2);
}

inline float angleSubtract(float angle1, float angle2){
    return angleMod(angle1 - angle2);
}

#endif