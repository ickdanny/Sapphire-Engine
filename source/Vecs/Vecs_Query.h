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
} VecsQuery;

/*
 * Constructs and returns a new query with the
 * specified accept and reject component sets;
 * iterates over the given archetype list and accepts
 * all fitting archetypes automatically
 */
VecsQuery vecsQueryMake(
    VecsComponentSet acceptComponentSet,
    VecsComponentSet rejectComponentSet,
    ArrayList *archetypeListPtr
);

/*
 * Adds the specified archetype into the given query
 * if it fits the query restrictions, otherwise does
 * nothing; returns true if the archetype was accepted,
 * false otherwise
 */
bool _vecsQueryTryAcceptArchetype(
    VecsQuery *queryPtr,
    size_t archetypeIndex
);

/*
 * Frees all memory associated with the specified
 * query
 */
void vecsQueryFree(VecsQuery *queryPtr);

/*
 * Iterates over the entities that fit a given query
 */
typedef struct VecsQueryItr{
    VecsQuery *_queryPtr;
    /*
     * index of the current archetype being iterated
     * in the query's archetype index list (not the
     * global archetype list)
     */
    size_t _currentArchetypeIndex;

    /* the iterator over the current archetype */
    _VecsArchetypeItr _archetypeItr;

    size_t _storedModificationCount;
} VecsQueryItr;

/*
 * Returns true if the specified query iterator
 * has more elements, false otherwise
 */
bool vecsQueryItrHasEntity(VecsQueryItr *itrPtr);

/*
 * Advances the query itr to point to the next entity
 */
void vecsQueryItrAdvance(VecsQueryItr *itrPtr);

/*
 * Returns a pointer to the component specified by
 * the given component id of the entity currently being
 * pointed to by the given query iterator; error
 * if the component id is invalid; returns NULL if the
 * component is a marker
 */
void *_vecsQueryItrGetPtr(
    VecsQueryItr *itrPtr,
    VecsComponentId componentId
);

/*
 * Returns a pointer to the specified component of the
 * entity currently being pointed to by the given
 * query iterator; error if the query does not accept
 * the component in question; returns NULL if the
 * component is a marker
 */
#define vecsQueryItrGetPtr( \
    typeName, \
    itrPtr \
) \
    ((typeName*)_vecsQueryItrGetPtr( \
        itrPtr, \
        vecsComponentGetId(typeName) \
    ))

/*
 * Returns the specified component of the entity
 * currently being pointed to by the given query
 * iterator; error if the query does not accept the
 * component in question; should not be used for
 * marker components
 */
#define vecsQueryItrGet( \
    typeName, \
    itrPtr \
) \
    ( \
        (*vecsQueryItrGetPtr( \
            typeName, \
            itrPtr \
        )) \
    )

/* query itr does not need to be freed */

#endif