#ifndef VECS_WORLD_H
#define VECS_WORLD_H

#include "Constructure.h"

#include "Vecs_Entity.h"
#include "_Vecs_EntityList.h"
#include "Vecs_ComponentList.h"
#include "Vecs_Query.h"

/*
 * An ECS world encapsulates all entity data and
 * provides access to various methods to interact
 * with said data; should not be moved once the ECS
 * begins running
 */
typedef struct VecsWorld{
    /* an unordered list of archetypes */
    ArrayList _archetypeList;
    /* an unordered list of queries */
    ArrayList _queryList;
    /*
     * a map of component sets to indices into the
     * archetype list
     */
    HashMap _archetypeIndexMap;
    /*
     * a map of component set pairs to indices into the
     * query list
     */
    HashMap _queryIndexMap;

    _VecsEntityList _entityList;
    VecsComponentList *_componentListPtr;

    ArrayList _addComponentQueue;
    ArrayList _setComponentQueue;
    ArrayList _removeComponentQueue;
    ArrayList _addEntityQueue;
    ArrayList _removeEntityQueue;
} VecsWorld;

/*
 * Constructs and returns a new ECS world by value;
 * does not take ownership of the given component list
 */
VecsWorld vecsWorldMake(
    size_t entityCapacity,
    VecsComponentList *componentListPtr
);

/* Clears all state of the specified ECS world */
void vecsWorldClear(VecsWorld *worldPtr);

/*
 * Returns a new query iterator; error if the accept
 * and reject sets intersect
 */
VecsQueryItr vecsWorldRequestQueryItr(
    VecsWorld *worldPtr,
    VecsComponentSet acceptComponentSet,
    VecsComponentSet rejectComponentSet
);

/*
 * Gets the entity specified by the given id; error if
 * no such entity is currently live
 */
VecsEntity vecsWorldGetEntityById(
    VecsWorld *worldPtr,
    VecsEntity entityId
);

/*
 * Returns true if the given entity is live, false
 * otherwise
 */
bool vecsWorldIsEntityLive(
    VecsWorld *worldPtr,
    VecsEntity entity
);

/*
 * Returns true if the given entity is dead, false
 * otherwise
 */
#define vecsWorldIsEntityDead(worldPtr, entity) \
    (!(vecsWorldIsEntityLive(worldPtr, entity)))

/*
 * Returns true if the entity described by the given
 * id is live (ignoring generation), false otherwise
 */
bool vecsWorldIsIdLive(
    VecsWorld *worldPtr,
    VecsEntity entityId
);

/*
 * Returns true if the entity described by the given
 * id is dead (ignoring generation), false otherwise
 */
#define vecsWorldIsIdDead(worldPtr, entityId) \
    (!(vecsWorldIsIdLive(worldPtr, entityId)))

/*
 * Returns true if the given entity contains a
 * component of the specified id, false otherwise
 * (including if entity is dead)
 */
bool _vecsWorldEntityContainsComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity
);

/*
 * Returns true if the given entity contains a
 * component of the specified type, false otherwise
 * (including if entity is dead)
 */
#define vecsWorldEntityContainsComponent( \
    typeName, \
    worldPtr, \
    entity \
) \
    (_vecsWorldEntityContainsComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity \
    ))

/*
 * Returns true if the entity described by the given
 * id contains a component of the specified id, false
 * otherwise (including if entity is dead)
 */
bool _vecsWorldIdContainsComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entityId
);

/*
 * Returns true if the entity described by the given
 * id contains a component of the specified type,
 * false otherwise (including if entity is dead)
 */
#define vecsWorldIdContainsComponent( \
    typeName, \
    worldPtr, \
    entityId \
) \
    (_vecsWorldIdContainsComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entityId \
    ))

/*
 * Returns a pointer to the component specified by the
 * given component id of the given entity; error if the
 * component id is invalid; returns NULL if the
 * component is a marker or if the entity is dead
 */
void *_vecsWorldEntityGetPtr(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity
);

/*
 * Returns a pointer to the component of the specified
 * type of the given entity; error if the component
 * is invalid; returns NULL if the component is a
 * marker or if the entity is dead
 */
#define vecsWorldEntityGetPtr( \
    typeName, \
    worldPtr, \
    entity \
) \
    ((typeName *)_vecsWorldEntityGetPtr( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity \
    ))

/*
 * Returns the component of the specified type of the
 * given entity; error if the component is invalid;
 * undefined behavior if the component is a marker or
 * if the entity is dead
 */
#define vecsWorldEntityGet( \
    typeName, \
    worldPtr, \
    entity \
) \
    (*vecsWorldEntityGetPtr( \
        typeName, \
        worldPtr, \
        entity \
    ))

/*
 * Returns a pointer to the component specified by the
 * given component id of the entity specified by the
 * given id; error if the component id is invalid;
 * returns NULL if the component is a marker or if the
 * entity is dead
 */
void *_vecsWorldIdGetPtr(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entityId
);

/*
 * Returns a pointer to the component of the specified
 * type of the entity specified by the given id;
 * error if the component is invalid; returns NULL
 * if the component is a marker or if the entity is
 * dead
 */
#define vecsWorldIdGetPtr( \
    typeName, \
    worldPtr, \
    entityId \
) \
    ((typeName *)_vecsWorldIdGetPtr( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entityId \
    ))

/*
 * Returns the component of the specified type of the
 * entity specified by the given id; error if the
 * component is invalid; undefined behavior if the
 * component is a marker or if the entity is dead
 */
#define vecsWorldIdGet( \
    typeName, \
    worldPtr, \
    entityId \
) \
    (*vecsWorldIdGetPtr( \
        typeName, \
        worldPtr, \
        entityId \
    ))

/*
 * Adds the given component to the specified entity,
 * returns true if successful, false otherwise; the
 * componentPtr is shallow copied and is not freed by
 * the ECS
 */
bool _vecsWorldEntityAddComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity,
    void *componentPtr
);

/*
 * Adds the given component to the specified entity,
 * returns true if successful, false otherwise; the
 * componentPtr is shallow copied and is not freed by
 * the ECS
 */
#define vecsWorldEntityAddComponent( \
    typeName, \
    worldPtr, \
    entity, \
    componentPtr \
) \
    (_vecsWorldEntityAddComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity, \
        componentPtr \
    ))

/*
 * Adds the given component to the entity specified
 * by the given id, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
bool _vecsWorldIdAddComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entityId,
    void *componentPtr
);

/*
 * Adds the given component to the entity specified
 * by the given id, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
#define vecsWorldIdAddComponent( \
    typeName, \
    worldPtr, \
    entityId, \
    componentPtr \
) \
    (_vecsWorldIdAddComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entityId, \
        componentPtr \
    ))

/*
 * Queues an order to add the given component to the
 * specified entity, returns true if successful, false
 * otherwise (e.g. the entity is dead); the
 * componentPtr is shallow copied to the heap and the
 * original pointer is not freed by the ECS
 */
bool _vecsWorldEntityQueueAddComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity,
    void *componentPtr
);

/*
 * Queues an order to add the given component to the
 * specified entity, returns true if successful, false
 * otherwise (e.g. the entity is dead); the
 * componentPtr is shallow copied to the heap and the
 * original pointer is not freed by the ECS
 */
#define vecsWorldEntityQueueAddComponent( \
    typeName, \
    worldPtr, \
    entity, \
    componentPtr \
) \
    (_vecsWorldEntityQueueAddComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity, \
        componentPtr \
    ))

/*
 * Queues an order to add the given component to the
 * entity currently specified by the given id, error
 * if the entity is dead; the componentPtr is shallow
 * copied to the heap and the original pointer is not
 * freed by the ECS
 */
#define vecsWorldIdQueueAddComponent( \
    typeName, \
    worldPtr, \
    entityId, \
    componentPtr \
) \
    (_vecsWorldEntityQueueAddComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        vecsWorldGetEntityById(worldPtr, entityId), \
        componentPtr \
    ))

/*
 * Sets the given component to the specified entity,
 * returns true if successful, false otherwise; the
 * componentPtr is shallow copied and is not freed by
 * the ECS
 */
bool _vecsWorldEntitySetComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity,
    void *componentPtr
);

/*
 * Sets the given component to the specified entity,
 * returns true if successful, false otherwise; the
 * componentPtr is shallow copied and is not freed by
 * the ECS
 */
#define vecsWorldEntitySetComponent( \
    typeName, \
    worldPtr, \
    entity, \
    componentPtr \
) \
    (_vecsWorldEntitySetComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity, \
        componentPtr \
    ))

/*
 * Sets the given component to the entity specified
 * by the given id, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
bool _vecsWorldIdSetComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entityId,
    void *componentPtr
);

/*
 * Sets the given component to the entity specified
 * by the given id, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
#define vecsWorldIdSetComponent( \
    typeName, \
    worldPtr, \
    entityId, \
    componentPtr \
) \
    (_vecsWorldIdSetComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entityId, \
        componentPtr \
    ))

/*
 * Queues an order to set the given component of the
 * specified entity to the provided value, returns true
 * if successful, false otherwise (e.g. the entity is
 * dead); the componentPtr is shallow copied to the
 * heap and the original pointer is not freed by the
 * ECS
 */
bool _vecsWorldEntityQueueSetComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity,
    void *componentPtr
);

/*
 * Queues an order to set the given component of the
 * specified entity to the provided value, returns true
 * if successful, false otherwise (e.g. the entity is
 * dead); the componentPtr is shallow copied to the
 * heap and the original pointer is not freed by the
 * ECS
 */
#define vecsWorldEntityQueueSetComponent( \
    typeName, \
    worldPtr, \
    entity, \
    componentPtr \
) \
    (_vecsWorldEntityQueueSetComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity, \
        componentPtr \
    ))

/*
 * Queues an order to set the given component of the
 * entity currently specified by the given id to the
 * provided value, error if the entity is dead; the
 * componentPtr is shallow copied to the heap and the
 * original pointer is not freed by the ECS
 */
#define vecsWorldIdQueueSetComponent( \
    typeName, \
    worldPtr, \
    entityId, \
    componentPtr \
) \
    (_vecsWorldEntityQueueSetComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        vecsWorldGetEntityById(worldPtr, entityId), \
        componentPtr \
    ))

/*
 * Removes the specified component from the given
 * entity, returns true if successful, false otherwise;
 * cannot remove entity id component
 */
bool _vecsWorldEntityRemoveComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity
);

/*
 * Removes the specified component from the given
 * entity, returns true if successful, false otherwise;
 * cannot remove entity id component
 */
#define vecsWorldEntityRemoveComponent( \
    typeName, \
    worldPtr, \
    entity \
) \
    (_vecsWorldEntityRemoveComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity \
    ))

/*
 * Removes the specified component from the entity
 * specified by the given id, returns true if
 * successful, false otherwise; cannot remove entity
 * id component
 */
bool _vecsWorldIdRemoveComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entityId
);

/*
 * Removes the specified component from the entity
 * specified by the given id, returns true if
 * successful, false otherwise; cannot remove entity
 * id component
 */
#define vecsWorldIdRemoveComponent( \
    typeName, \
    worldPtr, \
    entityId \
) \
    (_vecsWorldIdRemoveComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entityId \
    ))

/*
 * Queues an order to remove the specified component
 * from the given entity
 */
bool _vecsWorldEntityQueueRemoveComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity
);

/*
 * Queues an order to remove the specified component
 * from the given entity
 */
#define vecsWorldEntityQueueRemoveComponent( \
    typeName, \
    worldPtr, \
    entity \
) \
    (_vecsWorldEntityQueueRemoveComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        entity \
    ))

/*
 * Queues an order to remove the specified component
 * from the entity currently specified by the given id
 */
#define vecsWorldIdQueueRemoveComponent( \
    typeName, \
    worldPtr, \
    entityId \
) \
    (_vecsWorldEntityQueueRemoveComponent( \
        worldPtr, \
        vecsComponentGetId(typeName), \
        vecsWorldGetEntityById(worldPtr, entityId) \
    ))

/* Used for adding entities */
typedef struct VecsComponentDataPair{
    VecsComponentId componentId;

    /*
     * ptr to the component which will be copied into
     * the ECS world, or null if its a marker
     */
    void *componentPtr;
} VecsComponentDataPair;

/*
 * Adds the specified entity to the given ECS world
 * and returns the new entity; takes ownership of the
 * provided components but does not free the component
 * list
 */
VecsEntity vecsWorldAddEntity(
    VecsWorld *worldPtr,
    ArrayList *componentDataPairListPtr
);

/*
 * Queues an order to add the specified entity to the
 * given ECS world; takes ownership of the provided
 * components and frees the component list when
 * the order is handled - the component list will be
 * shallow copied to the heap so the user must still
 * free their version of it (the copy is 1-level deep)
 */
void vecsWorldQueueAddEntity(
    VecsWorld *worldPtr,
    ArrayList *componentDataPairListPtr
);

/*
 * Removes the specified entity from the given ECS
 * world, returns true if successful, false otherwise
 */
bool vecsWorldEntityRemoveEntity(
    VecsWorld *worldPtr,
    VecsEntity entity
);

/*
 * Removes the entity specified by the given id
 * from the given ECS world, returns true if
 * successful, false otherwise
 */
bool vecsWorldIdRemoveEntity(
    VecsWorld *worldPtr,
    VecsEntity entityId
);

/*
 * Queues an order to remove the specified entity from
 * the given ECS world, returns true if successful,
 * false otherwise (e.g. if the entity is already dead)
 */
bool vecsWorldEntityQueueRemoveEntity(
    VecsWorld *worldPtr,
    VecsEntity entity
);

/*
 * Queues an order to remove the entity specified by
 * the given id from the given ECS world
 */
#define vecsWorldIdQueueRemoveEntity( \
    worldPtr, \
    entityId \
) \
    vecsWorldEntityQueueRemoveEntity( \
        worldPtr, \
        vecsWorldGetEntityById(worldPtr, entityId) \
    )

/*
 * Handles all queued orders given to the ECS world
 * since the last time this function was called
 */
void vecsWorldHandleOrders(VecsWorld *worldPtr);

/*
 * Frees the memory associated with the given ECS
 * world
 */
void vecsWorldFree(VecsWorld *worldPtr);

#endif