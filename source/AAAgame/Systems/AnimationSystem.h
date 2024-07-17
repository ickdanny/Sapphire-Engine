#ifndef ANIMATIONSYSTEM_H
#define ANIMATIONSYSTEM_H

#include "SystemCommon.h"

/*
 * Handles animating entity graphics by looping through
 * frames and switching animations depending on
 * movement direction
 */
void animationSystem(Game *gamePtr, Scene *scenePtr);

#endif