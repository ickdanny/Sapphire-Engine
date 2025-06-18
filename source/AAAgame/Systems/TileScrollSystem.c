#include "TileScrollSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(SpriteInstructionId)
    | vecsComponentSetFromId(TilingInstructionId)
    | vecsComponentSetFromId(TileScrollId);

/* scrolls tiled sprites */
void tileScrollSystem(Game *gamePtr, Scene *scenePtr){
    /* get entities that need tile scrolling */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&itr)){
        SpriteInstruction *spriteInstrPtr
            = vecsQueryItrGetPtr(
                SpriteInstruction,
                &itr
            );
        TilingInstruction *tilingInstrPtr
            = vecsQueryItrGetPtr(
                TilingInstruction,
                &itr
            );
        TileScroll tileScroll = vecsQueryItrGet(
            TileScroll,
            &itr
        );

        /* add the scroll to the pixel offset */
        tilingInstrPtr->pixelOffset
            = point2DAddVector2D(
                tilingInstrPtr->pixelOffset,
                vector2DDivide(
                    tileScroll,
                    config_updatesPerSecond
                )
            );

        /*
         * normalize pixel offset to within the bounds
         * of the actual sprite
         */
        float spriteWidth
            = spriteInstrPtr->spritePtr->width;
        float spriteHeight
            = spriteInstrPtr->spritePtr->height;

        if(tilingInstrPtr->pixelOffset.x < 0.0f){
            tilingInstrPtr->pixelOffset.x
                += spriteWidth;
        }
        else if(tilingInstrPtr->pixelOffset.x
            >= spriteWidth
        ){
            tilingInstrPtr->pixelOffset.x
                -= spriteWidth;
        }
        if(tilingInstrPtr->pixelOffset.y < 0.0f){
            tilingInstrPtr->pixelOffset.y
                += spriteHeight;
        }
        else if(tilingInstrPtr->pixelOffset.y
            >= spriteHeight
        ){
            tilingInstrPtr->pixelOffset.y
                -= spriteHeight;
        }

        vecsQueryItrAdvance(&itr);
    }
}