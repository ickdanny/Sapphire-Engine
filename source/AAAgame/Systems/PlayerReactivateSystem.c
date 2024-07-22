#include "PlayerReactivateSystem.h"

/* Reactivates the player after respawning */
void playerReactivateSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    if(scenePtr->messages.playerStateEntry.state
        == player_respawnIFrames
    ){
        /* re-add pickup collisions */
        PickupCollisionTarget pickupCollisionTarget
            = collision_none;
        windWorldHandleAddComponent(
            PickupCollisionTarget,
            &(scenePtr->ecsWorld),
            scenePtr->messages.playerStateEntry
                .playerHandle,
            &pickupCollisionTarget
        );
    }
}