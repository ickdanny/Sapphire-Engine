#ifndef TRIFECTA_CONCURRENCY_H
#define TRIFECTA_CONCURRENCY_H

#ifdef __APPLE__

#include <pthread.h>

typedef pthread_t Thread;

#endif /* end __APPLE__ */

/* 
 * Represents the possible results from a call to
 * threadJoin()
 */
typedef enum JoinReturnCode{
    joinSuccess,
    joinNotJoinable,
    joinNoSuchThread,
    joinDeadlock,
    joinUnknownError
} JoinReturnCode;

/* Attempts to join the specified thread */
JoinReturnCode threadJoin(Thread thread);

#endif