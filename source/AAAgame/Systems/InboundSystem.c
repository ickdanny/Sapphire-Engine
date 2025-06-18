#include "InboundSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(PositionId)
    | vecsComponentSetFromId(InboundId);

/*
 * Transforms a point to be within the specified
 * boundary from the outside of the game field
 */
static void inboundPoint(
    Point2D *pointPtr,
    Inbound bound
){
	float lowXBound = bound + config_gameOffsetX;
    float lowYBound = bound + config_gameOffsetY;
	float highXBound = config_gameWidth - bound
        + config_gameOffsetX;
	float highYBound = config_gameHeight - bound
        + config_gameOffsetY;
	if (pointPtr->x < lowXBound) {
		pointPtr->x = lowXBound;
	}
	else if (pointPtr->x > highXBound) {
		pointPtr->x = highXBound;
	}
	if (pointPtr->y < lowYBound) {
		pointPtr->y = lowYBound;
	}
	else if (pointPtr->y > highYBound) {
		pointPtr->y = highYBound;
	}
}

/* Prevents entities from leaving a certain boumdary */
void inboundSystem(Game *gamePtr, Scene *scenePtr){
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
        Inbound bound = vecsQueryItrGet(
            Inbound,
            &itr
        );

        inboundPoint(
            &(positionPtr->currentPos),
            bound
        );
        
        vecsQueryItrAdvance(&itr);
    }
}