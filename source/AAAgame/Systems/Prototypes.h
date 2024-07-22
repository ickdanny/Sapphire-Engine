#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "SystemCommon.h"

/*
 * Applies the specified prototype to the given
 * component list array; also applies the depth
 * offset when creating the sprite instruction
 */
void applyPrototype(
    Game *gamePtr,
    Scene *scenePtr,
    String *prototypeIDPtr,
    ArrayList *componentListPtr,
    int depthOffset
);

#endif