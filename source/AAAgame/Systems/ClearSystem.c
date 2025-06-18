#include "ClearSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(VecsEntityId)
    | vecsComponentSetFromId(ClearableMarkerId);

/* clears bullets when clear flag is set */
void clearSystem(Game *gamePtr, Scene *scenePtr){
    /* bail if clear flag not set */
    if(!scenePtr->messages.clearFlag){
        return;
    }
    
    scenePtr->messages.clearFlag = false;

    /* get entities with clear marker */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    /* kill all entities with clear marker */
    while(vecsQueryItrHasEntity(&itr)){
        VecsEntity entity = vecsQueryItrGet(VecsEntity,
            &itr
        );
        arrayListPushBack(VecsEntity,
            &(scenePtr->messages.deaths),
            entity
        );
        vecsQueryItrAdvance(&itr);
    }
}