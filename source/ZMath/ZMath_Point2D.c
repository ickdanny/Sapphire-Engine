#include "ZMath_Point2D.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

/* Makes a value copy of the given Point2D */
Point2D point2DCopy(const Point2D toCopy){
    return toCopy;
}

/* Copies the Point2D source into destination */
void point2DCopyInto(
    Point2D *destination, 
    const Point2D *source
){
    destination->x = source->x;
    destination->y = source->y;
}

/* Returns the distance between two Points */
float point2DDistance(Point2D a, Point2D b){
    return sqrtf(powf(b.x - a.x, 2) 
        + powf(b.y - a.y, 2));
}

/* Returns the degree angle from Point2D A to B */
float point2DAngle(Point2D a, Point2D b){
    return toDegrees(atan2f(-(b.y - a.y), b.x - a.x));
}

/* Prints the given Point2D to the given C String */
void printPoint2D(
    Point2D point, 
    char *str, 
    int arraySize
){
    snprintf(
        str, 
        arraySize, 
        "P(%.3f, %.3f)", 
        point.x, 
        point.y
    );
}