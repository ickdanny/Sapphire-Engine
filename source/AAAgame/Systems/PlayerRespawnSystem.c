#include "PlayerRespawnSystem.h"

/* Handles respawning the player after death */
void playerRespawnSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    if(scenePtr->messages.playerStateEntry.state
        == player_respawning
    ){
        VecsEntity playerHandle
            = scenePtr->messages.playerStateEntry
                .playerHandle;
        PlayerData *playerDataPtr
            = vecsWorldEntityGetPtr(PlayerData,
                &(scenePtr->ecsWorld),
                playerHandle
            );

        /* make the player collidable */
        windWorldHandleAddComponent(CollidableMarker,
            &(scenePtr->ecsWorld),
            playerHandle,
            NULL
        );
        /*
         * note that pickup collisions are reinstated
         * in the reactivate system
         */

        /* reset player's position to spawn */
        Position position = {
            config_playerSpawn,
            config_playerSpawn
        };
        vecsWorldEntitySetComponent(Position,
            &(scenePtr->ecsWorld),
            playerHandle,
            &position
        );

        playerDataPtr->bombs = config_respawnBombs;
        if(playerDataPtr->lives > 0){
            --(playerDataPtr->lives);
        }
        else{
            /* only occurs upon continue */
            if(playerDataPtr->continues <= 0){
                pgError(
                    "expect player to have continues "
                    "if respawning with no lives; "
                    SRC_LOCATION
                );
            }
            --(playerDataPtr->continues);
            playerDataPtr->lives
                = config_continueLives;
        }
    }
}