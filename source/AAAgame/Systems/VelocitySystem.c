#include "VelocitySystem.h"

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
        bitsetSet(&accept, PositionID);
        bitsetSet(
            &accept,
            VelocityID
        );

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* updates position according to velocity */
void velocitySystem(Game *gamePtr, Scene *scenePtr){
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
        Velocity *velocityPtr
            = windQueryItrGetPtr(Velocity,
                &itr
            );
        positionPtr->pastPos = positionPtr->currentPos;
        positionPtr->currentPos = point2DAddPolar(
            positionPtr->currentPos,
            *velocityPtr
        );
        windQueryItrAdvance(&itr);
    }
}