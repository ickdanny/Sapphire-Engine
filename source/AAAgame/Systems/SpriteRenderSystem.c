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
        bitsetSet(&normalSpriteAccept, PositionID);
        bitsetSet(
            &normalSpriteAccept,
            VisibleMarkerID
        );
        bitsetSet(
            &normalSpriteAccept,
            SpriteInstructionID
        );

        normalSpriteReject = bitsetMake(numComponents);
        bitsetSet(&normalSpriteReject, SubImageID);
        bitsetSet(
            &normalSpriteReject,
            TilingInstructionID
        );

        subSpriteAccept = bitsetMake(numComponents);
        bitsetSet(&subSpriteAccept, PositionID);
        bitsetSet(&subSpriteAccept, VisibleMarkerID);
        bitsetSet(
            &subSpriteAccept,
            SpriteInstructionID
        );
        bitsetSet(&subSpriteAccept, SubImageID);

        tileSpriteAccept = bitsetMake(numComponents);
        bitsetSet(&tileSpriteAccept, PositionID);
        bitsetSet(&tileSpriteAccept, VisibleMarkerID);
        bitsetSet(
            &tileSpriteAccept,
            SpriteInstructionID
        );
        bitsetSet(
            &tileSpriteAccept,
            TilingInstructionID
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
        windQueryItrAdvance(&normalItr);
    }

    /* draw sub sprites */
    WindQueryItr subItr = windWorldRequestQueryItr(
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
        windQueryItrAdvance(&subItr);
    }

    /* draw tiled sprites */
    WindQueryItr tileItr = windWorldRequestQueryItr(
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
        windQueryItrAdvance(&tileItr);
    }
}