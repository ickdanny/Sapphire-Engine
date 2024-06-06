#include "GameLoop.h"

#include "Trifecta.h"
#include "PGUtil.h"

/* Constructs and returns a new GameLoop by value */
GameLoop gameLoopMake(
    int updatesPerSecond,
    int maxUpdatesPerFrame,
    void *updateUserPtr,
    void *renderUserPtr,
    void (*updateFunc)(void*),
    void (*renderFunc)(void*)
){
    GameLoop toRet = {0};
    toRet.running = false;
    toRet.updatesPerSecond = updatesPerSecond;
    toRet.maxUpdatesWithoutFrame = maxUpdatesPerFrame;
    toRet.updateUserPtr = updateUserPtr;
    toRet.renderUserPtr = renderUserPtr;
    toRet.updateFunc = updateFunc;
    toRet.renderFunc = renderFunc;
    return toRet;
}

/* Begins running the specified GameLoop */
void gameLoopRun(GameLoop *gameLoopPtr){
    assertTrue(
        gameLoopPtr->updatesPerSecond > 0,
        "gameloop ups must be > 0"
    );
    double secondsBetweenUpdates
        = (1.0) / (gameLoopPtr->updatesPerSecond);
    uint64_t nanoBetweenUpdates 
        = secondsToNano(secondsBetweenUpdates);

    TimePoint nextUpdate = getCurrentTime();
    int updatesWithoutFrame = 0;
    int timeCompareResult = 0;

    /* the actual game loop */
    gameLoopPtr->running = true;
    while(gameLoopPtr->running){
		/* force draw every few updates */
		if(updatesWithoutFrame
            >= gameLoopPtr->maxUpdatesWithoutFrame
        ){
			(gameLoopPtr->renderFunc)(
                gameLoopPtr->renderUserPtr
            );
			updatesWithoutFrame = 0;
		}
		/* update if currentTime > nextUpdate */
        timeCompareResult = timePointCompare(
            getCurrentTime(),
            nextUpdate
        );
		if(timeCompareResult > 0){
			(gameLoopPtr->updateFunc)(
                gameLoopPtr->updateUserPtr
            );
            nextUpdate = addTimeNano(
                nextUpdate,
                nanoBetweenUpdates
            );
            /* 
             * update nextUpdate if
             * nextUpdate < currentTime 
             */
            timeCompareResult = timePointCompare(
                nextUpdate,
                getCurrentTime()
            );
			if(timeCompareResult < 0) {
				nextUpdate = getCurrentTime();
			}
		}
		/* 
         * draw frame if possible 
         * i.e. if currentTime < nextUpdate
         */
        timeCompareResult = timePointCompare(
            getCurrentTime(),
            nextUpdate
        );
		if(timeCompareResult < 0){
			(gameLoopPtr->renderFunc)(
                gameLoopPtr->renderUserPtr
            );
            /* sleep until the next update */
            sleepUntil(nextUpdate);
		}
		else {
			++updatesWithoutFrame;
		}
    }
}

/* Stops running the specified GameLoop */
void gameLoopStop(GameLoop *gameLoopPtr){
    gameLoopPtr->running = false;
}