#ifndef ZMATH_H
#define ZMATH_H

#include "ZMath_AABB.h"
#include "ZMath_Angle.h"
#include "ZMath_Bitwise.h"
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
AABB aabbAddVector(AABB aabb, Vector vector);

/* 
 * Returns the result of subtracting the given Vector 
 * from the given AABB 
 */
AABB aabbSubtractVector(AABB aabb, Vector vector);

/* 
 * Returns true if the given Point is within the 
 * given AABB, false otherwise 
 */
bool isPointWithinAABB(
    Point point, 
    AABB *const aabbPtr
);

#endif