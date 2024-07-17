#include "AnimationSystem.h"

/*
 * The value below which a vector is considered 0 for
 * the purposes of deciding which animation
 * to use
 */
#define velocityEpsilon 0.01f

static Bitset accept;
static bool initialized = false;

/* destroys the animation system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the animation system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, SpriteInstructionID);
        bitsetSet(&accept, AnimationsID);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/*
 * Attempts to switch to the animation left of current;
 * returns true if the animation successfully changed,
 * false otherwise
 */
bool tryToTurnLeft(Animations *animationsPtr){
    /*
     * if the current index is already leftmost i.e. 0,
     * bail out
     */
    if(animationsPtr->currentIndex == 0){
        return false;
    }

    size_t nextIndex = animationsPtr->currentIndex - 1;

    /* reset indices of animations */
    arrayListGetPtr(Animation,
        &(animationsPtr->animations),
        animationsPtr->currentIndex
    )->currentIndex = 0;
    arrayListGetPtr(Animation,
        &(animationsPtr->animations),
        nextIndex
    )->currentIndex = 0;

    /* set animation left */
    animationsPtr->currentIndex = nextIndex;
    return true;
}

/*
 * Attempts to switch to the animation right of
 * current; returns true if the animation successfully
 * changed, false otherwise
 */
bool tryToTurnRight(Animations *animationsPtr){
    /*
     * if the current index is already rightmost,
     * bail out
     */
    if(animationsPtr->currentIndex + 1
        >= animationsPtr->animations.size
    ){
        return false;
    }

    size_t nextIndex = animationsPtr->currentIndex + 1;

    /* reset indices of animations */
    arrayListGetPtr(Animation,
        &(animationsPtr->animations),
        animationsPtr->currentIndex
    )->currentIndex = 0;
    arrayListGetPtr(Animation,
        &(animationsPtr->animations),
        nextIndex
    )->currentIndex = 0;

    /* set animation right */
    animationsPtr->currentIndex = nextIndex;
    return true;
}

/*
 * Attempts to switch to the animation closer to the
 * center (idle); returns true if animation
 * successfully changed, false otherwise
 */
bool tryToTurnCenter(Animations *animationsPtr){
    /* bail if already at center */
    if(animationsPtr->currentIndex
        == animationsPtr->idleIndex
    ){
        return false;
    }

    /* if currently left of center, go right */
    if(animationsPtr->currentIndex
        < animationsPtr->idleIndex
    ){
        return tryToTurnRight(animationsPtr);
    }
    /* if currently right of center, go left */
    else{
        return tryToTurnLeft(animationsPtr);
    }
}

/*
 * Handles changing the animation if an entity is
 * moving in a certain direction; returns true if the
 * animation changed, false otherwise
 */
bool handleTurning(
    Scene *scenePtr,
    WindEntity handle,
    Animations *animationsPtr
){
    /* bail if not enough animations to turn*/
    if(animationsPtr->animations.size <= 1){
        return false;
    }
    /* bail if entity has no velocity */
    if(!windWorldHandleContainsComponent(Velocity,
        &(scenePtr->ecsWorld),
        handle
    )){
        return false;
    }

    Velocity velocity = windWorldHandleGet(Velocity,
        &(scenePtr->ecsWorld),
        handle
    );
    float xVelocity = polarToVector(velocity).x;
    if(xVelocity < -velocityEpsilon){
        return tryToTurnLeft(animationsPtr);
    }
    else if(xVelocity > velocityEpsilon){
        return tryToTurnRight(animationsPtr);
    }
    else{
        return tryToTurnCenter(animationsPtr);
    }
}

/*
 * Updates the animation and sprite instruction of
 * a single entity; returns true if the animation
 * must continue, false if it is done and should be
 * removed
 */
bool handleAnimation(
    Scene *scenePtr,
    WindEntity handle,
    SpriteInstruction *spriteInstructionptr,
    Animations *animationsPtr
){
    //todo
}

/*
 * Handles animating entity graphics by looping through
 * frames and switching animations depending on
 * movement direction
 */
void animationSystem(Game *gamePtr, Scene *scenePtr){
    init();

    /* get entities with position and velocity */
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    while(windQueryItrHasEntity(&itr)){
        SpriteInstruction *spriteInstructionPtr
            = windQueryItrGetPtr(
                SpriteInstruction,
                &itr
            );
        Animations *animationsPtr = windQueryItrGetPtr(
            Animations,
            &itr
        );
        WindEntity handle = windWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentID(&itr)
        );
        //todo
        windQueryItrAdvance(&itr);
    }
}