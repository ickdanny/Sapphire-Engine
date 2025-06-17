#include "Vecs_World.h"

#define archetypeListInitCapacity 50
#define queryListInitCapacity 50

/* A pair of component sets used for query mapping */
typedef struct VecsComponentSetPair{
    VecsComponentSet accept;
    VecsComponentSet reject;
} VecsComponentSetPair;

/* Hash function for component set */
static size_t vecsComponentSetHash(
    const void *componentSetPtr
){
    VecsComponentSet *castPtr
        = (VecsComponentSet*)componentSetPtr;
    VecsComponentSet componentSet = *castPtr;
    if(sizeof(componentSet) >= sizeof(size_t)){
        return componentSet;
    }
    else if(sizeof(componentSet)
        == (sizeof(size_t) / 2)
    ){
        size_t low = componentSet;
        size_t high = (componentSet >> sizeof(size_t));
        return low + high;
    }
    /* be lazy and just truncate */
    else{
        return componentSet;
    }
}

/* Equals function for component set */
static bool vecsComponentSetEquals(
    const void *componentSetPtr1,
    const void *componentSetPtr2
){
    VecsComponentSet *castPtr1
        = (VecsComponentSet*)componentSetPtr1;
    VecsComponentSet *castPtr2
        = (VecsComponentSet*)componentSetPtr2;
    return (*castPtr1) == (*castPtr2);
}

/* Hash function for component set pair */
static size_t vecsComponentSetPairHash(
    const void *componentSetPairPtr)
{
    VecsComponentSetPair *castPtr
        = (VecsComponentSetPair*)componentSetPairPtr;
    size_t hash = 0;
    hash += vecsComponentSetHash(&(castPtr->accept));
    hash *= 7;
    hash += vecsComponentSetHash(&(castPtr->reject));
    return hash;
}

/* Equals function for component set pair */
static bool vecsComponentSetPairEquals(
    const void *componentSetPairPtr1,
    const void *componentSetPairPtr2
){
    VecsComponentSetPair *castPtr1
        = (VecsComponentSetPair*)componentSetPairPtr1;
    VecsComponentSetPair *castPtr2
        = (VecsComponentSetPair*)componentSetPairPtr2;
    return (castPtr1->accept == castPtr2->accept)
        && (castPtr1->reject == castPtr2->reject);
}

/* an order to add a component to an entity */
typedef struct AddComponentOrder{
    VecsEntity entity;
    VecsComponentId componentId;
    void *componentPtr;
} AddComponentOrder;

/*
 * frees the copy of the component specified in the
 * given AddComponentOrder which was stored on the
 * heap
 */
static void addComponentOrderFree(
    AddComponentOrder *orderPtr
){
    pgFree(orderPtr->componentPtr);
}

/* an order to set a component in an entity */
typedef struct SetComponentOrder{
    VecsEntity entity;
    VecsComponentId componentId;
    void *componentPtr;
} SetComponentOrder;

/*
 * frees the copy of the component specified in the
 * given SetComponentOrder which was stored on the
 * heap
 */
static void setComponentOrderFree(
    SetComponentOrder *orderPtr
){
    pgFree(orderPtr->componentPtr);
}

/* an order to remove a component from an entity */
typedef struct RemoveComponentOrder{
    VecsEntity entity;
    VecsComponentId componentId;
} RemoveComponentOrder;

/* an order to add an entity */
typedef struct AddEntityOrder{
    /* a 1-level copy of what the user passes in */
    ArrayList componentDataPairList;
    /*
     * for efficiency, when an add entity order is
     * created, all components get copied into the
     * same heap block; this points to the base
     * of that heap block but is NULL if the entity
     * is all markers
     */
    void *componentDataBasePtr;
} AddEntityOrder;

/*
 * frees the memory associated with the given
 * AddEntityOrder
 */
static void addEntityOrderFree(
    AddEntityOrder *orderPtr
){
    /*
     * do not run the destructor, just free the
     * dynamically allocated component values
     */
    if(orderPtr->componentDataBasePtr){
        pgFree(orderPtr->componentDataBasePtr);
    }
    arrayListFree(VecsComponentDataPair,
        &(orderPtr->componentDataPairList)
    );
}

/* an order to remove an entity */
typedef struct RemoveEntityOrder{
    VecsEntity entity;
} RemoveEntityOrder;

/*
 * Constructs and returns a new ECS world by value;
 * does not take ownership of the given component list
 */
VecsWorld vecsWorldMake(
    size_t entityCapacity,
    VecsComponentList *componentListPtr
){
    return (VecsWorld) {
        ._archetypeList = arrayListMake(
            _VecsArchetype,
            archetypeListInitCapacity
        ),
        ._queryList = arrayListMake(
            VecsQuery,
            queryListInitCapacity
        ),
        ._archetypeIndexMap = hashMapMake(
            VecsComponentSet,
            size_t,
            archetypeListInitCapacity * 2,
            vecsComponentSetHash,
            vecsComponentSetEquals
        ),
        ._queryIndexMap = hashMapMake(
            VecsComponentSetPair,
            size_t,
            queryListInitCapacity * 2,
            vecsComponentSetPairHash,
            vecsComponentSetPairEquals
        ),
        ._entityList = _vecsEntityListMake(
            entityCapacity
        ),
        /* does not take ownership */
        ._componentListPtr = componentListPtr,
        ._addComponentQueue = arrayListMake(
            AddComponentOrder,
            entityCapacity
        ),
        ._setComponentQueue = arrayListMake(
            SetComponentOrder,
            entityCapacity
        ),
        ._removeComponentQueue = arrayListMake(
            RemoveComponentOrder,
            entityCapacity
        ),
        ._addEntityQueue = arrayListMake(
            AddEntityOrder,
            entityCapacity
        ),
        ._removeEntityQueue = arrayListMake(
            RemoveEntityOrder,
            entityCapacity
        )
    };
}

/* Clears all state of the specified ECS world */
void vecsWorldClear(VecsWorld *worldPtr){
    /*
     * handle orders so we don't have to deal with
     * manually clearing out the order queues
     */
    vecsWorldHandleOrders(worldPtr);

    /* clear archetypes of entity data */
    arrayListApply(_VecsArchetype,
        &(worldPtr->_archetypeList),
        _vecsArchetypeClear
    );

    /* clear entity metadata */
    _vecsEntityListClear(&(worldPtr->_entityList));
}

/*
 * Initializes a new query and inserts it into the
 * given ECS world; error if a mapping
 * for the given component set pair already exists
 */
static VecsQuery *vecsWorldInsertQuery(
    VecsWorld *worldPtr,
    VecsComponentSetPair componentSetPair
){
    assertFalse(
        hashMapHasKeyPtr(VecsComponentSetPair, size_t,
            &(worldPtr->_queryIndexMap),
            &componentSetPair
        ),
        "error: trying to insert query when mapping "
        "already exists; "
        SRC_LOCATION
    );
    /*
     * query ctor will automatically try accept all
     * archetypes
     */
    VecsQuery newQuery = vecsQueryMake(
        componentSetPair.accept,
        componentSetPair.reject,
        &(worldPtr->_archetypeList)
    );
    arrayListPushBack(VecsQuery,
        &(worldPtr->_queryList),
        newQuery
    );
    /*
     * new query is now owned by the world; do not
     * free
     */

    /*
     * map the bitset pair to the last list index;
     * the bitset pair is now owned by the hashmap
     */
    size_t lastQueryListIndex
        = worldPtr->_queryList.size - 1;
    hashMapPutPtr(VecsComponentSetPair, size_t,
        &(worldPtr->_queryIndexMap),
        &componentSetPair,
        &lastQueryListIndex
    );
    
    return arrayListBackPtr(VecsQuery,
        &(worldPtr->_queryList)
    );
}

/*
 * Returns a new query iterator; error if the accept
 * and reject sets intersect
 */
VecsQueryItr vecsWorldRequestQueryItr(
    VecsWorld *worldPtr,
    VecsComponentSet acceptComponentSet,
    VecsComponentSet rejectComponentSet
){
    assertNotNull(
        worldPtr,
        "null passed; " SRC_LOCATION
    );
    VecsComponentSetPair componentSetPair = {
        .accept = acceptComponentSet,
        .reject = rejectComponentSet
    };
    VecsQuery *queryPtr = NULL;

    if(hashMapHasKeyPtr(VecsComponentSetPair, size_t,
        &(worldPtr->_queryIndexMap),
        &componentSetPair
    )){
        size_t queryIndex = *hashMapGetPtr(
            VecsComponentSetPair,
            size_t,
            &(worldPtr->_queryIndexMap),
            &componentSetPair
        );
        queryPtr = arrayListGetPtr(VecsQuery,
            &(worldPtr->_queryList),
            queryIndex
        );
    }
    else{
        queryPtr = vecsWorldInsertQuery(
            worldPtr,
            componentSetPair
        );
    }
    return vecsQueryItr(queryPtr);
}

/*
 * Gets the entity specified by the given id; error if
 * no such entity is currently live
 */
VecsEntity vecsWorldGetEntityById(
    VecsWorld *worldPtr,
    VecsEntity entityId
){
    assertFalse(
        _vecsEntityListIsIdDead(
            &(worldPtr->_entityList),
            entityId
        ),
        "error: try to get entity with dead id; "
        SRC_LOCATION
    );
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListIdGetMetadata(
            &(worldPtr->_entityList),
            entityId
        );
    return entityMetadataPtr->_canonicalEntity;
}

/*
 * Returns true if the given entity is live, false
 * otherwise
 */
bool vecsWorldIsEntityLive(
    VecsWorld *worldPtr,
    VecsEntity entity
){
    return _vecsEntityListIsEntityLive(
        &(worldPtr->_entityList),
        entity
    );
}

/*
 * Returns true if the entity described by the given
 * id is live (ignoring generation), false otherwise
 */
bool vecsWorldIsIdLive(
    VecsWorld *worldPtr,
    VecsEntity entityId
){
    return _vecsEntityListIsIdLive(
        &(worldPtr->_entityList),
        entityId
    );
}

/*
 * Returns true if the given entity contains a
 * component of the specified id, false otherwise
 * (including if entity is dead)
 */
bool _vecsWorldEntityContainsComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity
){
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            &(worldPtr->_entityList),
            entity
        );
    return vecsComponentSetContainsId(
        entityMetadataPtr->_componentSet,
        componentId
    );
}

/*
 * Returns true if the entity described by the given
 * id contains a component of the specified id, false
 * otherwise (including if entity is dead)
 */
bool _vecsWorldIdContainsComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entityId
){
    if(vecsWorldIsIdDead(worldPtr, entityId)){
        return false;
    }
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            &(worldPtr->_entityList),
            vecsWorldGetEntityById(worldPtr, entityId)
        );
    return vecsComponentSetContainsId(
        entityMetadataPtr->_componentSet,
        componentId
    );
}

/*
 * Initializes a new archetype and inserts it into the
 * given ECS world; error if a mapping for the given
 * component set already exists
 */
static _VecsArchetype *vecsWorldInsertArchetype(
    VecsWorld *worldPtr,
    VecsComponentSet componentSet
){
    assertFalse(
        hashMapHasKeyPtr(VecsComponentSet, size_t,
            &(worldPtr->_archetypeIndexMap),
            &componentSet
        ),
        "error: trying to insert archetype when "
        "mapping already exists; "
        SRC_LOCATION
    );
    /* archetype ctor makes a copy of the bitset */
    _VecsArchetype newArchetype = _vecsArchetypeMake(
        componentSet,
        worldPtr->_entityList._entityMetadataArray
            .size,
        worldPtr->_componentListPtr,
        &(worldPtr->_entityList)
    );
    arrayListPushBack(_VecsArchetype,
        &(worldPtr->_archetypeList),
        newArchetype
    );
    size_t lastArchetypeListIndex
        = worldPtr->_archetypeList.size - 1;
    /* have every query try to accept new archetype */
    for(size_t i = 0;
        i < worldPtr->_queryList.size;
        ++i
    ){
        _vecsQueryTryAcceptArchetype(
            arrayListGetPtr(VecsQuery,
                &(worldPtr->_queryList),
                i
            ),
            lastArchetypeListIndex
        );
    }
    /*
     * new archetype is now owned by the world; do not
     * free
     */

    /* map the component set to the last list index */
    hashMapPutPtr(VecsComponentSet, size_t,
        &(worldPtr->_archetypeIndexMap),
        &componentSet,
        &lastArchetypeListIndex
    );
    
    return arrayListBackPtr(_VecsArchetype,
        &(worldPtr->_archetypeList)
    );
}

/*
 * Returns a pointer to the archetype having the given
 * component set; creates such an archetype if needed
 */
static _VecsArchetype *vecsWorldGetArchetype(
    VecsWorld *worldPtr,
    VecsComponentSet componentSet
){
    size_t *indexPtr = hashMapGetPtr(
        VecsComponentSet,
        size_t,
        &(worldPtr->_archetypeIndexMap),
        &componentSet
    );
    size_t index = 0;
    /*
     * if failed to retrieve index ptr, make a new
     * archetype
     */
    if(!indexPtr){
        index = worldPtr->_archetypeIndexMap.size;
        vecsWorldInsertArchetype(
            worldPtr,
            componentSet
        );
    }
    else{
        index = *indexPtr;
    }
    assertTrue(
        index < worldPtr->_archetypeIndexMap.size,
        "error: archetype index out of bounds; "
        SRC_LOCATION
    );
    return arrayListGetPtr(_VecsArchetype,
        &(worldPtr->_archetypeList),
        index
    );
}

/*
 * Returns a pointer to the archetype of the given
 * entity; error if the entity is dead
 */
static _VecsArchetype *vecsWorldEntityGetArchetype(
    VecsWorld *worldPtr,
    VecsEntity entity
){
    assertTrue(
        vecsWorldIsEntityLive(
            worldPtr,
            entity
        ),
        "error: try to get archetype of dead entity; "
        SRC_LOCATION
    );
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            &(worldPtr->_entityList),
            entity
        );
    assertNotNull(
        entityMetadataPtr,
        "error: entity is live but has no metadata; "
        SRC_LOCATION
    );
    assertNotNull(
        entityMetadataPtr->_archetypePtr,
        "error: entity is live but has no archetype; "
        SRC_LOCATION
    );
    return entityMetadataPtr->_archetypePtr;
}

/*
 * Returns a pointer to the archetype of the entity
 * specified by the given id; error if the entity
 * is dead
 */
static _VecsArchetype *vecsWorldIdGetArchetype(
    VecsWorld *worldPtr,
    VecsEntity entityId
){
    assertTrue(
        vecsWorldIsIdLive(
            worldPtr,
            entityId
        ),
        "error: try to get archetype of dead entity; "
        SRC_LOCATION
    );
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListIdGetMetadata(
            &(worldPtr->_entityList),
            entityId
        );
    assertNotNull(
        entityMetadataPtr,
        "error: entity is live but has no metadata; "
        SRC_LOCATION
    );
    assertNotNull(
        entityMetadataPtr->_archetypePtr,
        "error: entity is live but has no archetype; "
        SRC_LOCATION
    );
    return entityMetadataPtr->_archetypePtr;
}

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
){
    /* return NULL if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return NULL;
    }
    /* error if component Id is invalid */
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            &(worldPtr->_entityList),
            entity
        );
    assertTrue(
        vecsComponentSetContainsId(
            entityMetadataPtr->_componentSet,
            componentId
        ),
        "error: trying to get component entity lacks; "
        SRC_LOCATION
    );
    /* return NULL if component is marker */
    VecsComponentMetadata componentMetadata
        = vecsComponentListGetMetadata(
            worldPtr->_componentListPtr,
            componentId
        );
    if(componentMetadata._componentSize == 0){
        return NULL;
    }

    /* get ptr to archetype */
    _VecsArchetype *archetypePtr
        = vecsWorldEntityGetArchetype(
            worldPtr,
            entity
        );
    /* ask archetype to get a ptr to the component */
    return __vecsArchetypeGetPtr(
        archetypePtr,
        componentId,
        entity
    );
}

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
){
    /* return NULL if entity dead */
    if(vecsWorldIsIdDead(worldPtr, entityId)){
        return NULL;
    }
    /* error if component Id is invalid */
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListIdGetMetadata(
            &(worldPtr->_entityList),
            entityId
        );
    assertTrue(
        vecsComponentSetContainsId(
            entityMetadataPtr->_componentSet,
            componentId
        ),
        "error: trying to get component entity lacks; "
        SRC_LOCATION
    );
    /* return NULL if component is marker */
    VecsComponentMetadata componentMetadata
        = vecsComponentListGetMetadata(
            worldPtr->_componentListPtr,
            componentId
        );
    if(componentMetadata._componentSize == 0){
        return NULL;
    }

    /* get ptr to archetype */
    _VecsArchetype *archetypePtr
        = vecsWorldIdGetArchetype(
            worldPtr,
            entityId
        );
    /* ask archetype to get a ptr to the component */
    return __vecsArchetypeGetPtr(
        archetypePtr,
        componentId,
        entityMetadataPtr->_canonicalEntity
    );
}

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
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    return _vecsWorldIdAddComponent(
        worldPtr,
        componentId,
        vecsEntityId(entity),
        componentPtr
    );
}

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
){
    /* return false if entity dead */
    if(vecsWorldIsIdDead(worldPtr, entityId)){
        return false;
    }
    /* return false if entity already has component */
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListIdGetMetadata(
            &(worldPtr->_entityList),
            entityId
        );
    if(vecsComponentSetContainsId(
        entityMetadataPtr->_componentSet,
        componentId
    )){
        return false;
    }
    /* get old archetype */
    _VecsArchetype *oldArchetypePtr
        = vecsWorldIdGetArchetype(
            worldPtr,
            entityId
        );
    /* get new archetype */
    VecsComponentSet newComponentSet
        = vecsComponentSetAddId(
            entityMetadataPtr->_componentSet,
            componentId
        );
    _VecsArchetype *newArchetypePtr
        = vecsWorldGetArchetype(
            worldPtr,
            newComponentSet
        );
    /* move components from old archetype to new */
    _vecsArchetypeMoveEntity(
        oldArchetypePtr,
        newArchetypePtr,
        entityMetadataPtr->_canonicalEntity
    );
    /*
     * add component to the entity in the new 
     * archetype (as long as it is not a marker)
     */
    __vecsArchetypeSetPtr(
        newArchetypePtr,
        componentId,
        entityMetadataPtr->_canonicalEntity,
        componentPtr
    );
    
    return true;
}

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
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    VecsComponentMetadata componentMetadata
        = vecsComponentListGetMetadata(
            worldPtr->_componentListPtr,
            componentId
        );
    void *heapCopy = NULL;
    /* make a heap copy if component not marker */
    if(componentMetadata._componentSize != 0){
        heapCopy = pgAlloc(
            1,
            componentMetadata._componentSize
        );
        memcpy(
            heapCopy,
            componentPtr,
            componentMetadata._componentSize
        );
    }
    /* queue up the order */
    AddComponentOrder order = {
        entity,
        componentId,
        heapCopy
    };
    arrayListPushBack(AddComponentOrder,
        &(worldPtr->_addComponentQueue),
        order
    );
    return true;
}

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
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    return _vecsWorldIdSetComponent(
        worldPtr,
        componentId,
        vecsEntityId(entity),
        componentPtr
    );
}

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
){
    /* return false if entity dead */
    if(vecsWorldIsIdDead(worldPtr, entityId)){
        return false;
    }

    _VecsArchetype *oldArchetypePtr
        = vecsWorldIdGetArchetype(
            worldPtr,
            entityId
        );
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListIdGetMetadata(
            &(worldPtr->_entityList),
            entityId
        );
    /* if entity has component, replace it */
    if(vecsComponentSetContainsId(
        entityMetadataPtr->_componentSet,
        componentId
    )){
        VecsComponentMetadata componentMetadata
            = vecsComponentListGetMetadata(
                worldPtr->_componentListPtr,
                componentId
            );
        /* do nothing if component is marker */
        if(componentMetadata._componentSize == 0){
            return true;
        }
        void *componentPtrIntoArchetype
            = __vecsArchetypeGetPtr(
                oldArchetypePtr,
                componentId,
                entityMetadataPtr->_canonicalEntity
            );
        /* run destructor on old component if needed */
        if(componentMetadata._destructor){
            componentMetadata._destructor(
                componentPtrIntoArchetype
            );
        }
        /* shallow copy component into archetype */
        memcpy(
            componentPtrIntoArchetype,
            componentPtr,
            componentMetadata._componentSize
        );
        return true;
    }
    /* otherwise, move archetypes (same as add) */
    VecsComponentSet newComponentSet
        = vecsComponentSetAddId(
            entityMetadataPtr->_componentSet,
            componentId
        );
    _VecsArchetype *newArchetypePtr
        = vecsWorldGetArchetype(
            worldPtr,
            newComponentSet
        );
    /* move components from old archetype to new */
    _vecsArchetypeMoveEntity(
        oldArchetypePtr,
        newArchetypePtr,
        entityMetadataPtr->_canonicalEntity
    );
    /*
     * add component to the entity in the new 
     * archetype (as long as it is not a marker)
     */
    __vecsArchetypeSetPtr(
        newArchetypePtr,
        componentId,
        entityMetadataPtr->_canonicalEntity,
        componentPtr
    );
    
    return true;
}

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
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    VecsComponentMetadata componentMetadata
        = vecsComponentListGetMetadata(
               worldPtr->_componentListPtr,
               componentId
        );
    void *heapCopy = NULL;
    /* make a heap copy if component not marker */
    if(componentMetadata._componentSize != 0){
        heapCopy = pgAlloc(
            1,
            componentMetadata._componentSize
        );
        memcpy(
            heapCopy,
            componentPtr,
            componentMetadata._componentSize
        );
    }
    /* queue up the order */
    SetComponentOrder order = {
        entity,
        componentId,
        heapCopy
    };
    arrayListPushBack(SetComponentOrder,
        &(worldPtr->_setComponentQueue),
        order
    );
    return true;
}

/*
 * Removes the specified component from the given
 * entity, returns true if successful, false otherwise;
 * cannot remove entity id component
 */
bool _vecsWorldEntityRemoveComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }

    return _vecsWorldIdRemoveComponent(
        worldPtr,
        componentId,
        vecsEntityId(entity)
    );
}

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
){
    /* return false if entity dead */
    if(vecsWorldIsIdDead(worldPtr, entityId)){
        return false;
    }
    /*
     * return false if attempting to remove entity id
     */
    if(componentId == VecsEntityId){
        return false;
    }
    /* return false if entity doesn't have component */
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListIdGetMetadata(
            &(worldPtr->_entityList),
            entityId
        );
    if(!vecsComponentSetContainsId(
        entityMetadataPtr->_componentSet,
        componentId
    )){
        return false;
    }
    /* get old archetype */
    _VecsArchetype *oldArchetypePtr
        = vecsWorldIdGetArchetype(
            worldPtr,
            entityId
        );
    /* do not run destructor; archetype move will */

    /* get new archetype */
    VecsComponentSet newComponentSet
        = vecsComponentSetRemoveId(
            entityMetadataPtr->_componentSet,
            componentId
        );
    _VecsArchetype *newArchetypePtr
        = vecsWorldGetArchetype(
            worldPtr,
            newComponentSet
        );
    /*
     * move components from old archetype to new,
     * essentially truncating the entity; also updates
     * the component set
     */
    _vecsArchetypeMoveEntity(
        oldArchetypePtr,
        newArchetypePtr,
        entityMetadataPtr->_canonicalEntity
    );
    return true;
}

/*
 * Queues an order to remove the specified component
 * from the given entity
 */
bool _vecsWorldEntityQueueRemoveComponent(
    VecsWorld *worldPtr,
    VecsComponentId componentId,
    VecsEntity entity
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    /* queue up the order */
    RemoveComponentOrder order = {
        entity,
        componentId
    };
    arrayListPushBack(RemoveComponentOrder,
        &(worldPtr->_removeComponentQueue),
        order
    );
    return true;
}

/*
 * Adds the specified entity to the given ECS world
 * and returns the new entity; takes ownership of the
 * provided components but does not free the component
 * list
 */
VecsEntity vecsWorldAddEntity(
    VecsWorld *worldPtr,
    ArrayList *componentDataPairListPtr
){
    /* allocate a new entity */
    VecsEntity entity = _vecsEntityListAllocate(
        &(worldPtr->_entityList)
    );
    /* grab entity metadata */
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            &(worldPtr->_entityList),
            entity
        );
    assertNotNull(
        entityMetadataPtr,
        "error: failed to get entity metadata of new "
        "entity; "
        SRC_LOCATION
    );
    /* toggle the component set for each component */
    VecsComponentSet componentSet
        = vecsEmptyComponentSet;
    VecsComponentId componentId = 0;
    for(size_t i = 0;
        i < componentDataPairListPtr->size;
        ++i
    ){
        componentId = arrayListGet(
            VecsComponentDataPair,
            componentDataPairListPtr,
            i
        ).componentId;
        assertFalse(
            vecsComponentSetContainsId(
                componentSet,
                componentId
            ),
            "error: duplicate component; "
            SRC_LOCATION
        );
        componentSet = vecsComponentSetAddId(
            componentSet,
            componentId
        );
    }
    /*
     * users should never manually add an entity
     * component
     */
    assertFalse(
        vecsComponentSetContainsId(
            componentSet,
            VecsEntityId
        ),
        "error: user cannot pass in an entity "
        "component; "
        SRC_LOCATION
    );
    /* toggle the entity component set */
    componentSet = vecsComponentSetAddId(
        componentSet,
        VecsEntityId
    );
    entityMetadataPtr->_componentSet = componentSet;
    /* get the entity's archetype */
    _VecsArchetype *archetypePtr
        = vecsWorldGetArchetype(
            worldPtr,
            componentSet
        );
    /* allocate space for entity's components */
    _vecsArchetypeAddEntity(
        archetypePtr,
        entity
    );
    /* shallow copy each component into archetype */
    VecsComponentMetadata componentMetadata = {0};
    for(size_t i = 0;
        i < componentDataPairListPtr->size;
        ++i
    ){
        componentId = arrayListGet(
            VecsComponentDataPair,
            componentDataPairListPtr,
            i
        ).componentId;
        componentMetadata
            = vecsComponentListGetMetadata(
                worldPtr->_componentListPtr,
                componentId
            );
        /* skip markers */
        if(componentMetadata._componentSize == 0){
            continue;
        }
        /* shallow copy each component from its ptr */
        __vecsArchetypeSetPtr(
            archetypePtr,
            componentId,
            entity,
            arrayListGet(VecsComponentDataPair,
                componentDataPairListPtr,
                i
            ).componentPtr
        );
    }
    return entity;
}

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
){
    /* add a dummy to the queue; construct in place */
    AddEntityOrder dummy = {0};
    arrayListPushBack(AddEntityOrder,
        &(worldPtr->_addEntityQueue),
        dummy
    );
    AddEntityOrder *orderPtr = arrayListBackPtr(
        AddEntityOrder,
        &(worldPtr->_addEntityQueue)
    );
    ArrayList *componentDataPairListCopyPtr
        = &(orderPtr->componentDataPairList);
    /* make empty array list */
    size_t numComponents
        = componentDataPairListPtr->size;
    *componentDataPairListCopyPtr = arrayListMake(
        VecsComponentDataPair,
        numComponents
    );
    /* shallow copy every component */
    VecsComponentMetadata componentMetadata = {0};
    VecsComponentDataPair dataPair = {0};
    /*
     * optimization: only allocate a single block
     * on the heap for each component and keep track
     * of each component's offset; here we calculate
     * the size of that block
     */
    size_t entireComponentDataSize = 0;
    for(size_t i = 0;
        i < numComponents;
        ++i
    ){
        dataPair = arrayListGet(VecsComponentDataPair,
            componentDataPairListPtr,
            i
        );
        componentMetadata
            = vecsComponentListGetMetadata(
                worldPtr->_componentListPtr,
                dataPair.componentId
            );
        entireComponentDataSize
            += componentMetadata._componentSize;
    }
    /* 
     * base ptr to the block with all components;
     * allocate unless all markers
     */
    void *entireComponentDataBasePtr = NULL;
    if(entireComponentDataSize != 0){
        entireComponentDataBasePtr = pgAlloc(
            1,
            entireComponentDataSize
        );
    }
    /* write the base ptr to the order */
    orderPtr->componentDataBasePtr
        = entireComponentDataBasePtr;
    /* ptr to the start of the current component */
    void *currentComponentPtr
        = entireComponentDataBasePtr;
    for(size_t i = 0;
        i < numComponents;
        ++i
    ){
        /* make a value copy of the data pair */
        dataPair = arrayListGet(VecsComponentDataPair,
            componentDataPairListPtr,
            i
        );
        componentMetadata = vecsComponentListGetMetadata(
            worldPtr->_componentListPtr,
            dataPair.componentId
        );
        /* if marker, use NULL as the component */
        if(componentMetadata._componentSize == 0){
            dataPair.componentPtr = NULL;
        }
        /* otherwise, make shallow copy on heap */
        else{
            memcpy(
                currentComponentPtr,
                dataPair.componentPtr,
                componentMetadata._componentSize
            );
            dataPair.componentPtr
                = currentComponentPtr;
            currentComponentPtr = voidPtrAdd(
                currentComponentPtr,
                componentMetadata._componentSize
            );
        }
        /* push to the copy list */
        arrayListPushBack(VecsComponentDataPair,
            componentDataPairListCopyPtr,
            dataPair
        );
    } /* end of for loop */
}

/*
 * Removes the specified entity from the given ECS
 * world, returns true if successful, false otherwise
 */
bool vecsWorldEntityRemoveEntity(
    VecsWorld *worldPtr,
    VecsEntity entity
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    return vecsWorldIdRemoveEntity(
        worldPtr,
        vecsEntityId(entity)
    );
}

/*
 * Removes the entity specified by the given id
 * from the given ECS world, returns true if
 * successful, false otherwise
 */
bool vecsWorldIdRemoveEntity(
    VecsWorld *worldPtr,
    VecsEntity entityId
){
    /* return false if entity dead */
    if(vecsWorldIsIdDead(worldPtr, entityId)){
        return false;
    }
    /* get archetype */
    _VecsArchetype *archetypePtr
        = vecsWorldIdGetArchetype(
            worldPtr,
            entityId
        );
    /* clear entity metadata */
    VecsEntity entity = vecsWorldGetEntityById(
        worldPtr,
        entityId
    );
    _vecsEntityListReclaim(
        &(worldPtr->_entityList),
        entity
    );
    /* clear entity component data */
    bool success = _vecsArchetypeRemoveEntity(
        archetypePtr,
        entity
    );
    assertTrue(
        success,
        "error: failed to remove entity from "
        "archetype; "
        SRC_LOCATION
    );
    return true;
}

/*
 * Queues an order to remove the specified entity from
 * the given ECS world, returns true if successful,
 * false otherwise (e.g. if the entity is already dead)
 */
bool vecsWorldEntityQueueRemoveEntity(
    VecsWorld *worldPtr,
    VecsEntity entity
){
    /* return false if entity dead */
    if(vecsWorldIsEntityDead(worldPtr, entity)){
        return false;
    }
    RemoveEntityOrder order = {
        .entity = entity
    };
    arrayListPushBack(RemoveEntityOrder,
        &(worldPtr->_removeEntityQueue),
        order
    );
    return true;
}

/*
 * Handles all queued remove entity orders given
 * to the ECS world since the last time this
 * function was called
 */
static void vecsWorldHandleRemoveEntityOrders(
    VecsWorld *worldPtr
){
    VecsEntity toRemove = {0};
    for(size_t i = 0;
        i < worldPtr->_removeEntityQueue.size;
        ++i
    ){
        toRemove = arrayListGet(RemoveEntityOrder,
            &(worldPtr->_removeEntityQueue),
            i
        ).entity;
        vecsWorldEntityRemoveEntity(
            worldPtr,
            toRemove
        );
    }
    arrayListClear(RemoveEntityOrder,
        &(worldPtr->_removeEntityQueue)
    );
}

/*
 * Handles all queued remove component orders given
 * to the ECS world since the last time this
 * function was called
 */
static void vecsWorldHandleRemoveComponentOrders(
    VecsWorld *worldPtr
){
    RemoveComponentOrder order = {0};
    for(size_t i = 0;
        i < worldPtr->_removeComponentQueue.size;
        ++i
    ){
        order = arrayListGet(RemoveComponentOrder,
            &(worldPtr->_removeComponentQueue),
            i
        );
        _vecsWorldEntityRemoveComponent(
            worldPtr,
            order.componentId,
            order.entity
        );
    }
    arrayListClear(RemoveComponentOrder,
        &(worldPtr->_removeComponentQueue)
    );
}

/*
 * Handles all queued add entity orders given
 * to the ECS world since the last time this
 * function was called
 */
static void vecsWorldHandleAddEntityOrders(
    VecsWorld *worldPtr
){
    AddEntityOrder *orderPtr = NULL;
    for(size_t i = 0;
        i < worldPtr->_addEntityQueue.size;
        ++i
    ){
        orderPtr = arrayListGetPtr(
            AddEntityOrder,
            &(worldPtr->_addEntityQueue),
            i
        );
        vecsWorldAddEntity(
            worldPtr,
            &(orderPtr->componentDataPairList)
        );
        /*
         * free the heap block associated with
         * the order
         */
        addEntityOrderFree(orderPtr);
    }
    arrayListClear(AddEntityOrder,
        &(worldPtr->_addEntityQueue)
    );
}

/*
 * Handles all queued add component orders given
 * to the ECS world since the last time this
 * function was called
 */
static void vecsWorldHandleAddComponentOrders(
    VecsWorld *worldPtr
){
    AddComponentOrder *orderPtr = NULL;
    for(size_t i = 0;
        i < worldPtr->_addComponentQueue.size;
        ++i
    ){
        orderPtr = arrayListGetPtr(
            AddComponentOrder,
            &(worldPtr->_addComponentQueue),
            i
        );
        _vecsWorldEntityAddComponent(
            worldPtr,
            orderPtr->componentId,
            orderPtr->entity,
            orderPtr->componentPtr
        );
        /*
         * free the heap block associated with
         * the order
         */
        addComponentOrderFree(orderPtr);
    }
    arrayListClear(AddComponentOrder,
        &(worldPtr->_addComponentQueue)
    );
}

/*
 * Handles all queued set component orders given
 * to the ECS world since the last time this
 * function was called
 */
static void vecsWorldHandleSetComponentOrders(
    VecsWorld *worldPtr
){
    SetComponentOrder *orderPtr = NULL;
    for(size_t i = 0;
        i < worldPtr->_setComponentQueue.size;
        ++i
    ){
        orderPtr = arrayListGetPtr(
            SetComponentOrder,
            &(worldPtr->_setComponentQueue),
            i
        );
        _vecsWorldEntitySetComponent(
            worldPtr,
            orderPtr->componentId,
            orderPtr->entity,
            orderPtr->componentPtr
        );
        /*
         * free the heap block associated with
         * the order
         */
        setComponentOrderFree(orderPtr);
    }
    arrayListClear(SetComponentOrder,
        &(worldPtr->_setComponentQueue)
    );
}

/*
 * Handles all queued orders given to the ECS world
 * since the last time this function was called
 */
void vecsWorldHandleOrders(VecsWorld *worldPtr){
    vecsWorldHandleRemoveEntityOrders(worldPtr);
    vecsWorldHandleRemoveComponentOrders(worldPtr);
    vecsWorldHandleAddEntityOrders(worldPtr);
    vecsWorldHandleAddComponentOrders(worldPtr);
    vecsWorldHandleSetComponentOrders(worldPtr);
}

/*
 * Frees the memory associated with the given ECS
 * world
 */
void vecsWorldFree(VecsWorld *worldPtr){
    /* does not free the component metadata */

    /*
     * handle orders so we don't have to deal with
     * manually clearing out the order queues
     */
    vecsWorldHandleOrders(worldPtr);

    arrayListApply(_VecsArchetype,
        &(worldPtr->_archetypeList),
        _vecsArchetypeFree
    );
    arrayListFree(_VecsArchetype,
        &(worldPtr->_archetypeList)
    );

    arrayListApply(VecsQuery,
        &(worldPtr->_queryList),
        vecsQueryFree
    );
    arrayListFree(VecsQuery, &(worldPtr->_queryList));

    hashMapFree(VecsComponentSet, size_t,
        &(worldPtr->_archetypeIndexMap)
    );

    hashMapFree(VecsComponentSetPair, size_t,
        &(worldPtr->_queryIndexMap)
    );

    _vecsEntityListFree(&(worldPtr->_entityList));

    /*
     * since orders were ran earlier, simply free
     * the queues
     */
    assertTrue(
        arrayListIsEmpty(
            &(worldPtr->_addComponentQueue)
        ),
        "error: expect addComponentQueue to be empty; "
        SRC_LOCATION
    );
    assertTrue(
        arrayListIsEmpty(
            &(worldPtr->_setComponentQueue)
        ),
        "error: expect setComponentQueue to be empty; "
        SRC_LOCATION
    );
    assertTrue(
        arrayListIsEmpty(
            &(worldPtr->_removeComponentQueue)
        ),
        "error: expect removeComponentQueue to be "
        "empty; "
        SRC_LOCATION
    );
    assertTrue(
        arrayListIsEmpty(
            &(worldPtr->_addEntityQueue)
        ),
        "error: expect addEntityQueue to be empty; "
        SRC_LOCATION
    );
    assertTrue(
        arrayListIsEmpty(
            &(worldPtr->_removeEntityQueue)
        ),
        "error: expect removeEntityQueue to be empty; "
        SRC_LOCATION
    );
    arrayListFree(AddComponentOrder,
        &(worldPtr->_addComponentQueue)
    );
    arrayListFree(SetComponentOrder,
        &(worldPtr->_setComponentQueue)
    );
    arrayListFree(RemoveComponentOrder,
        &(worldPtr->_removeComponentQueue)
    );
    arrayListFree(AddEntityOrder,
        &(worldPtr->_addEntityQueue)
    );
    arrayListFree(RemoveEntityOrder,
        &(worldPtr->_removeEntityQueue)
    );
}