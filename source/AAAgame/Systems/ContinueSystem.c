#include "ContinueSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the continue system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the continue system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PlayerDataID);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* retrieves the player handle */
static WindEntity getPlayerHandle(Scene *scenePtr){
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    /* get first player */
    if(windQueryItrHasEntity(&itr)){
        return windWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentID(&itr)
        );
    }
    /* error if cannot find player */
    else{
        pgError(
            "failed to find player; "
            SRC_LOCATION
        );
        return (WindEntity){0};
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
    arrayListPushBack(SceneID,
        &(gamePtr->messages.sceneEntryList),
        scene_continue
    );
    /* pause game */
    scenePtr->messages.pauseFlag = true;

    /*
     * send player data to global for init system to
     * create continue menu
     */
    WindEntity playerHandle
        = getPlayerHandle(scenePtr);
    PlayerData *playerDataPtr = windWorldHandleGetPtr(
        PlayerData,
        &(scenePtr->ecsWorld),
        playerHandle
    );
    gamePtr->messages.playerData.data = *playerDataPtr;
    gamePtr->messages.playerData.isPresent = true;
}

/*
 * Handles the return from the continue menu if the
 * player chooses to continue and return to the game
 */
static void returnFromContinueMenu(Scene *scenePtr){
    WindEntity playerHandle
        = getPlayerHandle(scenePtr);
    PlayerData *playerDataPtr = windWorldHandleGetPtr(
        PlayerData,
        &(scenePtr->ecsWorld),
        playerHandle
    );
    /* continue decrement occurs during respawning */
}

/* Handles entering and exiting the continue menu */
void continueSystem(Game *gamePtr, Scene *scenePtr){
    init();

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
            WindEntity playerHandle
                = getPlayerHandle(scenePtr);
            PlayerData *playerDataPtr
                = windWorldHandleGetPtr(
                    PlayerData,
                    &(scenePtr->ecsWorld),
                    playerHandle
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