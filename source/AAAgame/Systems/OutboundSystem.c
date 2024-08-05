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
        bitsetSet(&accept, PositionID);
        bitsetSet(&accept, OutboundID);

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
    WindQueryItr itr = windWorldRequestQueryItr(
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
            windWorldIDQueueRemoveEntity(
                &(scenePtr->ecsWorld),
                windQueryItrCurrentID(&itr)
            );
        }
        
        windQueryItrAdvance(&itr);
    }
    windWorldHandleOrders(&(scenePtr->ecsWorld));
}