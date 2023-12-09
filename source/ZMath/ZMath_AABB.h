#ifndef ZMATH_AABB_H
#define ZMATH_AABB_H

#include <string.h> /* for memcpy */
#include <math.h>
#include <stdbool.h>

#include "ZMath_Point.h"

/* A 2D AABB stored as bounds, not width/height */
typedef struct AABB{
    float xLow;
    float xHigh;
    float yLow;
    float yHigh;
} AABB;

/* Makes a value copy of the given AABB */
extern inline AABB aabbCopy(const AABB toCopy){
    return toCopy;
}

/* Copies the AABB source into destination */
extern inline void aabbCopyInto(AABB *destination, const AABB *source){
    memcpy(destination, source, sizeof(*source));
}

/* Constructs an AABB of the given radius */
extern inline AABB aabbFromRadius(float radius){
    AABB toRet = {0};
    toRet.xLow = -radius;
    toRet.xHigh = radius;
    toRet.yLow = -radius;
    toRet.yHigh = radius;
    return toRet;
}

/* Constructs a symmetrical AABB from given x and y */
extern inline AABB aabbFromXY(float x, float y){
    AABB toRet = {0};
    toRet.xLow = -x;
    toRet.xHigh = x;
    toRet.yLow = -y;
    toRet.yHigh = y;
    return toRet;
}

/* Constructs an AABB from the given dimensions */
extern inline AABB aabbFromDimensions(
    float xLow, 
    float xHigh, 
    float yLow, 
    float yHigh
){
    AABB toRet = {0};
    toRet.xLow = xLow;
    toRet.xHigh = xHigh;
    toRet.yLow = yLow;
    toRet.yHigh = yHigh;
    return toRet;
}

/* Returns the width of the given AABB (x) */
extern inline float aabbWidth(const AABB *aabbPtr){
    return fabsf(aabbPtr->xHigh - aabbPtr->xLow);
}

/* Returns the height of the given AABB (y) */
extern inline float aabbHeight(const AABB *aabbPtr){
    return fabsf(aabbPtr->yHigh - aabbPtr->yLow);
}

/* Returns the area of the given AABB */
extern inline float aabbArea(const AABB *aabbPtr){
    return aabbWidth(aabbPtr) * aabbHeight(aabbPtr);
}

/* Returns the geometric center of the given AABB */
extern inline Point aabbCenter(const AABB *aabbPtr){
    Point toRet = {0};
    toRet.x = (aabbPtr->xLow + aabbPtr->xHigh) / 2.0f;
    toRet.y = (aabbPtr->yLow + aabbPtr->yHigh) / 2.0f;
    return toRet;
}

/* Sets the x radius of the given AABB */
extern inline void aabbSetX(AABB *aabbPtr, float x){
    aabbPtr->xLow = -x;
    aabbPtr->xHigh = x;
}

/* Sets the y radius of the given AABB */
extern inline void aabbSetY(AABB *aabbPtr, float y){
    aabbPtr->yLow = -y;
    aabbPtr->yHigh = y;
}

/* Returns a new AABB from adding the given Point to the given AABB */
extern inline AABB aabbCenterAt(const AABB *aabbPtr, Point center){
    AABB toRet = {0};
    aabbCopyInto(&toRet, aabbPtr);
    toRet.xLow += center.x;
    toRet.xHigh += center.x;
    toRet.yLow += center.y;
    toRet.yHigh += center.y;
    return toRet;
}

/* Returns true if the given AABBs intersect, false otherwise */
extern inline bool aabbCollides(const AABB *aabb1Ptr, const AABB *aabb2Ptr){
    return aabb1Ptr->xLow <= aabb2Ptr->xHigh
        && aabb1Ptr->xHigh >= aabb2Ptr->xLow
        && aabb1Ptr->yLow <= aabb2Ptr->yHigh
        && aabb1Ptr->yHigh >= aabb2Ptr->yLow;
}

/* Returns a new AABB which is the smallest AABB containing both given AABBs */
extern inline AABB aabbMakeEncompassing(const AABB *aabb1Ptr, const AABB *aabb2Ptr){
    AABB toRet = {0};
    toRet.xLow = fminf(aabb1Ptr->xLow, aabb2Ptr->xLow);
    toRet.xHigh = fmaxf(aabb1Ptr->xHigh, aabb2Ptr->xHigh);
    toRet.yLow = fminf(aabb1Ptr->yLow, aabb2Ptr->yLow);
    toRet.yHigh = fmaxf(aabb1Ptr->yHigh, aabb2Ptr->yHigh);
    return toRet;
}

#endif