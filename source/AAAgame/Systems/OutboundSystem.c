#include "OutboundSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(VecsEntityId)
    | vecsComponentSetFromId(PositionId)
    | vecsComponentSetFromId(OutboundId);

/*
 * Removes entities which are outside of a certain
 * boundary from the edge of the game
 */
void outboundSystem(Game *gamePtr, Scene *scenePtr){
    /* get entities with position and velocity */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&itr)){
        Position *positionPtr = vecsQueryItrGetPtr(
            Position,
            &itr
        );
        Outbound bound = vecsQueryItrGet(
            Outbound,
            &itr
        );

        if(isOutOfBounds(
            positionPtr->currentPos,
            bound)
        ){
            VecsEntity entity = vecsQueryItrGet(
                VecsEntity,
                &itr
            );
            vecsWorldEntityQueueRemoveEntity(
                &(scenePtr->ecsWorld),
                entity
            );
        }
        
        vecsQueryItrAdvance(&itr);
    }
    vecsWorldHandleOrders(&(scenePtr->ecsWorld));
}