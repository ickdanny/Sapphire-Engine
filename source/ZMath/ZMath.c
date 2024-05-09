#include "ZMath.h"

/* 
 * Returns the result of adding the given Vector to 
 * the given AABB 
 */
AABB aabbAddVector(AABB aabb, Vector vector){
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
AABB aabbSubtractVector(AABB aabb, Vector vector){
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
bool isPointWithinAABB(
    Point point, 
    AABB *const aabbPtr
){
    return point.x > aabbPtr->xLow
        && point.x < aabbPtr->xHigh
        && point.y > aabbPtr->yLow
        && point.y < aabbPtr->yHigh;
}