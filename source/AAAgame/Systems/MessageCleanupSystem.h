#ifndef MESSAGECLEANUPSYSTEM_H
#define MESSAGECLEANUPSYSTEM_H

#include "SystemCommon.h"

/* cleans up messages left over from the last update */
void messageCleanupSystem(
    Game *gamePtr,
    Scene *scenePtr
);

#endif