#ifndef WINDECS_WORLD_H
#define WINDECS_WORLD_H

#include "Constructure.h"

#include "WindECS_Entity.h"
#include "_WindECS_Entities.h"
#include "WindECS_Components.h"
#include "WindECS_Query.h"

/*
 * An ECS world encapsulates all entity data and
 * provides access to various methods to interact
 * with said data; should not be moved once the ECS
 * begins running
 */
typedef struct WindWorld{
    /* an unordered list of archetypes */
    ArrayList _archetypeList;
    /* an unordered list of queries */
    ArrayList _queryList;
    /*
     * a map of bitsets to indices into the archetype
     * list
     */
    HashMap _archetypeIndexMap;
    /*
     * a map of bitset pairs to indices into the
     * query list
     */
    HashMap _queryIndexMap;

    /* entity metadata */
    _WindEntities _entities;
    /* weak ptr to user-defined component metadata */
    WindComponents *_componentsPtr;

    /* stores queued add component orders */
    ArrayList _addComponentQueue;
    /* stores queued set component orders */
    ArrayList _setComponentQueue;
    /* stores queued remove component orders */
    ArrayList _removeComponentQueue;
    /* stores queued add entity orders */
    ArrayList _addEntityQueue;
    /* stores queued remove entity orders */
    ArrayList _removeEntityQueue;
} WindWorld;

/*
 * Constructs and returns a new ECS world by value;
 * does not take ownership of the given WindComponents
 */
WindWorld windWorldMake(
    size_t entityCapacity,
    WindComponents *componentsPtr
);

/* Clears all state of the specified ECS world */
void windWorldClear(WindWorld *worldPtr);

/*
 * Returns a new query iterator; error if the accept
 * and reject sets intersect; does not take ownership
 * of the arguments
 */
WindQueryItr windWorldRequestQueryItr(
    WindWorld *worldPtr,
    Bitset *acceptComponentSetPtr,
    Bitset *rejectComponentSetPtr
);

/*
 * Creates and returns a handle to the entity of the
 * specified ID; error if the ID is currently dead
 */
WindEntity windWorldMakeHandle(
    WindWorld *worldPtr,
    WindEntityIDType entityID
);

/*
 * Returns true if the entity described by the given
 * entity handle is alive, false otherwise
 */
bool windWorldIsHandleAlive(
    WindWorld *worldPtr,
    WindEntity handle
);

/*
 * Returns true if the entity described by the given
 * entity handle is dead, false otherwise
 */
#define windWorldIsHandleDead(WORLDPTR, HANDLE) \
    (!(windWorldIsHandleAlive(WORLDPTR, HANDLE)))

/*
 * Returns true if the entity described by the given
 * ID is alive (ignoring generation), false otherwise
 */
bool windWorldIsIDAlive(
    WindWorld *worldPtr,
    WindEntityIDType entityID
);

/*
 * Returns true if the entity described by the given
 * ID is dead (ignoring generation), false otherwise
 */
#define windWorldIsIDDead(WORLDPTR, ENTITYID) \
    (!(windWorldIsIDAlive(WORLDPTR, ENTITYID)))

/*
 * Returns true if the entity described by the given
 * entity handle contains a component of the specified
 * ID, false otherwise (including if entity is dead)
 */
bool _windWorldHandleContainsComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
);

/*
 * Returns true if the entity described by the given
 * entity handle contains a component of the specified
 * type, false otherwise (including if entity is dead)
 */
#define windWorldHandleContainsComponent( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE \
) \
    (_windWorldHandleContainsComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE \
    ))

/*
 * Returns true if the entity described by the given
 * ID contains a component of the specified ID, false
 * otherwise (including if entity is dead)
 */
bool _windWorldIDContainsComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
);

/*
 * Returns true if the entity described by the given
 * ID contains a component of the specified type,
 * false otherwise (including if entity is dead)
 */
#define windWorldIDContainsComponent( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID \
) \
    (_windWorldIDContainsComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        ENTITYID \
    ))

/*
 * Returns a pointer to the component specified by the
 * given componentID of the entity specified by the
 * given handle; error if the componentID is invalid;
 * returns NULL if the component is a marker or if the
 * entity is dead
 */
void *_windWorldHandleGetPtr(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
);

/*
 * Returns a pointer to the component of the specified
 * type of the entity specified by the given handle;
 * error if the componentID is invalid; returns NULL
 * if the component is a marker or if the entity is
 * dead
 */
#define windWorldHandleGetPtr( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE \
) \
    ((TYPENAME *)_windWorldHandleGetPtr( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE \
    ))

/*
 * Returns the component of the specified type of the
 * entity specified by the given handle; error if the
 * componentID is invalid; undefined behavior if the
 * component is a marker or if the entity is dead
 */
#define windWorldHandleGet( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE \
) \
    (*windWorldHandleGetPtr( \
        TYPENAME, \
        WORLDPTR, \
        HANDLE \
    ))

/*
 * Returns a pointer to the component specified by the
 * given componentID of the entity specified by the
 * given ID; error if the componentID is invalid;
 * returns NULL if the component is a marker or if the
 * entity is dead
 */
void *_windWorldIDGetPtr(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
);

/*
 * Returns a pointer to the component of the specified
 * type of the entity specified by the given ID;
 * error if the componentID is invalid; returns NULL
 * if the component is a marker or if the entity is
 * dead
 */
#define windWorldIDGetPtr( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID \
) \
    ((TYPENAME *)_windWorldIDGetPtr( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        ENTITYID \
    ))

/*
 * Returns the component of the specified type of the
 * entity specified by the given ID; error if the
 * componentID is invalid; undefined behavior if the
 * component is a marker or if the entity is dead
 */
#define windWorldIDGet( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID \
) \
    (*windWorldIDGetPtr( \
        TYPENAME, \
        WORLDPTR, \
        ENTITYID \
    ))

/*
 * Adds the given component to the entity specified
 * by the given handle, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
bool _windWorldHandleAddComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Adds the given component to the entity specified
 * by the given handle, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
#define windWorldHandleAddComponent( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE, \
    COMPONENTPTR \
) \
    (_windWorldHandleAddComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE, \
        COMPONENTPTR \
    ))

/*
 * Adds the given component to the entity specified
 * by the given ID, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
bool _windWorldIDAddComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID,
    void *componentPtr
);

/*
 * Adds the given component to the entity specified
 * by the given ID, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
#define windWorldIDAddComponent( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID, \
    COMPONENTPTR \
) \
    (_windWorldIDAddComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        ENTITYID, \
        COMPONENTPTR \
    ))

/*
 * Queues an order to add the given component to the
 * entity specified by the given handle, returns true
 * if successful, false otherwise (e.g. the handle
 * is dead); the componentPtr is shallow copied to the
 * heap and the original pointer is not freed by the
 * ECS
 */
bool _windWorldHandleQueueAddComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Queues an order to add the given component to the
 * entity specified by the given handle, returns true
 * if successful, false otherwise (e.g. the handle
 * is dead); the componentPtr is shallow copied to the
 * heap and the original pointer is not freed by the
 * ECS
 */
#define windWorldHandleQueueAddComponent( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE, \
    COMPONENTPTR \
) \
    (_windWorldHandleQueueAddComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE, \
        COMPONENTPTR \
    ))

/*
 * Queues an order to add the given component to the
 * entity currently specified by the given ID, error
 * if the entity is dead; the componentPtr is shallow
 * copied to the heap and the original pointer is not
 * freed by the ECS
 */
#define windWorldIDQueueAddComponent( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID, \
    COMPONENTPTR \
) \
    (_windWorldHandleQueueAddComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        windWorldMakeHandle(WORLDPTR, ENTITYID), \
        COMPONENTPTR \
    ))

/*
 * Sets the given component to the entity specified
 * by the given handle, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
bool _windWorldHandleSetComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Sets the given component to the entity specified
 * by the given handle, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
#define windWorldHandleSetComponent( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE, \
    COMPONENTPTR \
) \
    (_windWorldHandleSetComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE, \
        COMPONENTPTR \
    ))

/*
 * Sets the given component to the entity specified
 * by the given ID, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
bool _windWorldIDSetComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID,
    void *componentPtr
);

/*
 * Sets the given component to the entity specified
 * by the given ID, returns true if successful,
 * false otherwise; the componentPtr is shallow copied
 * and is not freed by the ECS
 */
#define windWorldIDSetComponent( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID, \
    COMPONENTPTR \
) \
    (_windWorldIDSetComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        ENTITYID, \
        COMPONENTPTR \
    ))

/*
 * Queues an order to set the given component of the
 * entity specified by the given handle to the 
 * provided value, returns true if successful, false
 * otherwise (e.g. the handle is dead); the
 * componentPtr is shallow copied to the heap and the
 * original pointer is not freed by the ECS
 */
bool _windWorldHandleQueueSetComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Queues an order to set the given component of the
 * entity specified by the given handle to the 
 * provided value, returns true if successful, false
 * otherwise (e.g. the handle is dead); the
 * componentPtr is shallow copied to the heap and the
 * original pointer is not freed by the ECS
 */
#define windWorldHandleQueueSetComponent( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE, \
    COMPONENTPTR \
) \
    (_windWorldHandleQueueSetComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE, \
        COMPONENTPTR \
    ))

/*
 * Queues an order to set the given component of the
 * entity currently specified by the given ID to the
 * provided value, error if the entity is dead; the
 * componentPtr is shallow copied to the heap and the
 * original pointer is not freed by the ECS
 */
#define windWorldIDQueueSetComponent( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID, \
    COMPONENTPTR \
) \
    (_windWorldHandleQueueSetComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        windWorldMakeHandle(WORLDPTR, ENTITYID), \
        COMPONENTPTR \
    ))

/*
 * Removes the specified component from the entity
 * specified by the given handle, returns true if
 * successful, false otherwise
 */
bool _windWorldHandleRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
);

/*
 * Removes the specified component from the entity
 * specified by the given handle, returns true if
 * successful, false otherwise
 */
#define windWorldHandleRemoveComponent( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE \
) \
    (_windWorldHandleRemoveComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE \
    ))

/*
 * Removes the specified component from the entity
 * specified by the given ID, returns true if
 * successful, false otherwise
 */
bool _windWorldIDRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
);

/*
 * Removes the specified component from the entity
 * specified by the given ID, returns true if
 * successful, false otherwise
 */
#define windWorldIDRemoveComponent( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID \
) \
    (_windWorldIDRemoveComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        ENTITYID \
    ))

/*
 * Queues an order to remove the specified component
 * from the entity specified by the given handle
 */
bool _windWorldHandleQueueRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
);

/*
 * Queues an order to remove the specified component
 * from the entity specified by the given handle
 */
#define windWorldHandleQueueRemoveComponent( \
    TYPENAME, \
    WORLDPTR, \
    HANDLE \
) \
    (_windWorldHandleQueueRemoveComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        HANDLE \
    ))

/*
 * Queues an order to remove the specified component
 * from the entity currently specified by the given ID
 */
#define windWorldIDQueueRemoveComponent( \
    TYPENAME, \
    WORLDPTR, \
    ENTITYID \
) \
    (_windWorldHandleQueueRemoveComponent( \
        WORLDPTR, \
        windComponentGetID(TYPENAME), \
        windWorldMakeHandle(WORLDPTR, ENTITYID) \
    ))

/* Used for adding entities */
typedef struct WindComponentDataPair{
    WindComponentIDType componentID;

    /*
     * ptr to the component which will be copied into
     * the ECS world, or null if its a marker
     */
    void *componentPtr;
} WindComponentDataPair;

/*
 * Adds the specified entity to the given ECS world
 * and returns its handle; takes ownership of the
 * provided components but does not free the component
 * list
 */
WindEntity windWorldAddEntity(
    WindWorld *worldPtr,
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
void windWorldQueueAddEntity(
    WindWorld *worldPtr,
    ArrayList *componentDataPairListPtr
);

/*
 * Removes the entity specified by the given handle
 * from the given ECS world, returns true if
 * successful, false otherwise
 */
bool windWorldHandleRemoveEntity(
    WindWorld *worldPtr,
    WindEntity handle
);

/*
 * Removes the entity specified by the given ID
 * from the given ECS world, returns true if
 * successful, false otherwise
 */
bool windWorldIDRemoveEntity(
    WindWorld *worldPtr,
    WindEntityIDType entityID
);

/*
 * Queues an order to remove the entity specified by
 * the given handle from the given ECS world, returns
 * true if successful, false otherwise (e.g. if the
 * entity is already dead)
 */
bool windWorldHandleQueueRemoveEntity(
    WindWorld *worldPtr,
    WindEntity handle
);

/*
 * Queues an order to remove the entity specified by
 * the given ID from the given ECS world
 */
#define windWorldIDQueueRemoveEntity( \
    WORLDPTR, \
    ENTITYID \
) \
    windWorldHandleQueueRemoveEntity( \
        WORLDPTR, \
        windWorldMakeHandle(WORLDPTR, ENTITYID) \
    )

/*
 * Handles all queued orders given to the ECS world
 * since the last time this function was called
 */
void windWorldHandleOrders(WindWorld *worldPtr);

/*
 * Frees the memory associated with the given ECS
 * world
 */
void windWorldFree(WindWorld *worldPtr);

#endif