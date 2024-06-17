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
    WindEntityGenerationType _generation;
} WindEntity;

#endif