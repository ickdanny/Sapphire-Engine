#ifndef WINDECS_ENTITY_H
#define WINDECS_ENTITY_H

#include <stdint.h>

typedef uint32_t WindEntityGenerationType;
typedef uint32_t WindEntityIDType;

/*
 * An Entity is a small type holding information 
 * needed to retrieve components associated with
 * itself
 */
typedef struct WindEntity{
    WindEntityIDType entityID;
    /* a generation cannot be 0 */
    WindEntityGenerationType _generation;
} WindEntity;

/*
 * Returns true if the two specified entities are
 * equal, false otherwise
 */
#define windEntityEquals(LEFT, RIGHT) \
    (((LEFT).entityID == (RIGHT).entityID) \
        && ((LEFT)._generation == (RIGHT)._generation))

#endif