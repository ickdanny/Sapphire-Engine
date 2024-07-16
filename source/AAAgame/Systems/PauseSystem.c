#include "PauseSystem.h"

#include "GameCommand.h"

/* Responsible for entering the pause menu */
void pauseSystem(Game *gamePtr, Scene *scenePtr){
    ArrayList *gameCommandsPtr
        = &(scenePtr->messages.gameCommands);

    /* bail if no game commands */
    if(arrayListIsEmpty(gameCommandsPtr)){
        return;
    }

    /* collect all game commands into a move state */
    for(int i = 0; i < gameCommandsPtr->size; ++i){
        if(arrayListGet(
                GameCommand,
                gameCommandsPtr,
                i
            ) == game_pause
        ){
            arrayListPushBack(SceneID,
                &(gamePtr->messages.sceneEntryList),
                scene_pause
            );
            scenePtr->messages.pauseFlag = true;
        }
    }
}