#ifndef ZMATH_AABB_H
#define ZMATH_AABB_H

#include <stdbool.h>

#include "ZMath_Point2D.h"

/* A 2D AABB stored as bounds, not width/height */
typedef struct AABB{
    float xLow;
    float xHigh;
    float yLow;
    float yHigh;
} AABB;

/* Makes a value copy of the given AABB */
AABB aabbCopy(const AABB toCopy);

/* Copies the AABB source into destination */
void aabbCopyInto(
    AABB *destination, 
    const AABB *source
);

/* Constructs an AABB of the given radius */
AABB aabbMakeRadius(float radius);

/* Constructs a symmetrical AABB from given x and y */
AABB aabbMakeXY(float x, float y);

/* Constructs an AABB from the given dimensions */
AABB aabbMakeDimensions(
    float xLow, 
    float xHigh, 
    float yLow, 
    float yHigh
);

/* Returns the width of the given AABB (x) */
float aabbWidth(const AABB *aabbPtr);

/* Returns the height of the given AABB (y) */
float aabbHeight(const AABB *aabbPtr);

/* Returns the area of the given AABB */
float aabbArea(const AABB *aabbPtr);

/* Returns the geometric center of the given AABB */
Point2D aabbCenter(const AABB *aabbPtr);

/* Sets the x radius of the given AABB */
void aabbSetX(AABB *aabbPtr, float x);

/* Sets the y radius of the given AABB */
void aabbSetY(AABB *aabbPtr, float y);

/* 
 * Returns a new AABB from adding the given Point2D to 
 * the given AABB 
 */
AABB aabbCenterAt(const AABB *aabbPtr, Point2D center);

/* 
 * Returns true if the given AABBs intersect, 
 * false otherwise 
 */
bool aabbCollides(
    const AABB *aabb1Ptr, 
    const AABB *aabb2Ptr
);

/* 
 * Returns a new AABB which is the smallest AABB 
 * containing both given AABBs 
 */
AABB aabbMakeEncompassing(
    const AABB *aabb1Ptr, 
    const AABB *aabb2Ptr
);

#endif