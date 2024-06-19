#ifndef WINDECS_ARCHETYPE_H
#define WINDECS_ARCHETYPE_H

#include "Constructure.h"

#include "WindECS_Entity.h"
#include "WindECS_Components.h"

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
    Array _componentStorageArray;
    /* Stored (weak) pointer to the component RTTI */
    WindComponents *_componentsPtr;
} _WindArchetype;

/*
 * Constructs and returns a new _WindArchetype by
 * value; the archetype will make a copy of the
 * component set
 */
_WindArchetype _windArchetypeMake(
    const Bitset *componentSetPtr,
    WindEntityIDType numEntities,
    WindComponents *componentsPtr
);

/*
 * Returns a pointer to the component specified by
 * the given componentID of the entity specified by
 * the given entityID; error if the componentID or the
 * entityID is invalid
 */
void *__windArchetypeGetPtr(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
);

/*
 * Returns a pointer to the specified component of the
 * entity identified by the given entityID; error
 * if the entityID is invalid or if the specified
 * archetype does not hold the component in question
 */
#define _windArchetypeGetPtr( \
    TYPENAME, \
    ARCHETYPEPTR, \
    ENTITYID \
) \
    ((TYPENAME*)__windArchetypeGetPtr( \
        ARCHETYPEPTR, \
        windComponentGetID(TYPENAME), \
        ENTITYID \
    ))

/*
 * Returns the specified component of the entity
 * identified by the given entityID by value; error
 * if the entityID is invalid or if the specified
 * archetype does not hold the component in question
 */
#define _windArchetypeGet( \
    TYPENAME, \
    ARCHETYPEPTR, \
    ENTITYID \
) \
    ( \
        (TYPENAME) \
        (*_windArchetypeGetPtr( \
            TYPENAME, \
            ARCHETYPEPTR, \
            ENTITYID \
        )) \
    )

/*
 * Sets the component specified by the given
 * componentID of the entity specified by the given
 * entityID to the value stored in the given void ptr;
 * error if the componentID or the entityID is invalid
 */
void __windArchetypeSetPtr(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID,
    void *componentPtr
);

/*
 * Sets the specified component of the entity
 * specified by the given entityID to the value stored
 * in the given void ptr; error if the entityID is
 * invalid or if the specified archetype does not hold
 * the component in question
 */
#define _windArchetypeSetPtr( \
    TYPENAME, \
    ARCHETYPEPTR, \
    ENTITYID, \
    COMPONENTPTR \
) \
    _Generic(*COMPONENTPTR, \
        TYPENAME: __windArchetypeSetPtr( \
            ARCHETYPEPTR, \
            windComponentGetID(TYPENAME), \
            ENTITYID, \
            COMPONENTPTR \
        ) \
    )

/*
 * Archetype setting must be done via macro because
 * it expects values not pointers
 */
//todo value set

//todo for set component remember to run destructor

/*
 * Moves the entity identified by the given entityID
 * to the specified other archetype; error if the
 * same archetype is provided or if the entityID is
 * invalid
 */
void _windArchetypeMoveEntity(
    _WindArchetype *archetypePtr,
    WindEntityIDType entityID,
    _WindArchetype *newArchetypePtr
);

/*
 * Removes the entity identified by the given entityID;
 * error if the entityID is invalid
 */
void _windArchetypeRemoveEntity(
    _WindArchetype *archetypePtr,
    WindEntityIDType entityID
);

/*
 * Iterates over the entities of a specific Archetype
 */
typedef struct _WindArchetypeItr{
    //todo archetype itr
} _WindArchetypeItr;

#endif