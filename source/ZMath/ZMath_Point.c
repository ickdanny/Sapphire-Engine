#include "ZMath_Point.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

/* Makes a value copy of the given Point */
Point pointCopy(const Point toCopy){
    return toCopy;
}

/* Copies the Point source into destination */
void pointCopyInto(
    Point *destination, 
    const Point *source
){
    destination->x = source->x;
    destination->y = source->y;
}

/* Returns the distance between two Points */
float pointDistance(Point a, Point b){
    return sqrtf(powf(b.x - a.x, 2) 
        + powf(b.y - a.y, 2));
}

/* Returns the degree angle from Point A to B */
float pointAngle(Point a, Point b){
    return toDegrees(atan2f(-(b.y - a.y), b.x - a.x));
}

/* Prints the given Point to the given C String */
void printPoint(
    Point point, 
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