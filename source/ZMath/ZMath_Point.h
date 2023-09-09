#ifndef ZMATH_POINT_H
#define ZMATH_POINT_H

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "ZMath_Angle.h"
#include "ZMath_Numeric.h"

typedef struct Point{
    float x;
    float y;
} Point;

extern inline Point pointCopy(const Point toCopy){
    return toCopy;
}

extern inline void pointCopyInto(Point *destination, const Point source){
    destination->x = source.x;
    destination->y = source.y;
}

extern inline float pointDistance(Point a, Point b){
    return sqrtf(powf(b.x - a.x, 2) + powf(b.y - a.y, 2));
}

extern inline float pointAngle(Point a, Point b){
    return toDegrees(atan2f(-(b.y - a.y), b.x - a.x));
}

extern inline void pointToString(Point point, char* charPtr, int arraySize){
    snprintf(charPtr, arraySize, "P(%.3f, %.3f)", point.x, point.y);
}

#endif