#ifndef NATIVEFUNCS_H
#define NATIVEFUNCS_H

#include "SystemCommon.h"

/*
 * Returns a pointer to the native func set used by
 * the game
 */
UNNativeFuncSet *getNativeFuncSet();

/* Sets the game pointer for native funcs */
void setGameForNativeFuncs(Game *gamePtr);

/* Sets the scene pointer for native funcs */
void setSceneForNativeFuncs(Scene *scenePtr);

/* Sets the entity for native funcs */
void setEntityForNativeFuncs(VecsEntity handle);

#endif