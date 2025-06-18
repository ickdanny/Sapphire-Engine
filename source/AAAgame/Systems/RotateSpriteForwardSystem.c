#include "RotateSpriteForwardSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(SpriteInstructionId)
    | vecsComponentSetFromId(VelocityId)
    | vecsComponentSetFromId(
        RotateSpriteForwardMarkerId
    );

/*
 * rotates the entity's sprite to point in the
 * direction it is moving
 */
void rotateSpriteForwardSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    /* get entities with position and velocity */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&itr)){
        SpriteInstruction *spriteInstructionPtr
            = vecsQueryItrGetPtr(
                SpriteInstruction,
                &itr
            );
        Velocity *velocityPtr = vecsQueryItrGetPtr(
            Velocity,
            &itr
        );
        spriteInstructionPtr->rotation
            = -velocityPtr->angle - 90.0f;
        vecsQueryItrAdvance(&itr);
    }
}