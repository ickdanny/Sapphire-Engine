#ifndef TRIFECTA_TIME_H
#define TRIFECTA_TIME_H

#include <time.h>
#include <stdint.h>

#ifdef __APPLE__

typedef struct timespec TimePoint;

#endif /* end __APPLE__ */

/* Returns the current time */
TimePoint getCurrentTime();

/* 
 * Returns the result of adding the specified amount
 * of nanoseconds to the specified TimePoint
 */
TimePoint addTimeNano(
    TimePoint timePoint,
    uint64_t timeNano
);

/* 
 * Returns a negative value if the first TimePoint
 * is before the second, a positive value if the
 * first TimePoint is after the second, or 0 if
 * the two TimePoints are the same
 */
int timePointCompare(TimePoint left, TimePoint right){
    if(left.tv_sec < right.tv_sec){
        return -1;
    }
    if(left.tv_sec > right.tv_sec){
        return 1;
    }
    if(left.tv_nsec < right.tv_nsec){
        return -1;
    }
    if(left.tv_nsec > right.tv_nsec){
        return 1;
    }
    return 0;
}

/*
 * Returns the difference between the two specified
 * TimePoints in nanoseconds
 */
uint64_t timePointDiffNano(
    TimePoint left,
    TimePoint right
);

/* Sleeps until the specified TimePoint */
void sleepUntil(TimePoint timePoint);

#endif