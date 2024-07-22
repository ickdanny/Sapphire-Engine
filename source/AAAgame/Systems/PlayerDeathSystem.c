#include "PlayerDeathSystem.h"

/*
 * Detects when a player dies based on state changes
 * and publishes the corresponding death message
 */
void playerDeathSystem(Game *gamePtr, Scene *scenePtr){
    if(scenePtr->messages.playerStateEntry.state
        == player_dead
    ){
        arrayListPushBack(WindEntity,
            &(scenePtr->messages.deaths),
            scenePtr->messages.playerStateEntry
                .playerHandle
        );
    }
}