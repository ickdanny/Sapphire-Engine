#ifndef VECS_QUERY_H
#define VECS_QUERY_H

#include "Constructure.h"

#include "_Vecs_Archetype.h"

/*
 * A Query provides access to a specific subset of
 * entities in an ECS world based on component presence
 */
typedef struct VecsQuery{
    VecsComponentSet _acceptComponentSet;
    VecsComponentSet _rejectComponentSet;

    /* a pointer to the entire archetype list */
    ArrayList *_archetypeListPtr;

    /* stores a list of archetypes by index */
    ArrayList _archetypeIndexList;

    /* internal value for tracking modifications */
    size_t _modificationCount;

    //todo: hasrejectcomponentset can just compare to 0
} VecsQuery;

#endif