#include "SpriteRenderSystem.h"

static Bitset normalSpriteAccept;
static Bitset normalSpriteReject;
static Bitset subSpriteAccept;
static Bitset subSpriteReject;
static Bitset tileSpriteAccept;
static Bitset tileSpriteReject;
static bool initialized = false;

/* destroys the sprite render system */
static void destroy(){
    if(initialized){
        bitsetFree(&normalSpriteAccept);
        bitsetFree(&normalSpriteReject);
        bitsetFree(&subSpriteAccept);
        bitsetFree(&subSpriteReject);
        bitsetFree(&tileSpriteAccept);
        bitsetFree(&tileSpriteReject);

        initialized = false;
    }
}

/* inits the sprite render system */
static void init(){
    if(!initialized){
        normalSpriteAccept = bitsetMake(numComponents);
        bitsetSet(&normalSpriteAccept, PositionID);
        bitsetSet(
            &normalSpriteAccept,
            VisibleMarkerID
        );
        bitsetSet(
            &normalSpriteAccept,
            SpriteInstructionID
        );

        //todo: rest of component sets for render system
        normalSpriteReject = bitsetMake(numComponents);
        subSpriteAccept = bitsetMake(numComponents);
        subSpriteReject = bitsetMake(numComponents);
        tileSpriteAccept = bitsetMake(numComponents);
        tileSpriteReject = bitsetMake(numComponents);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* Renders sprites to screen */
void spriteRenderSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    init();

    /* draw normal sprites */
    WindQueryItr normalItr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &normalSpriteAccept,
        &normalSpriteReject
    );
    while(windQueryItrHasEntity(&normalItr)){
        Position *positionPtr = windQueryItrGetPtr(
            Position,
            &normalItr
        );
        SpriteInstruction *spriteInstrPtr
            = windQueryItrGetPtr(SpriteInstruction,
                &normalItr
            );
        tfWindowDrawSprite(
            gamePtr->windowPtr,
            positionPtr->currentPos,
            spriteInstrPtr
        );
    }
}