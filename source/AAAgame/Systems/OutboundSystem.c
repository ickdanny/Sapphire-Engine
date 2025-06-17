#include "OutboundSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the outbound system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the outbound system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PositionId);
        bitsetSet(&accept, OutboundId);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/*
 * Removes entities which are outside of a certain
 * boundary from the edge of the game
 */
void outboundSystem(Game *gamePtr, Scene *scenePtr){
    init();

    /* get entities with position and velocity */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    while(windQueryItrHasEntity(&itr)){
        Position *positionPtr = windQueryItrGetPtr(
            Position,
            &itr
        );
        Outbound bound = windQueryItrGet(
            Outbound,
            &itr
        );

        if(isOutOfBounds(
            positionPtr->currentPos,
            bound)
        ){
            windWorldIdQueueRemoveEntity(
                &(scenePtr->ecsWorld),
                windQueryItrCurrentId(&itr)
            );
        }
        
        vecsQueryItrAdvance(&itr);
    }
    vecsWorldHandleOrders(&(scenePtr->ecsWorld));
}