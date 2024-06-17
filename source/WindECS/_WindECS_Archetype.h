#ifndef WINDECS_ARCHETYPE_H
#define WINDECS_ARCHETYPE_H

#include "Constructure.h"

/*
 * An archetype stores all the entities which have
 * a specific set of components
 */
typedef struct _WindArchetype{
    /*
     * Each set bit in the bitset represents a
     * component type which is present in this
     * Archetype
     */
    Bitset _componentSet;
    /*
     * componentID indexes the sparse set for that
     * component
     */
    Array _componentSparseSetArray;
} _WindArchetype;

//todo archetype itr
typedef struct _WindArchetypeItr{

} _WindArchetypeItr;

#endif