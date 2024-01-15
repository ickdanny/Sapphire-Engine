#ifndef ZMATH_H
#define ZMATH_H

#include "ZMath_AABB.h"
#include "ZMath_Angle.h"
#include "ZMath_Constants.h"
#include "ZMath_Numeric.h"
#include "ZMath_Point.h"
#include "ZMath_Polar.h"
#include "ZMath_Rectangle.h"
#include "ZMath_Vector.h"

/* 
 * Returns the result of adding the given Vector to 
 * the given AABB 
 */
extern inline AABB aabbAddVector(
    AABB aabb, 
    Vector vector
){
    aabb.xLow += vector.x;
    aabb.xHigh += vector.x;
    aabb.yLow += vector.y;
    aabb.yHigh += vector.y;
    return aabb;
}

/* 
 * Returns the result of subtracting the given Vector 
 * from the given AABB 
 */
extern inline AABB aabbSubtractVector(
    AABB aabb, 
    Vector vector
){
    aabb.xLow -= vector.x;
    aabb.xHigh -= vector.x;
    aabb.yLow -= vector.y;
    aabb.yHigh -= vector.y;
    return aabb;
}

/* 
 * Returns true if the given Point is within the 
 * given AABB, false otherwise 
 */
extern inline bool isPointWithinAABB(
    Point point, 
    AABB *const aabbPtr
){
    return point.x > aabbPtr->xLow
        && point.x < aabbPtr->xHigh
        && point.y > aabbPtr->yLow
        && point.y < aabbPtr->yHigh;
}

#endif