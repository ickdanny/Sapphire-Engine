#ifndef ZMATH_POINT_H
#define ZMATH_POINT_H

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "ZMath_Angle.h"
#include "ZMath_Numeric.h"

/* A 2D point stored as floats */
typedef struct Point{
    float x;
    float y;
} Point;

/* Makes a value copy of the given Point */
extern inline Point pointCopy(const Point toCopy){
    return toCopy;
}

/* Copies the Point source into destination */
extern inline void pointCopyInto(Point *destination, const Point *source){
    destination->x = source->x;
    destination->y = source->y;
}

/* Returns the Pythagorean distance between two Points */
extern inline float pointDistance(Point a, Point b){
    return sqrtf(powf(b.x - a.x, 2) + powf(b.y - a.y, 2));
}

/* Returns the degree angle from Point A to B */
extern inline float pointAngle(Point a, Point b){
    return toDegrees(atan2f(-(b.y - a.y), b.x - a.x));
}

/* Prints the given Point to the given C String */
extern inline void printPoint(Point point, char *str, int arraySize){
    snprintf(str, arraySize, "P(%.3f, %.3f)", point.x, point.y);
}

#endif