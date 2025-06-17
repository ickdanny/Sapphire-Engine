#include "SpriteRenderSystem.h"

static Bitset normalSpriteAccept;
static Bitset normalSpriteReject;
static Bitset subSpriteAccept;
static Bitset tileSpriteAccept;
static bool initialized = false;

/* destroys the sprite render system */
static void destroy(){
    if(initialized){
        bitsetFree(&normalSpriteAccept);
        bitsetFree(&normalSpriteReject);
        bitsetFree(&subSpriteAccept);
        bitsetFree(&tileSpriteAccept);

        initialized = false;
    }
}

/* inits the sprite render system */
static void init(){
    if(!initialized){
        normalSpriteAccept = bitsetMake(numComponents);
        bitsetSet(&normalSpriteAccept, PositionId);
        bitsetSet(
            &normalSpriteAccept,
            VisibleMarkerId
        );
        bitsetSet(
            &normalSpriteAccept,
            SpriteInstructionId
        );

        normalSpriteReject = bitsetMake(numComponents);
        bitsetSet(&normalSpriteReject, SubImageId);
        bitsetSet(
            &normalSpriteReject,
            TilingInstructionId
        );

        subSpriteAccept = bitsetMake(numComponents);
        bitsetSet(&subSpriteAccept, PositionId);
        bitsetSet(&subSpriteAccept, VisibleMarkerId);
        bitsetSet(
            &subSpriteAccept,
            SpriteInstructionId
        );
        bitsetSet(&subSpriteAccept, SubImageId);

        tileSpriteAccept = bitsetMake(numComponents);
        bitsetSet(&tileSpriteAccept, PositionId);
        bitsetSet(&tileSpriteAccept, VisibleMarkerId);
        bitsetSet(
            &tileSpriteAccept,
            SpriteInstructionId
        );
        bitsetSet(
            &tileSpriteAccept,
            TilingInstructionId
        );

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
    VecsQueryItr normalItr = vecsWorldRequestQueryItr(
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
        vecsQueryItrAdvance(&normalItr);
    }

    /* draw sub sprites */
    VecsQueryItr subItr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &subSpriteAccept,
        NULL
    );
    while(windQueryItrHasEntity(&subItr)){
        Position *positionPtr = windQueryItrGetPtr(
            Position,
            &subItr
        );
        SpriteInstruction *spriteInstrPtr
            = windQueryItrGetPtr(SpriteInstruction,
                &subItr
            );
        SubImage *subImagePtr = windQueryItrGetPtr(
            SubImage,
            &subItr
        );
        tfWindowDrawSubSprite(
            gamePtr->windowPtr,
            positionPtr->currentPos,
            spriteInstrPtr,
            subImagePtr
        );
        vecsQueryItrAdvance(&subItr);
    }

    /* draw tiled sprites */
    VecsQueryItr tileItr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &tileSpriteAccept,
        NULL
    );
    while(windQueryItrHasEntity(&tileItr)){
        Position *positionPtr = windQueryItrGetPtr(
            Position,
            &tileItr
        );
        SpriteInstruction *spriteInstrPtr
            = windQueryItrGetPtr(SpriteInstruction,
                &tileItr
            );
        TilingInstruction *tilingInstrPtr
            = windQueryItrGetPtr(
                TilingInstruction,
                &tileItr
            );
        tfWindowDrawTileSprite(
            gamePtr->windowPtr,
            &(tilingInstrPtr->drawRect),
            spriteInstrPtr,
            tilingInstrPtr->pixelOffset
        );
        vecsQueryItrAdvance(&tileItr);
    }
}