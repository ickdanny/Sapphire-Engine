#ifndef VECS_ENTITYLIST_H
#define VECS_ENTITYLIST_H

#include "Constructure.h"

#include "_Vecs_EntityMetadata.h"

/* Stores metadata for all entities of an ECS world */
typedef struct _VecsEntityList{
    Array _entityMetadataArray;
    size_t _nextCreatedEntityId;
    size_t _numEntities;
} _VecsEntityList;

/*
 * Constructs and returns a new empty entity list
 * by value
 */
_VecsEntityList _vecsEntityListMake(
    size_t maxEntities
);

/* Clears all entity data from the given entity list */
void _vecsEntityListClear(
    _VecsEntityList *entityListPtr
);

/*
 * Allocates a new entity with no components and
 * returns it
 */
VecsEntity _vecsEntityListAllocate(
    _VecsEntityList *entityListPtr
);

/*
 * Reclaims an entity and puts its id back into the
 * pool; error if the entity is not alive
 */
void _vecsEntityListReclaim(
    _VecsEntityList *entityListPtr,
    VecsEntity toReclaim
);

/*
 * Returns true if the specified entity is alive,
 * false otherwise
 */
bool _vecsEntityListIsAlive(
    _VecsEntityList *entityListPtr,
    VecsEntity entity
);

/*
 * Returns true if the specified entity is dead,
 * false otherwise
 */
#define _vecsEntityListIsDead(entityListPtr, entity) \
    (!(_vecsEntityListIsAlive(entityListPtr, entity)))

/*
 * Returns true if an entity with the same id as the
 * provided entity is currently alive, false otherwise
 */
bool _vecsEntityListIsIdAlive(
    _VecsEntityList *entityListPtr,
    VecsEntity entityId
);

/*
 * Returns true if no entity with the specified id is
 * currently alive, false otherwise
 */
#define _vecsEntityListIsIdDead( \
    entityListPtr, \
    entity \
) \
    (!(_vecsEntityListIsIdAlive( \
        entityListPtr, \
        entity \
    )))

/*
 * Returns a pointer to the metadata for the specified
 * entity, or NULL if that entity is not alive
 */
_VecsEntityMetadata *_vecsEntityListGetMetadata(
    _VecsEntityList *entityListPtr,
    VecsEntity entity
);

/*
 * Returns a pointer to the metadata for the specified
 * entity id, or NULL if no such entity is alive
 */
_VecsEntityMetadata *_vecsEntityListIdGetMetadata(
    _VecsEntityList *entityListPtr,
    VecsEntity entityId
);

/*
 * Frees the memory associated with the given
 * entity list
 */
void _vecsEntityListFree(
    _VecsEntityList *entityListPtr
);

#endif