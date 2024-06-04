#ifndef TRIFECTA_CONCURRENCY_H
#define TRIFECTA_CONCURRENCY_H

#include <stdbool.h>

#ifdef __APPLE__

#include <pthread.h>

typedef pthread_t Thread;

#endif /* end __APPLE__ */

/* 
 * A typedef for a function which can be passed
 * into threadCreate()
 */
typedef void*(*RunnableFuncPtr)(void*);

/* 
 * The tuple returned by threadCreate() with a bool
 * indicating success and a Thread for the newly
 * created thread if successful
 */
typedef struct CreateReturn{
    bool success;
    Thread thread;
} CreateReturn;

/* 
 * Creates a new thread and returns its ID if
 * successful
 */
CreateReturn threadCreate(
    RunnableFuncPtr func,
    void* arg
);

/* Kills the specified thread */
void threadKill(Thread toKill);

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