#include "PlayerMovementSystem.h"

#include "GameCommand.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the player movement system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the player movement system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PlayerDataId);
        bitsetSet(&accept, VelocityId);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* Holds data pertaining to player movement input */
typedef uint_fast8_t PlayerMoveState;

#define upMask ((PlayerMoveState)1)
#define downMask ((PlayerMoveState)2)
#define leftMask ((PlayerMoveState)4)
#define rightMask ((PlayerMoveState)8)
#define focusMask ((PlayerMoveState)16)

#define verticalMask (upMask | downMask)
#define horizontalMask (leftMask | rightMask)
#define omniDirectionalMask \
    (verticalMask | horizontalMask)

/*
 * Returns true if the player is either not moving
 * or is attempting to move in all directions
 * simultaneously, false otherwise
 */
#define isZero(MOVESTATE) \
    ((MOVESTATE & omniDirectionalMask) == 0 \
        || (MOVESTATE & omniDirectionalMask) \
            == omniDirectionalMask)

/*
 * Returns true if the player is moving up, false
 * otherwise
 */
#define isUp(MOVESTATE) (MOVESTATE & upMask)

/*
 * Returns true if the player is moving down, false
 * otherwise
 */
#define isDown(MOVESTATE) (MOVESTATE & downMask)

/*
 * Returns true if the player is moving left, false
 * otherwise
 */
#define isLeft(MOVESTATE) (MOVESTATE & leftMask)

/*
 * Returns true if the player is moving right, false
 * otherwise
 */
#define isRight(MOVESTATE) (MOVESTATE & rightMask)

/*
 * Returns true if the player is focused, false
 * otherwise
 */
#define isFocused(MOVESTATE) (MOVESTATE & focusMask)

/* helper macro for setting state */
#define _setHelper(MOVESTATE, VALUE, MASK) \
    do{ \
        if(VALUE){ \
            MOVESTATE |= MASK; \
        } \
        else{ \
            MOVESTATE &= ~MASK; \
        } \
    } while(false)

/* Sets the up state to the specified value */
#define setUp(MOVESTATE, UP) \
    _setHelper(MOVESTATE, UP, upMask)

/* Sets the down state to the specified value */
#define setDown(MOVESTATE, DOWN) \
    _setHelper(MOVESTATE, DOWN, downMask)

/* Sets the left state to the specified value */
#define setLeft(MOVESTATE, LEFT) \
    _setHelper(MOVESTATE, LEFT, leftMask)

/* Sets the right state to the specified value */
#define setRight(MOVESTATE, RIGHT) \
    _setHelper(MOVESTATE, RIGHT, rightMask)

/* Sets the focus state to the specified value */
#define setFocus(MOVESTATE, FOCUS) \
    _setHelper(MOVESTATE, FOCUS, focusMask)

/*
 * Returns true if the player is in a state where they
 * can move, false otherwise
 */
static bool playerCanMove(
    const PlayerData *playerDataPtr
){
    switch(playerDataPtr->stateMachine.state){
        case player_normal:
        case player_bombing:
        case player_respawnIFrames:
            return true;
        case player_dead:
        case player_respawning:
        case player_gameOver:
            return false;
        default:
            pgError(
                "unexpected default player state; "
                SRC_LOCATION
            );
            return false;
    }
}

/*
 * Converts a move state into the corresponding
 * velocity (polar vector)
 */
static Velocity moveStateToVelocity(
    PlayerMoveState moveState
){
    /* if state is zero, return zero vector */
    if(isZero(moveState)){
        return ((Velocity){0});
    }

    /* use ints for precision */
    int x = 0;
    int y = 0;
    if(isUp(moveState)){
        ++y;
    }
    if(isDown(moveState)){
        --y;
    }
    if(isLeft(moveState)){
        --x;
    }
    if(isRight(moveState)){
        ++x;
    }
    if(x || y){
        Vector2D cartesian = {x, y};
        float magnitude = isFocused(moveState)
            ? config_focusedSpeed
            : config_playerSpeed;
        Velocity toRet = polarFromVector(cartesian);
        polarSetMagnitude(&toRet, magnitude);
        return toRet;
    }
    else{
        return ((Velocity){0});
    }
}

/* Handles moving the player according to input */
void playerMovementSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    init();

    ArrayList *gameCommandsPtr
        = &(scenePtr->messages.gameCommands);

    /*
     * even if no game commands, still need to set
     * the zero vector
     */

    /* collect all game commands into a move state */
    PlayerMoveState moveState = 0;
    for(int i = 0; i < gameCommandsPtr->size; ++i){
        switch(arrayListGet(GameCommand,
            gameCommandsPtr,
            i
        )){
            case game_focus:
                setFocus(moveState, true);
                break;
            case game_up:
                setUp(moveState, true);
                break;
            case game_down:
                setDown(moveState, true);
                break;
            case game_left:
                setLeft(moveState, true);
                break;
            case game_right:
                setRight(moveState, true);
                break;
            default:
                /* ignore all other game commands */
                break;
        }
    }

    /* translate move state into a move vector */
    Velocity velocity = moveStateToVelocity(moveState);

    /* get entities with player data and velocity */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    while(vecsQueryItrHasEntity(&itr)){
        PlayerData *playerDataPtr = vecsQueryItrGetPtr(
            PlayerData,
            &itr
        );
        Velocity *velocityPtr = vecsQueryItrGetPtr(
            Velocity,
            &itr
        );
        if(playerCanMove(playerDataPtr)){
            *velocityPtr = velocity;
        }
        else{
            *velocityPtr = (Velocity){0};
        }
        vecsQueryItrAdvance(&itr);
    }
}