#ifndef ZMATH_POINT_H
#define ZMATH_POINT_H

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

#endif