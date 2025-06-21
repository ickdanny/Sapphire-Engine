#include "ContinueSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(VecsEntityId)
    | vecsComponentSetFromId(PlayerDataId);


/* retrieves the player entity */
static VecsEntity getPlayerEntity(Scene *scenePtr){
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    /* get first player */
    if(vecsQueryItrHasEntity(&itr)){
        return vecsQueryItrGet(VecsEntity, &itr);
    }
    /* error if cannot find player */
    else{
        pgError(
            "failed to find player; "
            SRC_LOCATION
        );
        return (VecsEntity){0};
    }
}

/*
 * enters the continue menu by signaling a scene push
 */
static void enterContinueMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    /* signal scene push */
    arrayListPushBack(SceneId,
        &(gamePtr->messages.sceneEntryList),
        scene_continue
    );
    /* pause game */
    scenePtr->messages.pauseFlag = true;

    /*
     * send player data to global for init system to
     * create continue menu
     */
    VecsEntity playerEntity
        = getPlayerEntity(scenePtr);
    PlayerData *playerDataPtr = vecsWorldEntityGetPtr(
        PlayerData,
        &(scenePtr->ecsWorld),
        playerEntity
    );
    gamePtr->messages.playerData.data = *playerDataPtr;
    gamePtr->messages.playerData.isPresent = true;
}

/*
 * Handles the return from the continue menu if the
 * player chooses to continue and return to the game
 */
static void returnFromContinueMenu(Scene *scenePtr){
    VecsEntity playerEntity
        = getPlayerEntity(scenePtr);
    PlayerData *playerDataPtr = vecsWorldEntityGetPtr(
        PlayerData,
        &(scenePtr->ecsWorld),
        playerEntity
    );
    /* continue decrement occurs during respawning */
}

/* Handles entering and exiting the continue menu */
void continueSystem(Game *gamePtr, Scene *scenePtr){
    /* bail if not in game */
    if(scenePtr->id != scene_game){
        return;
    }

    int *timer1Ptr = &(scenePtr->messages.timer1);

    /* if timer > -1, that means continue is active */
    if(*timer1Ptr > -1){
        /* if timer hits 1, go to continue screen */
        if(*timer1Ptr == 1){
            enterContinueMenu(gamePtr, scenePtr);
        }
        /*
         * if timer hits 0, indicates player has chosen
         * to continue
         */
        else if(*timer1Ptr == 0){
            returnFromContinueMenu(scenePtr);
        }
        /* decrement timer; it will end at -1 */
        --(*timer1Ptr);
    }
    /*
     * otherwise, check to see if we need to start a
     * continue timer
     */
    else{
        if(scenePtr->messages.playerStateEntry.state
            == player_dead
        ){
            /* check to see if player out of lives */
            VecsEntity playerEntity
                = getPlayerEntity(scenePtr);
            PlayerData *playerDataPtr
                = vecsWorldEntityGetPtr(PlayerData,
                    &(scenePtr->ecsWorld),
                    playerEntity
                );
            if(playerDataPtr->lives <= 0
                && playerDataPtr->continues > 0
            ){
                /*
                 * set a timer just before the player
                 * exits the death state
                 */
                scenePtr->messages.timer1
                    = config_deathTicks - 2;
            }
            /*
             * if out of continues, this system does
             * nothing; see game over system
             */
        }
    }
}