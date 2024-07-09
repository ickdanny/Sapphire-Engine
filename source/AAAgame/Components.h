#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "WindECS.h"

/*
 * Allocates and returns a new WindComponents object
 * containing the RTTI details of every component
 * used in the game engine
 */
WindComponents *componentsMake();

//todo: think components needs some macros
//each component needs TYPENAME##ID defined

#endif