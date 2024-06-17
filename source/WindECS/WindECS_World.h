#ifndef WINDECS_WORLD_H
#define WINDECS_WORLD_H

#include "Constructure.h"

#include "WindECS_Entity.h"

typedef struct _WindEntityMetadata{
    /*
     * Each set bit in the bitset represents a
     * component type which the entity has
     */
    Bitset _componentSet;
    WindEntityGenerationType _generation;
} _WindEntityMetadata;

/*
 * An ECS world encapsulates all entity data and
 * provides access to various methods to interact
 * with said data
 */
typedef struct WindWorld{
    ArrayList _archetypeArrayList;
    ArrayList _queryArrayList;

    Bitset _currentEntityIDs;

    //todo: max num of components?
} WindWorld;

#endif