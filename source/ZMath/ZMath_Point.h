#ifndef ZMATH_POINT_H
#define ZMATH_POINT_H

#include "ZMath_Angle.h"

typedef struct Point{
    float x;
    float y;
} Point;

inline Point pointCopy(Point toCopy){
    return toCopy;
}

inline void pointCopyInto(Point *destination, Point source){
    destination->x = source.x;
    destination->y = source.y;
}

inline float pointDistance(Point a, Point b){
    return sqrtf(powf(b.x - a.x, 2) + powf(b.y - a.y, 2));
}

inline float pointAngle(Point a, Point b){
    return toDegrees(atan2f(-(b.y - a.y), b.x - a.x));
}

#endif