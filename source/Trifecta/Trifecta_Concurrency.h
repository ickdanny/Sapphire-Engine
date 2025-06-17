#ifndef TRIFECTA_CONCURRENCY_H
#define TRIFECTA_CONCURRENCY_H

#include <stdbool.h>

#ifdef __unix__

#include <pthread.h>

typedef pthread_t Thread;

#endif /* end __unix__ */

#ifdef WIN32

#include "Trifecta_Win32.h"

typedef HANDLE Thread;

#endif /* end WIN32 */

#ifdef __unix__

/* 
 * A typedef for a function which can be passed
 * into threadCreate()
 */
typedef void*(*RunnableFuncPtr)(void*);

#define DECLARE_RUNNABLE_FUNC(FUNCNAME, PARAMNAME) \
    void* FUNCNAME(void *PARAMNAME)

#endif /* end __unix__ */

#ifdef WIN32

/* 
 * A typedef for a function which can be passed
 * into threadCreate()
 */
typedef DWORD (WINAPI *RunnableFuncPtr)(void*);

#define DECLARE_RUNNABLE_FUNC(FUNCNAME, PARAMNAME) \
    DWORD WINAPI FUNCNAME(void *PARAMNAME)

#endif /* end WIN32 */

/*
 * Initializes the current thread; should be called by
 * runnable functions
 */
void initThread();

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
 * Creates a new thread and returns its Id if
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