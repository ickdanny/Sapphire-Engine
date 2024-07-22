#ifndef PLAYERDEATHSYSTEM_H
#define PLAYERDEATHSYSTEM_H

#include "SystemCommon.h"

/*
 * Detects when a player dies based on state changes
 * and publishes the corresponding death message
 */
void playerDeathSystem(Game *gamePtr, Scene *scenePtr);

#endif