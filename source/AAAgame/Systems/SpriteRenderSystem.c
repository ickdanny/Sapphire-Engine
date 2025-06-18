#include "SpriteRenderSystem.h"

static VecsComponentSet normalSpriteAccept
    = vecsComponentSetFromId(PositionId)
    | vecsComponentSetFromId(VisibleMarkerId)
    | vecsComponentSetFromId(SpriteInstructionId);

static VecsComponentSet normalSpriteReject
    = vecsComponentSetFromId(SubImageId)
    | vecsComponentSetFromId(TilingInstructionId);

static VecsComponentSet subSpriteAccept
    = vecsComponentSetFromId(PositionId)
    | vecsComponentSetFromId(VisibleMarkerId)
    | vecsComponentSetFromId(SpriteInstructionId)
    | vecsComponentSetFromId(SubImageId);

static VecsComponentSet tileSpriteAccept
    = vecsComponentSetFromId(PositionId)
    | vecsComponentSetFromId(VisibleMarkerId)
    | vecsComponentSetFromId(SpriteInstructionId)
    | vecsComponentSetFromId(TilingInstructionId);

/* Renders sprites to screen */
void spriteRenderSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    /* draw normal sprites */
    VecsQueryItr normalItr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        normalSpriteAccept,
        normalSpriteReject
    );
    while(vecsQueryItrHasEntity(&normalItr)){
        Position *positionPtr = vecsQueryItrGetPtr(
            Position,
            &normalItr
        );
        SpriteInstruction *spriteInstrPtr
            = vecsQueryItrGetPtr(SpriteInstruction,
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
        subSpriteAccept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&subItr)){
        Position *positionPtr = vecsQueryItrGetPtr(
            Position,
            &subItr
        );
        SpriteInstruction *spriteInstrPtr
            = vecsQueryItrGetPtr(SpriteInstruction,
                &subItr
            );
        SubImage *subImagePtr = vecsQueryItrGetPtr(
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
        tileSpriteAccept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&tileItr)){
        Position *positionPtr = vecsQueryItrGetPtr(
            Position,
            &tileItr
        );
        SpriteInstruction *spriteInstrPtr
            = vecsQueryItrGetPtr(SpriteInstruction,
                &tileItr
            );
        TilingInstruction *tilingInstrPtr
            = vecsQueryItrGetPtr(
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