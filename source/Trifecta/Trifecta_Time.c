#include "Trifecta_Time.h"

#include <stdbool.h>

#define oneBillion (1000000000L)

#ifdef __APPLE__

#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

/* Returns the current time */
TimePoint getCurrentTime(){
    /* 
     * OSX clock_gettime resolution is 1000ns; for
     * more precision use mach_absolute_time 
     */

    /* get absolute time */
    uint64_t time = mach_absolute_time();
    /* convert to Nanoseconds */
    Nanoseconds timeNano = AbsoluteToNanoseconds(
        *(AbsoluteTime*)&time
    );
    /* convert to TimePoint */
    TimePoint toRet = {0};
    toRet = addTimeNano(toRet, *(uint64_t*)&timeNano);
    return toRet;
}

/* 
 * Returns the result of adding the specified amount
 * of nanoseconds to the specified TimePoint
 */
TimePoint addTimeNano(
    TimePoint timePoint,
    uint64_t timeNano
){
    unsigned int secondsToAdd = timeNano / oneBillion;
    uint64_t nanosecondsToAdd = timeNano % oneBillion;
    timePoint.tv_sec += secondsToAdd;
    timePoint.tv_nsec += nanosecondsToAdd;

    /* carry if needed*/
    if(timePoint.tv_nsec >= oneBillion){
        ++(timePoint.tv_sec);
        timePoint.tv_nsec -= oneBillion;
    }

    return timePoint;
}

/* Sleeps until the specified TimePoint */
void sleepUntil(TimePoint timePoint){
    TimePoint currentTime = getCurrentTime();
    bool completed = timePointCompare(
        currentTime,
        timePoint
    ) >= 0;
    while(!completed){
        //todo: nanosleep
    }
}

#endif /* end __APPLE__ */