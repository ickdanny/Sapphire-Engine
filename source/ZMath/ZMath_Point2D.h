#ifndef ZMATH_POINT_H
#define ZMATH_POINT_H

#include "ZMath_Angle.h"
#include "ZMath_Numeric.h"

/* A 2D point stored as floats */
typedef struct Point2D{
    float x;
    float y;
} Point2D;

/* Makes a value copy of the given Point2D */
Point2D point2DCopy(const Point2D toCopy);


/* Copies the Point2D source into destination */
void point2DCopyInto(
    Point2D *destination, 
    const Point2D *source
);

/* Returns the distance between two Points */
float point2DDistance(Point2D a, Point2D b);

/* Returns the degree angle from Point2D A to B */
float point2DAngle(Point2D a, Point2D b);

/* Prints the given Point2D to the given C String */
void printPoint2D(
    Point2D point,
    char *str,
    int arraySize
);

#endif