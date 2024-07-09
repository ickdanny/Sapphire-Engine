#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "WindECS.h"
#include "ZMath.h"

/*
 * Allocates and returns a new WindComponents object
 * containing the RTTI details of every component
 * used in the game engine
 */
WindComponents *componentsMake();

typedef struct Position{
    Point2D currentPos;
    Point2D pastPos;
} Position;

/* each component needs TYPENAME##ID defined */
typedef enum ComponentID{
    PositionID,
    numComponents,
} ComponentID;

#endif