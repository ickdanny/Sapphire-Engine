#include "PlayerRespawnSystem.h"

/* Handles respawning the player after death */
void playerRespawnSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    if(scenePtr->messages.playerStateEntry.state
        == player_respawning
    ){
        pgWarning("player respawn system");
        WindEntity playerHandle
            = scenePtr->messages.playerStateEntry
                .playerHandle;
        PlayerData *playerDataPtr
            = windWorldHandleGetPtr(PlayerData,
                &(scenePtr->ecsWorld),
                playerHandle
            );

        /* make the player collidable */
        windWorldHandleAddComponent(CollidableMarker,
            &(scenePtr->ecsWorld),
            playerHandle,
            NULL
        );

        /* reset player's position to spawn */
        Position position = {
            config_playerSpawn,
            config_playerSpawn
        };
        windWorldHandleSetComponent(Position,
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
            playerDataPtr->lives
                = config_continueLives;
        }
    }
}