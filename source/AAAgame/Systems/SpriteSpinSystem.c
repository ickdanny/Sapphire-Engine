#include "SpriteSpinSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(SpriteInstructionId)
    | vecsComponentSetFromId(SpriteSpinId);

/* Applies a constant spin to an entity's sprite */
void spriteSpinSystem(Game *gamePtr, Scene *scenePtr){
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
        SpriteSpin spriteSpin = vecsQueryItrGet(
            SpriteSpin,
            &itr
        );
        spriteInstructionPtr->rotation += spriteSpin;
        vecsQueryItrAdvance(&itr);
    }
}