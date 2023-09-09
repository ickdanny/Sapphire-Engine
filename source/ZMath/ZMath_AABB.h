#ifndef ZMATH_AABB_H
#define ZMATH_AABB_H

#include <string.h> /* for memcpy */
#include <math.h>
#include <stdbool.h>

#include "ZMath_Point.h"

typedef struct AABB{
    float xLow;
    float xHigh;
    float yLow;
    float yHigh;
} AABB;

extern inline AABB aabbCopy(const AABB toCopy){
    return toCopy;
}

extern inline void aabbCopyInto(AABB *destination, const AABB *source){
    memcpy(destination, source, sizeof(*source));
}

extern inline AABB aabbFromRadius(float radius){
    AABB toRet = {0};
    toRet.xLow = -radius;
    toRet.xHigh = radius;
    toRet.yLow = -radius;
    toRet.yHigh = radius;
    return toRet;
}

extern inline AABB aabbFromXY(float x, float y){
    AABB toRet = {0};
    toRet.xLow = -x;
    toRet.xHigh = x;
    toRet.yLow = -y;
    toRet.yHigh = y;
    return toRet;
}

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

extern inline float aabbWidth(const AABB *aabbPtr){
    return fabsf(aabbPtr->xHigh - aabbPtr->xLow);
}

extern inline float aabbHeight(const AABB *aabbPtr){
    return fabsf(aabbPtr->yHigh - aabbPtr->yLow);
}

extern inline float aabbArea(const AABB *aabbPtr){
    return aabbWidth(aabbPtr) * aabbHeight(aabbPtr);
}

extern inline Point aabbCenter(const AABB *aabbPtr){
    Point toRet = {0};
    toRet.x = (aabbPtr->xLow + aabbPtr->xHigh) / 2.0f;
    toRet.y = (aabbPtr->yLow + aabbPtr->yHigh) / 2.0f;
    return toRet;
}

extern inline void aabbSetX(AABB *aabbPtr, float x){
    aabbPtr->xLow = -x;
    aabbPtr->xHigh = x;
}

extern inline void aabbSetY(AABB *aabbPtr, float y){
    aabbPtr->yLow = -y;
    aabbPtr->yHigh = y;
}

extern inline AABB aabbCenterAt(const AABB *aabbPtr, Point center){
    AABB toRet = {0};
    toRet.xLow += center.x;
    toRet.xHigh += center.x;
    toRet.yLow += center.y;
    toRet.yHigh += center.y;
    return toRet;
}

extern inline bool aabbCollides(const AABB *aabb1Ptr, const AABB *aabb2Ptr){
    return aabb1Ptr->xLow <= aabb2Ptr->xHigh
        && aabb1Ptr->xHigh >= aabb2Ptr->xLow
        && aabb1Ptr->yLow <= aabb2Ptr->yHigh
        && aabb1Ptr->yHigh >= aabb2Ptr->yLow;
}

extern inline AABB aabbMakeEncompassing(const AABB *aabb1Ptr, const AABB *aabb2Ptr){
    AABB toRet = {0};
    toRet.xLow = fminf(aabb1Ptr->xLow, aabb2Ptr->xLow);
    toRet.xHigh = fmaxf(aabb1Ptr->xHigh, aabb2Ptr->xHigh);
    toRet.yLow = fminf(aabb1Ptr->yLow, aabb2Ptr->yLow);
    toRet.yHigh = fmaxf(aabb1Ptr->yHigh, aabb2Ptr->yHigh);
    return toRet;
}

#endif