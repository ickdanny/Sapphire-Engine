#ifndef COLLISIONDETECTIONSYSTEM_H
#define COLLISIONDETECTIONSYSTEM_H

#include "SystemCommon.h"

/*
 * detects collisions between sources and targets of
 * the same collision type
 */
void collisionDetectionSystem(
    Game *gamePtr,
    Scene *scenePtr
);

#endif