#include "VelocitySystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(PositionId)
    | vecsComponentSetFromId(VelocityId);

/* updates position according to velocity */
void velocitySystem(Game *gamePtr, Scene *scenePtr){
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
        Velocity *velocityPtr = vecsQueryItrGetPtr(
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