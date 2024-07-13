#include "MessageCleanupSystem.h"

/* cleans up messages left over from the last update */
void messageCleanupSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    //todo cleanup deaths
    scenePtr->messages.pauseFlag = false;
}