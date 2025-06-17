#include "CreditsSystem.h"

#define waitTicks (60 * 39)

/*
 * handles exiting the credits screen after a set
 * amount of time
 */
void creditsSystem(Game *gamePtr, Scene *scenePtr){
    /* bail if scene is not credits */
    if(scenePtr->id != scene_credits){
        return;
    }

    int *timer1Ptr = &(scenePtr->messages.timer1);
    if(*timer1Ptr > -1){
        /* if timer is done, return to menu */
        if(*timer1Ptr == 0){
            String *trackIdPtr = &(gamePtr->messages
                .startMusicString);
            stringClear(trackIdPtr);
            stringAppendC(trackIdPtr, "01");
            gamePtr->messages.sceneExitToId
                = scene_main;
        }
        --(*timer1Ptr);
    }
    else{
        *timer1Ptr = waitTicks;
    }
}