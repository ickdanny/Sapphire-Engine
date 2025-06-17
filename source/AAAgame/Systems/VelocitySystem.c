#include "VelocitySystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the velocity system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the velocity system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PositionId);
        bitsetSet(&accept, VelocityId);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* updates position according to velocity */
void velocitySystem(Game *gamePtr, Scene *scenePtr){
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
        Velocity *velocityPtr = windQueryItrGetPtr(
            Velocity,
            &itr
        );
        Velocity scaledVelocity = polarDivide(
            *velocityPtr,
            config_updatesPerSecond
        );
        positionPtr->pastPos = positionPtr->currentPos;
        positionPtr->currentPos = point2DAddPolar(
            positionPtr->currentPos,
            scaledVelocity
        );
        vecsQueryItrAdvance(&itr);
    }
}