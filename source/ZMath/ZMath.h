#ifndef ZMATH_H
#define ZMATH_H

#include "ZMath_AABB.h"
#include "ZMath_Angle.h"
#include "ZMath_Bitwise.h"
#include "ZMath_Constants.h"
#include "ZMath_Matrix4x4.h"
#include "ZMath_Numeric.h"
#include "ZMath_Point2D.h"
#include "ZMath_Polar.h"
#include "ZMath_Rectangle.h"
#include "ZMath_Vector2D.h"

/* 
 * Returns the result of adding the given Vector2D to 
 * the given AABB 
 */
AABB aabbAddVector(AABB aabb, Vector2D vector);

/* 
 * Returns the result of subtracting the given Vector2D 
 * from the given AABB 
 */
AABB aabbSubtractVector(AABB aabb, Vector2D vector);

/* 
 * Returns true if the given Point2D is within the 
 * given AABB, false otherwise 
 */
bool isPointWithinAABB(
    Point2D point, 
    AABB *const aabbPtr
);

#endif