#include "PlayerStateSystem.h"

#include "GameCommand.h"

#define noTimer -1

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(VecsEntityId)
    | vecsComponentSetFromId(PlayerDataId);

/*
 * Returns true if has used a bomb this tick, false
 * otherwise
 */
static bool isPlayerBomb(
    Scene *scenePtr,
    PlayerData *playerDataPtr
){
    if(playerDataPtr->bombs > 0){
        ArrayList *gameCommandsPtr
            = &(scenePtr->messages.gameCommands);
        for(size_t i = 0;
            i < gameCommandsPtr->size;
            ++i
        ){
            GameCommand command = arrayListGet(
                GameCommand,
                gameCommandsPtr,
                i
            );
            if(command == game_bomb){
                return true;
            }
        }
    }
    return false;
}

/*
 * performs entry operations defined by the current
 * state of the state machine
 */
static void onEntry(
    Scene *scenePtr,
    VecsEntity playerHandle,
    PlayerData *playerDataPtr
){
    PlayerState state
        = playerDataPtr->stateMachine.state;
    scenePtr->messages.playerStateEntry.playerHandle
        = playerHandle;
    scenePtr->messages.playerStateEntry.state = state;

    switch(state){
        case player_none:
            /* do nothing */
            break;
        case player_normal:
        case player_gameOver:
            playerDataPtr->stateMachine.timer
                = noTimer;
            break;
        case player_bombing:
            playerDataPtr->stateMachine.timer
                = config_bombIFrameTicks;
            break;
        case player_dead:
            playerDataPtr->stateMachine.timer
                = config_deathTicks;
            break;
        case player_respawning:
            playerDataPtr->stateMachine.timer
                = config_respawnTicks;
            break;
        case player_respawnIFrames:
            playerDataPtr->stateMachine.timer
                = config_respawnIFrameTicks;
            break;
        default:
            pgError(
                "Unexpected default player state; "
                SRC_LOCATION
            );
            break;
    }
}

/*
 * updates the state machine and returns the next state
 */
static PlayerState onUpdate(
    Scene *scenePtr,
    VecsEntity playerHandle,
    PlayerData *playerDataPtr
){
    switch(playerDataPtr->stateMachine.state){
        case player_none:
            /* send to normal state */
            return player_normal;
        case player_normal:
            /* check for bomb */
            if(isPlayerBomb(scenePtr, playerDataPtr)){
                return player_bombing;
            }
            /* check for death clock */
            if(playerDataPtr->stateMachine.timer > 0){
                --(playerDataPtr->stateMachine.timer);
            }
            else if(
                playerDataPtr->stateMachine.timer == 0
            ){
                return player_dead;
            }
            /*
             * if hit and not on timer, start death
             * bomb timer
             */
            else if(scenePtr->messages.playerHits.size
                > 0
            ){
                playerDataPtr->stateMachine.timer
                    = config_deathBombTicks;
            }
            return player_normal;
        case player_bombing:
            /* check to see if bomb state is over */
            if(playerDataPtr->stateMachine.timer > 0){
                --(playerDataPtr->stateMachine.timer);
            }
            else if(playerDataPtr->stateMachine.timer
                == 0
            ){
                return player_normal;
            }
            return player_bombing;
        case player_dead:
            /* check to see if dead state is over */
            if(playerDataPtr->stateMachine.timer > 0){
                --(playerDataPtr->stateMachine.timer);
            }
            else if(playerDataPtr->stateMachine.timer
                == 0
            ){
                /*
                 * check lives and continues to decide
                 * whether to respawn or not
                 */
                if(playerDataPtr->lives <= 0 
                    && playerDataPtr->continues <= 0
                ){
                    return player_gameOver;
                }
                return player_respawning;
            }
            return player_dead;
        case player_respawning:
            /* check to see if respawn state is over */
            if(playerDataPtr->stateMachine.timer > 0){
                --(playerDataPtr->stateMachine.timer);
            }
            else if(playerDataPtr->stateMachine.timer
                == 0
            ){
                return player_respawnIFrames;
            }
            return player_respawning;
        case player_respawnIFrames:
            /* check for bomb */
            if(isPlayerBomb(scenePtr, playerDataPtr)){
                return player_bombing;
            }
            /*
             * check to see if respawn IFrame state is
             * over
             */
            if(playerDataPtr->stateMachine.timer > 0){
                --(playerDataPtr->stateMachine.timer);
            }
            else if(playerDataPtr->stateMachine.timer
                == 0
            ){
                return player_normal;
            }
            return player_respawnIFrames;
        case player_gameOver:
            /* dead end; self loop */
            return player_gameOver;
        default:
            pgError(
                "Unexpected default player state; "
                SRC_LOCATION
            );
            return player_none;
            break;
    }
}

/*
 * performs exit operations defined by the current
 * state of the state machine
 */
static void onExit(
    Scene *scenePtr,
    VecsEntity playerHandle,
    PlayerData *playerDataPtr
){
    switch(playerDataPtr->stateMachine.state){
        case player_none:
            /* do nothing */
            break;
        case player_normal:
        case player_bombing:
        case player_dead:
        case player_respawning:
        case player_respawnIFrames:
            playerDataPtr->stateMachine.timer
                = noTimer;
            break;
        case player_gameOver:
            /* do nothing */
            break;
        default:
            pgError(
                "Unexpected default player state; "
                SRC_LOCATION
            );
            break;
    }
}

/* Updates the state machine of the specified player */
static void updatePlayerStateMachine(
    Scene *scenePtr,
    VecsEntity playerHandle,
    PlayerData *playerDataPtr
){
    PlayerState nextState = onUpdate(
        scenePtr,
        playerHandle,
        playerDataPtr
    );
    /* if state different, move to new state */
    if(nextState != playerDataPtr->stateMachine.state){
        onExit(scenePtr, playerHandle, playerDataPtr);
        playerDataPtr->stateMachine.state = nextState;
        onEntry(scenePtr, playerHandle, playerDataPtr);

        /* update the new state */
        updatePlayerStateMachine(
            scenePtr,
            playerHandle,
            playerDataPtr
        );
    }
}

/* handles updating the player state machine */
void playerStateSystem(Game *gamePtr, Scene *scenePtr){
    /* clear player state entry */
    scenePtr->messages.playerStateEntry.playerHandle
        = (VecsEntity){0};
    scenePtr->messages.playerStateEntry.state
        = player_none;

    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&itr)){
        VecsEntity entity = vecsQueryItrGet(VecsEntity,
            &itr
        );

        PlayerData *playerDataPtr
            = vecsWorldEntityGetPtr(PlayerData,
                &(scenePtr->ecsWorld),
                entity
            );

        updatePlayerStateMachine(
            scenePtr,
            entity,
            playerDataPtr
        );

        vecsQueryItrAdvance(&itr);
    }
}