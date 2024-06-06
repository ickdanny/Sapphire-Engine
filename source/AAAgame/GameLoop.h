#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <stdbool.h>

/* 
 * The GameLoop class handles the scheduling of 
 * game updates
 */
typedef struct GameLoop{
    bool running;
    int updatesPerSecond;
    int maxUpdatesWithoutFrame;
    void *updateUserPtr;
    void *renderUserPtr;
    
    /* 
     * The update function will be called with the
     * updateUserPtr
     */
    void (*updateFunc)(void*);
    /* 
     * The render function will be called with the
     * renderUserPtr
     */
    void (*renderFunc)(void*);
} GameLoop;

/* Constructs and returns a new GameLoop by value */
GameLoop gameLoopMake(
    int updatesPerSecond,
    int maxUpdatesPerFrame,
    void *updateUserPtr,
    void *renderUserPtr,
    void (*updateFunc)(void*),
    void (*renderFunc)(void*)
);

/* Begins running the specified GameLoop */
void gameLoopRun(GameLoop *gameLoopPtr);

/* Stops running the specified GameLoop */
void gameLoopStop(GameLoop *gameLoopPtr);

#endif