#include "ClearSystem.h"

static Bitset accept;
static bool initialized = false;

/* destroys the clear system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the clear system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, ClearMarkerID);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* clears bullets when clear flag is set */
void clearSystem(Game *gamePtr, Scene *scenePtr){
    /* bail if clear flag not set */
    if(!scenePtr->messages.clearFlag){
        return;
    }
    
    scenePtr->messages.clearFlag = false;

    init();

    /* get entities with clear marker */
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    /* kill all entities with clear marker */
    while(windQueryItrHasEntity(&itr)){
        WindEntity handle = windWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentID(&itr)
        );
        arrayListPushBack(WindEntity,
            &(scenePtr->messages.deaths),
            handle
        );
        windQueryItrAdvance(&itr);
    }
}