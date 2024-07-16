#include "LoadingScreenSystem.h"

#define timerTicks 75

/* Handles the countdown for the loading screen */
void loadingScreenSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    /* bail if the scene is not the loading screen */
    if(scenePtr->id != scene_loading){
        return;
    }

    /* decrement timer if needed */
    if(scenePtr->messages.timer1 > 0){
        --(scenePtr->messages.timer1);
    }
    /* if timer over, pop the scene */
    if(scenePtr->messages.timer1 == 0){
        --(scenePtr->messages.timer1);
        gamePtr->messages.sceneExitToID = scene_game;
    }
    /* if timer was never set, set the timer */
    else if(scenePtr->messages.timer1 == -1){
        scenePtr->messages.timer1 = timerTicks;
    }
}