#include "RotateSpriteForwardSystem.h"

static Bitset accept;
static bool initialized = false;

/* destroys the rotate sprite forward system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the rotate sprite forward system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, SpriteInstructionID);
        bitsetSet(&accept, VelocityID);
        bitsetSet(
            &accept,
            RotateSpriteForwardMarkerID
        );

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/*
 * rotates the entity's sprite to point in the
 * direction it is moving
 */
void rotateSpriteForwardSystem(
    Game *gamePtr,
    Scene *scenePtr
){
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
        Velocity *velocityPtr = windQueryItrGetPtr(
            Velocity,
            &itr
        );
        spriteInstructionPtr->rotation
            = -velocityPtr->angle - 90.0f;
        windQueryItrAdvance(&itr);
    }
}