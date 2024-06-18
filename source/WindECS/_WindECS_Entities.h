#ifndef WINDECS_ENTITYMETADATA_H
#define WINDECS_ENTITYMETADATA_H

#include "Constructure.h"

#include "WindECS_Entity.h"

/* Represents the state of a single entity */
typedef struct _WindEntityMetadata{
    /*
     * Each set bit in the bitset represents a
     * component type which the entity has
     */
    Bitset _componentSet;
    WindEntityGenerationType _generation;
} _WindEntityMetadata;

/*
 * Stores data pertaining to the state of all entities;
 * the maximum number of entities is set during
 * construction and cannot be exceeded
 */
typedef struct _WindEntities{
    /*
     * A bitset representing the current entity IDs
     * in use where a set bit means in use
     */
    Bitset _currentEntityIDs;
    /*
     * an array of entity metadata indexed by entity ID
     */
    Array _entityMetadata;

    WindEntityIDType _nextCreatedEntityID;
    WindEntityIDType _numEntities;
} _WindEntities;

/*
 * Constructs and returns a new _WindEntities by value
 */
_WindEntities _windEntitiesMake(
    WindEntityIDType numEntityIDs
);

/* Creates a new entity and returns its handle */
WindEntity _windEntitiesCreate(
    _WindEntities *entitiesPtr
);

/*
 * Reclaims an entity and puts its ID back into the
 * pool
 */
void _windEntitiesReclaim(
    _WindEntities *entitiesPtr,
    WindEntity toReclaim
);

/*
 * Returns true if the specified entity is alive,
 * false otherwise
 */
bool _windEntitiesIsAlive(
    _WindEntities *entitiesPtr,
    WindEntity entity
);

/*
 * Returns true if the specified entity is dead,
 * false otherwise
 */
#define _windEntitiesIsDead(ENTITIESPTR, ENTITY) \
    (!(_windEntitiesIsAlive(ENTITIESPTR, ENTITY)))

/*
 * Returns a pointer to the metadata for the specified
 * entity, or NULL if that entity is not alive
 */
_WindEntityMetadata *_windEntitiesGetMetadata(
    _WindEntities *entitiesPtr,
    WindEntity entity
);

#endif