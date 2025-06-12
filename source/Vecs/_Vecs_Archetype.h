#ifndef VECS_ARCHETYPE_H
#define VECS_ARCHETYPE_H

#include "Constructure.h"

#include "_Vecs_EntityList.h"
#include "Vecs_ComponentList.h"

/*
 * An archetype stores the component data for all the
 * entities with a specific set of components
 */
typedef struct _VecsArchetype{
    /*
     * defines set of components for entities stored
     * in this archetype
     */
    VecsComponentSet _componentSet;

    /*
     * C array of arraylists for storing component data
     * indexed by component id
     */
    ArrayList _componentStorageLists[
        vecsMaxNumComponents
    ];

    /* pointer to the component metadata list */
    VecsComponentList *_componentListPtr;

    /* pointer to the entity metadata list */
    _VecsEntityList *_entityListPtr;

    /* internal value for tracking modifications */
    size_t _modificationCount;
} _VecsArchetype;

/*
 * Constructs and returns a new archetype by value
 */
_VecsArchetype _vecsArchetypeMake(
    VecsComponentSet componentSet,
    size_t initEntityCapacity,
    VecsComponentList *componentListPtr,
    _VecsEntityList *entityListPtr
);

/*
 * Clears all component data from the specified
 * archetype
 */
void _vecsArchetypeClear(_VecsArchetype *archetypePtr);

/*
 * Errors if the specified component id is invalid for
 * the given archetype
 */
void _vecsArchetypeErrorIfBadComponent(
    _VecsArchetype *archetypePtr,
    VecsComponentId componentId
);

/*
 * Errors if the specified entity id is not within the
 * given archetype
 */
void _vecsArchetypeErrorIfBadEntity(
    _VecsArchetype *archetypePtr,
    VecsEntity entity
);

/*
 * Returns a pointer to the component specified by
 * the given component id of the specified entity;
 * error if the component or the entity is invalid;
 * returns NULL if the component is a marker
 */
void *__vecsArchetypeGetPtr(
    _VecsArchetype *archetypePtr,
    VecsComponentId componentId,
    VecsEntity entity
);

/*
 * Returns a pointer to the specified component of the
 * entity identified by the given entity id; error
 * if the entity is invalid or if the specified
 * archetype does not hold the component in question;
 * returns NULL if the component is a marker
 */
#define _vecsArchetypeGetPtr( \
    typename, \
    archetypePtr, \
    entity \
) \
    ((typename*)__vecsArchetypeGetPtr( \
        archetypePtr, \
        vecsComponentGetId(typename), \
        entity \
    ))

/*
 * Returns the specified component of the entity
 * identified by the given entity id by value; error
 * if the entity is invalid or if the specified
 * archetype does not hold the component in question;
 * should not be used for marker components
 */
#define _vecsArchetypeGet( \
    typename, \
    archetypePtr, \
    entity \
) \
    ( \
        (typename) \
        (*_vecsArchetypeGetPtr( \
            typename, \
            archetypePtr, \
            entity \
        )) \
    )

//todo: probably need an interface for allocating space
//for a new entity i.e. index

/*
 * Sets the component specified by the given
 * component id of the entity specified by the given
 * entity id to the value stored in the given void ptr;
 * error if the component id is invalid; does nothing
 * if NULL is passed or if the component is a marker;
 * assumes entity index has already been assigned
 */
void __vecsArchetypeSetPtr(
    _VecsArchetype *archetypePtr,
    VecsComponentId componentId,
    VecsEntity entity,
    void *componentPtr
);

/*
 * Sets the specified component of the entity
 * specified by the given entity id to the value stored
 * in the given void ptr; error if the specified
 * archetype does not hold the component in question;
 * does nothing if NULL is passed or if the component
 * is a marker; assumes entity index has already been
 * assigned
 */
#define _vecsArchetypeSetPtr( \
    typename, \
    archetypePtr, \
    entity, \
    componentPtr \
) \
    _Generic(*componentPtr, \
        typename: __vecsArchetypeSetPtr( \
            archetypePtr, \
            vecsComponentGetId(typename), \
            entity, \
            componentPtr \
        ) \
    )

/*
 * Sets the specified component of the entity
 * specified by the given entity id to the given
 * component; error if the specified archetype does
 * not hold the component in question; should not be
 * used for marker components; assumes entity index
 * has already been assigned
 */
#define _vecsArchetypeSet( \
    typename, \
    archetypePtr, \
    entityId, \
    component \
) \
    do{ \
        vecsComponentId componentId \
            = vecsComponentGetId(typename); \
        _vecsArchetypeErrorIfBadComponent( \
            archetypePtr, \
            componentId \
        ); \
        _vecsArchetypeErrorIfBadEntity( \
            archetypePtr, \
            entity \
        ); \
        VecsComponentMetadata componentMetadata \
            = vecsComponentListGetMetadata( \
                archetypePtr->_componentListPtr, \
                componentId \
            ); \
        assertFalse( \
            componentMetadata._componentSize == 0, \
            "cannot use archetype set on a marker; " \
            SRC_LOCATION \
        }; \
        assertTrue( \
            componentMetadata._componentSize \
                == sizeof(typename), \
            "mismatching component size between " \
            "metadata and sizeof; " \
            SRC_LOCATION \
        ); \
        _VecsEntityMetadata *entityMetadataPtr \
            = _vecsEntityListGetMetadata( \
                archetypePtr->_entityListPtr, \
                entity \
            ); \
        size_t index \
            = entityMetadataPtr->_indexInArchetype; \
        ArrayList *componentListPtr \
            = &(archetypePtr->_componentStorageLists \
                [componentId]); \
        void *componentSlotPtr = arrayListGetPtr( \
            typename, \
            componentListPtr, \
            index \
        ); \
        if(vecsComponentSetContainsId( \
            entityMetadataPtr \
                ->_initializedComponentSet, \
            componentId \
        )){ \
            if(componentMetadata._destructor){ \
                componentMetadata._destructor( \
                    componentSlotPtr \
                ); \
            } \
        } \
        else{ \
            entityMetadataPtr \
            ->_initializedComponentSet \
                = vecsComponentSetAddId( \
                    entityMetadataPtr \
                        ->_initializedComponentSet, \
                    componentId \
                ); \
        } \
        arrayListSet( \
            typename, \
            componentListPtr, \
            index, \
            component \
        ); \
        ++(archetypePtr->_modificationCount); \
    } while(false)

/*
 * Moves the entity identified by the given entity id
 * to the specified other archetype; error if the
 * same archetype is provided or if the entity is
 * invalid
 */
void _vecsArchetypeMoveEntity(
    _VecsArchetype *srcArchetypePtr,
    _VecsArchetype *destArchetypePtr,
    VecsEntity entity
);

/*
 * Removes the entity identified by the given entity
 * id; returns true if entity successfully removed,
 * false if entity was not originally in the archetype
 */
bool _vecsArchetypeRemoveEntity(
    _VecsArchetype *archetypePtr,
    VecsEntity entity
);

/*
 * Frees the memory associated with the given
 * archetype
 */
void _vecsArchetypeFree(_VecsArchetype *archetypePtr);

/*
 * Iterates over the entities of a specific archetype;
 * to get the entity being iterated over, use the
 * built-in VecsEntity component (i.e. component id 0)
 */
typedef struct _VecsArchetypeItr{
    /* pointer to the archetype to iterate over */
    _VecsArchetype *_archetypePtr;

    /* current index into the component lists */
    size_t _currentIndex;

    /*
     * a value used to detect when archetype has been
     * modified during iteration
     */
    size_t _storedModificationCount;
} _VecsArchetypeItr;

/* Returns an iterator over the specified archetype */
_VecsArchetypeItr _vecsArchetypeItr(
    _VecsArchetype *archetypePtr
);

/*
 * Returns true if the specified archetype iterator
 * has more elements, false otherwise
 */
bool _vecsArchetypeItrHasEntity(
    _VecsArchetypeItr *itrPtr
);

/*
 * Advances the specified archetype itr to point to the
 * next entity
 */
void _vecsArchetypeItrAdvance(
    _VecsArchetypeItr *itrPtr
);

/*
 * Returns a pointer to the component specified by
 * the given component id of the entity currently being
 * pointed to by the given archetype iterator; error
 * if the component id is invalid; returns NULL if the
 * component is a marker
 */
void *__vecsArchetypeItrGetPtr(
    _VecsArchetypeItr *itrPtr,
    VecsComponentId componentId
);

/*
 * Returns a pointer to the specified component of the
 * entity currently being pointed to by the given
 * archetype iterator; error if the archetype does not
 * hold the component in question; returns NULL if the
 * component is a marker
 */
#define _vecsArchetypeItrGetPtr( \
    typename, \
    itrPtr \
) \
    ((typename*)__vecsArchetypeItrGetPtr( \
        itrPtr, \
        vecsComponentGetId(typename) \
    ))

/*
 * Returns the specified component of the entity
 * currently being pointed to by the given archetype
 * iterator; error if the archetype does not hold the
 * component in question; should not be used for
 * marker components
 */
#define _vecsArchetypeItrGet( \
    typename, \
    itrPtr \
) \
    ( \
        (typename) \
        (*_vecsArchetypeItrGetPtr( \
            typename, \
            itrPtr \
        )) \
    )

/* the archetype itr does not need to be freed */

#endif
