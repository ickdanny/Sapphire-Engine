#ifndef WINDECS_WORLD_H
#define WINDECS_WORLD_H

#include "Constructure.h"

#include "WindECS_Entity.h"
#include "_WindECS_Entities.h"

/*
 * An ECS world encapsulates all entity data and
 * provides access to various methods to interact
 * with said data
 */
typedef struct WindWorld{
    ArrayList _archetypeArrayList;
    ArrayList _queryArrayList;

    Bitset _currentEntityIDs;

    //todo: num of components?
} WindWorld;

#endif