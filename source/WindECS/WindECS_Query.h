#ifndef WINDECS_QUERY_H
#define WINDECS_QUERY_H

#include "Constructure.h"

#include "_WindECS_Archetype.h"

/*
 * A Query provides access to a specific set Entities
 */
typedef struct WindQuery{
    /* 
     * Each set bit in the bitset represents a
     * component type which this query will require
     * entities have
     */
    Bitset _acceptComponentSet;
    /* 
     * Each set bit in the bitset represents a
     * component type which this query will require
     * entities not have
     */
    Bitset _rejectComponentSet;

    /* A weak pointer to the archetype list */
    ArrayList *_archetypeListPtr;

    /*
     * Stores a list of archetypes by index that fit
     * the query
     */
    ArrayList _archetypeIndexList;

    /* internal value for tracking modifications */
    size_t _modificationCount;

    /*
     * true if the query has a reject component set,
     * false otherwise 
     */
    bool _hasRejectComponentSet;
} WindQuery;

/*
 * Constructs and returns a new WindQuery with the
 * specified accept and reject component sets
 * (NULL can be passed for the reject set);
 * the new query makes copies of the passed bitsets;
 * iterates over the given archetype list and accepts
 * all fitting archetypes automatically
 */
WindQuery windQueryMake(
    Bitset *acceptComponentSetPtr,
    Bitset *rejectComponentSetPtr,
    ArrayList *archetypeListPtr
);

/*
 * Adds the specified archetype into the given query
 * if it fits the query restrictions, otherwise does
 * nothing; returns true if the archetype was accepted,
 * false otherwise
 */
bool windQueryTryAcceptArchetype(
    WindQuery *queryPtr,
    size_t archetypeIndex
);

/*
 * Frees all memory associated with the specified
 * WindQuery
 */
void windQueryFree(WindQuery *queryPtr);

/*
 * Iterates over the entities that fit a given query
 */
typedef struct WindQueryItr{
    /*
     * pointer to the query to iterate over, or
     * NULL to indicate that there are no entities
     * left
     */
    WindQuery *_queryPtr;
    /*
     * index of the current archetype being iterated
     * in the query's archetype index list (not the
     * global archetype list)
     */
    size_t _currentArchetypeIndex;
    /* the iterator over the current archetype */
    _WindArchetypeItr _archetypeItr;
    /*
     * a value used to detect when archetype has been
     * modified during iteration
     */
    size_t _storedModificationCount;
} WindQueryItr;

/* Returns an iterator over the specified query */
WindQueryItr windQueryItr(WindQuery *queryPtr);

/*
 * Returns true if the specified query iterator
 * has more elements, false otherwise
 */
bool windQueryItrHasEntity(WindQueryItr *itrPtr);

/*
 * Advances the query itr to point to the next entity
 */
void windQueryItrAdvance(WindQueryItr *itrPtr);

/*
 * Returns a pointer to the component specified by
 * the given componentID of the entity currently being
 * pointed to by the given query iterator; error
 * if the componentID is invalid; returns NULL if the
 * component is a marker
 */
void *_windQueryItrGetPtr(
    WindQueryItr *itrPtr,
    WindComponentIDType componentID
);

/*
 * Returns a pointer to the specified component of the
 * entity currently being pointed to by the given
 * query iterator; error if the query does not accept
 * the component in question; returns NULL if the
 * component is a marker
 */
#define windQueryItrGetPtr( \
    TYPENAME, \
    ITRPTR \
) \
    ((TYPENAME*)_windQueryItrGetPtr( \
        ITRPTR, \
        windComponentGetID(TYPENAME) \
    ))

/*
 * Returns the specified component of the entity
 * currently being pointed to by the given query
 * iterator; error if the query does not accept the
 * component in question; should not be used for
 * marker components
 */
#define windQueryItrGet( \
    TYPENAME, \
    ITRPTR \
) \
    ( \
        (*windQueryItrGetPtr( \
            TYPENAME, \
            ITRPTR \
        )) \
    )

/*
 * Returns the ID of the etity curently being pointed
 * to by the given query iterator; error if the query
 * is out of entities
 */
WindEntityIDType windQueryItrCurrentID(
    WindQueryItr *itrPtr
);

/*
 * neither the query itr nor the archetype itr need
 * to be freed
 */

#endif