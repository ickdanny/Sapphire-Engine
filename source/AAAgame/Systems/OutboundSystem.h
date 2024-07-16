#ifndef OUTBOUNDSYSTEM_H
#define OUTBOUNDSYSTEM_H

#include "SystemCommon.h"

/*
 * Removes entities which are outside of a certain
 * boundary from the edge of the game
 */
void outboundSystem(Game *gamePtr, Scene *scenePtr);

#endif