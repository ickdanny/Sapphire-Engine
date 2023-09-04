#ifndef ZMATH_H
#define ZMATH_H

#include "ZMath_AABB.h"
#include "ZMath_Angle.h"
#include "ZMath_Constants.h"
#include "ZMath_Point.h"
#include "ZMath_Polar.h"
#include "ZMath_Rectangle.h"
#include "ZMath_Vector.h"

inline AABB aabbAddVector(AABB aabb, Vector vector){
    aabb.xLow += vector.x;
    aabb.xHigh += vector.x;
    aabb.yLow += vector.y;
    aabb.yHigh += vector.y;
    return aabb;
}

inline AABB aabbSubtractVector(AABB aabb, Vector vector){
    aabb.xLow -= vector.x;
    aabb.xHigh -= vector.x;
    aabb.yLow -= vector.y;
    aabb.yHigh -= vector.y;
    return aabb;
}

inline bool isPointWithinAABB(Point point, AABB *const aabbPtr){
    return point.x > aabbPtr->xLow
        && point.x < aabbPtr->xHigh
        && point.y > aabbPtr->yLow
        && point.y < aabbPtr->yHigh;
}

inline int ceilingIntegerDivide(int x, int y){
    return x / y + (x % y != 0);
}

#endif