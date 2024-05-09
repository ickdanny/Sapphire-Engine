#ifndef ZMATH_POINT_H
#define ZMATH_POINT_H

#include "ZMath_Angle.h"
#include "ZMath_Numeric.h"

/* A 2D point stored as floats */
typedef struct Point{
    float x;
    float y;
} Point;

/* Makes a value copy of the given Point */
Point pointCopy(const Point toCopy);


/* Copies the Point source into destination */
void pointCopyInto(
    Point *destination, 
    const Point *source
);

/* Returns the distance between two Points */
float pointDistance(Point a, Point b);

/* Returns the degree angle from Point A to B */
float pointAngle(Point a, Point b);

/* Prints the given Point to the given C String */
void printPoint(Point point, char *str, int arraySize);

#endif