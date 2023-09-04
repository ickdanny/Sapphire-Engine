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

inline AABB aabbCopy(AABB toCopy){
    return toCopy;
}

inline void aabbCopyInto(AABB *destination, AABB* source){
    memcpy(destination, source, sizeof(AABB));
}

inline AABB aabbFromRadius(float radius){
    AABB toRet;
    toRet.xLow = -radius;
    toRet.xHigh = radius;
    toRet.yLow = -radius;
    toRet.yHigh = radius;
    return toRet;
}

inline AABB aabbFromXY(float x, float y){
    AABB toRet;
    toRet.xLow = -x;
    toRet.xHigh = x;
    toRet.yLow = -y;
    toRet.yHigh = y;
    return toRet;
}

inline AABB aabbFromDimensions(
    float xLow, 
    float xHigh, 
    float yLow, 
    float yHigh
){
    AABB toRet;
    toRet.xLow = xLow;
    toRet.xHigh = xHigh;
    toRet.yLow = yLow;
    toRet.yHigh = yHigh;
    return toRet;
}

inline float aabbWidth(AABB *const aabbPtr){
    return fabsf(aabbPtr->xHigh - aabbPtr->xLow);
}

inline float aabbHeight(AABB *const aabbPtr){
    return fabsf(aabbPtr->yHigh - aabbPtr->yLow);
}

inline float aabbArea(AABB *const aabbPtr){
    return aabbWidth(aabbPtr) * aabbHeight(aabbPtr);
}

inline Point aabbCenter(AABB *const aabbPtr){
    Point toRet;
    toRet.x = (aabbPtr->xLow + aabbPtr->xHigh) / 2.0f;
    toRet.y = (aabbPtr->yLow + aabbPtr->yHigh) / 2.0f;
    return toRet;
}

inline void aabbSetX(AABB *aabbPtr, float x){
    aabbPtr->xLow = -x;
    aabbPtr->xHigh = x;
}

inline void aabbSetY(AABB *aabbPtr, float y){
    aabbPtr->yLow = -y;
    aabbPtr->yHigh = y;
}

inline AABB aabbCenterAt(AABB aabb, Point center){
    aabb.xLow += center.x;
    aabb.xHigh += center.x;
    aabb.yLow += center.y;
    aabb.yHigh += center.y;
    return aabb;
}

inline bool aabbCollides(AABB *const aabb1Ptr, AABB *const aabb2Ptr){
    return aabb1Ptr->xLow <= aabb2Ptr->xHigh
        && aabb1Ptr->xHigh >= aabb2Ptr->xLow
        && aabb1Ptr->yLow <= aabb2Ptr->yHigh
        && aabb1Ptr->yHigh >= aabb2Ptr->yLow;
}

inline AABB aabbMakeEncompassing(AABB *const aabb1Ptr, AABB *const aabb2Ptr){
    AABB toRet;
    toRet.xLow = fminf(aabb1Ptr->xLow, aabb2Ptr->xLow);
    toRet.xHigh = fmaxf(aabb1Ptr->xHigh, aabb2Ptr->xHigh);
    toRet.yLow = fminf(aabb1Ptr->yLow, aabb2Ptr->yLow);
    toRet.yHigh = fmaxf(aabb1Ptr->yHigh, aabb2Ptr->yHigh);
    return toRet;
}

#endif