#include "SpriteSpinSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the sprite spin system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the sprite spin system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, SpriteInstructionID);
        bitsetSet(&accept, SpriteSpinID);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* Applies a constant spin to an entity's sprite */
void spriteSpinSystem(Game *gamePtr, Scene *scenePtr){
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
        SpriteSpin spriteSpin = windQueryItrGet(
            SpriteSpin,
            &itr
        );
        spriteInstructionPtr->rotation += spriteSpin;
        windQueryItrAdvance(&itr);
    }
}