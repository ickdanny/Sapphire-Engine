#ifndef TRIFECTA_TIME_H
#define TRIFECTA_TIME_H

#include <time.h>
#include <stdint.h>

#define _oneBillion (1000000000L)

/* 
 * Converts seconds to nanoseconds by multiplying by 
 * one billion
 */
#define secondsToNano(SECONDS) (SECONDS * _oneBillion)

#ifdef __unix__

typedef struct timespec TimePoint;

#endif /* end __unix__ */

#ifdef WIN32

#include "Trifecta_Win32.h"

typedef int64_t TimePoint;

#endif /* end WIN32 */

/* 
 * Returns TimePoint representing the specified number
 * of nanoseconds
 */
TimePoint makeTimeNano(uint64_t nanoseconds);

/* Returns a TimePoint representing 0 time */
#define makeTimeZero makeTimeNano(0)

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
int timePointCompare(TimePoint left, TimePoint right);

/*
 * Returns the difference between the two specified
 * TimePoints in nanoseconds
 */
int64_t timePointDiffNano(
    TimePoint left,
    TimePoint right
);

/* Sleeps until the specified TimePoint */
void sleepUntil(TimePoint timePoint);

#endif