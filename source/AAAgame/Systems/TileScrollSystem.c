#include "TileScrollSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the tile scroll system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the tile scroll system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, SpriteInstructionID);
        bitsetSet(&accept, TilingInstructionID);
        bitsetSet(&accept, TileScrollID);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* scrolls tiled sprites */
void tileScrollSystem(Game *gamePtr, Scene *scenePtr){
    init();

    /* get entities that need tile scrolling */
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    while(windQueryItrHasEntity(&itr)){
        SpriteInstruction *spriteInstrPtr
            = windQueryItrGetPtr(
                SpriteInstruction,
                &itr
            );
        TilingInstruction *tilingInstrPtr
            = windQueryItrGetPtr(
                TilingInstruction,
                &itr
            );
        TileScroll tileScroll = windQueryItrGet(
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

        windQueryItrAdvance(&itr);
    }
}