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
    /* internal value for tracking modifications */
    size_t _modificationCount;
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
 * Errors if the componentID is invalid for the
 * specified archetype
 */
void __windArchetypeErrorIfBadComponentID(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID
);

/*
 * Errors if the entityID is invalid for the specified
 * sparse set of the given component type
 */
void __windArchetypeErrorIfBadEntityID(
    SparseSet *componentStoragePtr,
    WindComponentMetadata componentMetadata,
    WindEntityIDType entityID
);

/*
 * Returns a pointer to the component specified by
 * the given componentID of the entity specified by
 * the given entityID; error if the componentID or the
 * entityID is invalid; returns NULL if the component
 * is a marker
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
 * archetype does not hold the component in question;
 * returns NULL if the component is a marker
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
 * archetype does not hold the component in question;
 * should not be used for marker components
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
 * error if the componentID is invalid; does nothing
 * if NULL is passed
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
 * in the given void ptr; error if the specified
 * archetype does not hold the component in question;
 * does nothing if NULL is passed
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

/*
 * Sets the specified component of the entity
 * specified by the given entityID to the given
 * component; error if the specified archetype does
 * not hold the component in question; should not be
 * used for marker components
 */
#define _windArchetypeSet( \
    TYPENAME, \
    ARCHETYPEPTR, \
    ENTITYID, \
    COMPONENT \
) \
    do{ \
        windComponentIDType componentID \
            = windComponentGetID(TYPENAME); \
        __windArchetypeErrorIfBadComponentID( \
            (ARCHETYPEPTR), \
            componentID \
        ); \
        SparseSet *componentStoragePtr = arrayGetPtr( \
            SparseSet, \
            &((ARCHETYPEPTR) \
                ->_componentStorageArray), \
            componentID \
        ); \
        WindComponentMetadata componentMetadata \
            = windComponentsGet( \
                (ARCHETYPEPTR)->_componentsPtr, \
                componentID \
            ); \
        if(componentMetadata._destructor){ \
            if(sparseSetContains(TYPENAME, \
                componentStoragePtr, \
                (ENTITYID) \
            )){ \
                void *oldComponentPtr \
                    = sparseSetGetPtr(TYPENAME, \
                        componentStoragePtr, \
                        (ENTITYID) \
                    ); \
                componentMetadata._destructor( \
                    oldComponentPtr \
                ); \
            } \
        } \
        sparseSetSet(TYPENAME, \
            componentStoragePtr, \
            (ENITTYID), \
            (COMPONENT) \
        ); \
        ++(archetypePtr->_modificationCount); \
    } while(false)
    
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
 * returns true if entity successfully removed, false
 * if entity was not originally in the archetype
 */
bool _windArchetypeRemoveEntity(
    _WindArchetype *archetypePtr,
    WindEntityIDType entityID
);

/*
 * Frees the memory associated with the given
 * Archetype
 */
void _windArchetypeFree(_WindArchetype *archetypePtr);

//todo: archetype itr
/*
 * Iterates over the entities of a specific Archetype
 */
typedef struct _WindArchetypeItr{
    /* pointer to the archetype to iterate over */
    _WindArchetype *_archetypePtr;
    /*
     * current index into the dense arrays in the
     * compnoent storages
     */
    size_t _currentIndex;
    /*
     * a value used to detect when archetype has been
     * modified during iteration
     */
    size_t _storedModificationCount;
    /*
     * the index of a real component present
     * in the archetype; iteration does not work
     * if the archetype only has markers and this value
     * will be set to SIZE_MAX in that case
     */
    size_t _presentComponentIndex;
} _WindArchetypeItr;

/* Returns an iterator over the specified archetype */
_WindArchetypeItr _windArchetypeItr(
    _WindArchetype *archetypePtr
);

/*
 * Returns true if the specified archetype iterator
 * has more elements, false otherwise
 */
bool _windArchetypeItrHasEntity(
    _WindArchetypeItr *itrPtr
);

/*
 * Advances the archetype itr to point to the next
 * entity
 */
void _windArchetypeItrAdvance(
    _WindArchetypeItr *itrPtr
);

/*
 * Returns a pointer to the component specified by
 * the given componentID of the entity currently being
 * pointed to by the given archetype iterator; error
 * if the componentID is invalid; returns NULL if the
 * component is a marker
 */
void *__windArchetypeItrGetPtr(
    _WindArchetypeItr *itrPtr,
    WindComponentIDType componentID
);

/*
 * Returns a pointer to the specified component of the
 * entity currently being pointed to by the given
 * archetype iterator; error if the archetype does not
 * hold the component in question; returns NULL if the
 * component is a marker
 */
#define _windArchetypeItrGetPtr( \
    TYPENAME, \
    ITRPTR \
) \
    ((TYPENAME*)__windArchetypeItrGetPtr( \
        ITRPTR, \
        windComponentGetID(TYPENAME) \
    ))

/*
 * Returns the specified component of the entity
 * currently being pointed to by the given archetype
 * iterator; error if the archetype does not hold the
 * component in question; should not be used for
 * marker components
 */
#define _windArchetypeItrGet( \
    TYPENAME, \
    ITRPTR \
) \
    ( \
        (TYPENAME) \
        (*_windArchetypeItrGetPtr( \
            TYPENAME, \
            ITRPTR \
        )) \
    )

#endif