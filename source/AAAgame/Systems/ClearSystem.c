#include "ClearSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

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
        bitsetSet(&accept, ClearableMarkerId);

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
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    /* kill all entities with clear marker */
    while(windQueryItrHasEntity(&itr)){
        VecsEntity handle = vecsWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentId(&itr)
        );
        arrayListPushBack(VecsEntity,
            &(scenePtr->messages.deaths),
            handle
        );
        vecsQueryItrAdvance(&itr);
    }
}