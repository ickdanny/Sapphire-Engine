#include "WindECS_World.h"

#define archetypeListInitCapacity 50
#define queryListInitCapacity 50

/* A pair of bitsets used for query mapping */
typedef struct BitsetPair{
    Bitset a;
    Bitset b;
} BitsetPair;

/*
 * Frees the memory associated with the given
 * BitsetPair
 */
static void bitsetPairFree(BitsetPair *bitsetPairPtr){
    bitsetFree(&(bitsetPairPtr->a));
    bitsetFree(&(bitsetPairPtr->b));
}

/* Hash function for bitset pair */
static size_t bitsetPairHash(
    const void *bitsetPairPtr)
{
    BitsetPair *castPtr = (BitsetPair*)bitsetPairPtr;
    size_t hash = bitsetHash(&(castPtr->a));
    hash *= 7;
    hash += bitsetHash(&(castPtr->b));
    return hash;
}

/* Equals function for bitset pair */
static bool bitsetPairEquals(
    const void *bitsetPairPtr1,
    const void *bitsetPairPtr2
){
    BitsetPair *castPtr1 = (BitsetPair*)bitsetPairPtr1;
    BitsetPair *castPtr2 = (BitsetPair*)bitsetPairPtr2;
    return bitsetEquals(
        &(castPtr1->a),
        &(castPtr2->a)
    ) && bitsetEquals(
        &(castPtr1->b),
        &(castPtr2->b)
    );
}

/* an order to add a component to an entity */
typedef struct AddComponentOrder{
    WindEntity handle;
    WindComponentIDType componentID;
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
    WindEntity handle;
    WindComponentIDType componentID;
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
    WindEntity handle;
    WindComponentIDType componentID;
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
    arrayListFree(WindComponentDataPair,
        &(orderPtr->componentDataPairList)
    );
}

/* an order to remove an entity */
typedef struct RemoveEntityOrder{
    WindEntity handle;
} RemoveEntityOrder;

/*
 * Constructs and returns a new ECS world by value;
 * takes ownership of the given WindComponents
 */
WindWorld windWorldMake(
    size_t entityCapacity,
    WindComponents *componentsPtr
){
    WindWorld toRet = {0};
    toRet._archetypeList = arrayListMake(
        _WindArchetype,
        archetypeListInitCapacity
    );
    toRet._queryList = arrayListMake(
        WindQuery,
        queryListInitCapacity
    );
    toRet._archetypeIndexMap = hashMapMake(
        Bitset,
        size_t,
        archetypeListInitCapacity * 2,
        bitsetHash,
        bitsetEquals
    );
    toRet._queryIndexMap = hashMapMake(
        BitsetPair,
        size_t,
        queryListInitCapacity * 2,
        bitsetPairHash,
        bitsetPairEquals
    );
    toRet._entities = _windEntitiesMake(
        entityCapacity,
        windComponentsNumComponents(componentsPtr)
    );

    /* take ownership of given wind components */
    toRet._components = *componentsPtr;
    memset(componentsPtr, 0, sizeof(*componentsPtr));
    
    toRet._addComponentQueue = arrayListMake(
        AddComponentOrder,
        entityCapacity
    );
    toRet._setComponentQueue = arrayListMake(
        SetComponentOrder,
        entityCapacity
    );
    toRet._removeComponentQueue = arrayListMake(
        RemoveComponentOrder,
        entityCapacity
    );
    toRet._addEntityQueue = arrayListMake(
        AddEntityOrder,
        entityCapacity
    );
    toRet._removeEntityQueue = arrayListMake(
        RemoveEntityOrder,
        entityCapacity
    );

    return toRet;
}

/*
 * Initializes a new query and inserts it into the
 * given ECS world; error if a mapping
 * for the given BitsetPair already exists; takes
 * ownership of the bitset pair passed in
 */
static WindQuery *windWorldInsertQuery(
    WindWorld *worldPtr,
    BitsetPair *bitsetPairPtr
){
    assertFalse(
        hashMapHasKeyPtr(BitsetPair, size_t,
            &(worldPtr->_queryIndexMap),
            bitsetPairPtr
        ),
        "error: trying to insert query when mapping "
        "already exists; "
        SRC_LOCATION
    );
    /*
     * if the reject component set is empty, pass
     * NULL instead
     */
    Bitset *rejectComponentSetPtrToPass
        = &(bitsetPairPtr->b);
    if(rejectComponentSetPtrToPass 
        && bitsetNone(rejectComponentSetPtrToPass)
    ){
        rejectComponentSetPtrToPass = NULL;
    }
    /*
     * query ctor will automatically try accept all
     * archetypes
     */
    WindQuery newQuery = windQueryMake(
        &(bitsetPairPtr->a),
        rejectComponentSetPtrToPass,
        &(worldPtr->_archetypeList)
    );
    arrayListPushBack(
        WindQuery,
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
    hashMapPutPtr(BitsetPair, size_t,
        &(worldPtr->_queryIndexMap),
        bitsetPairPtr,
        &lastQueryListIndex
    );
    
    return arrayListBackPtr(WindQuery,
        &(worldPtr->_queryList)
    );
}

/*
 * Returns a new query iterator; error if the accept
 * and reject sets intersect; does not take ownership
 * of the arguments
 */
WindQueryItr windWorldRequestQueryItr(
    WindWorld *worldPtr,
    Bitset *acceptComponentSetPtr,
    Bitset *rejectComponentSetPtr
){
    assertNotNull(
        worldPtr,
        "null passed; " SRC_LOCATION
    );
    assertNotNull(
        acceptComponentSetPtr,
        "accept component set cannot be null; "
        SRC_LOCATION
    );
    BitsetPair bitsetPair = {0};
    bitsetPair.a = bitsetCopy(acceptComponentSetPtr);
    if(rejectComponentSetPtr){
        bitsetPair.b = bitsetCopy(
            rejectComponentSetPtr
        );
    }
    /* if reject bitset empty, use empty bitset */
    else{
        bitsetPair.b = bitsetMake(1);
    }
    WindQuery *queryPtr = 0;
    /*
     * if hash map has key, get it and then free the
     * bitset pair
     */
    if(hashMapHasKeyPtr(BitsetPair, size_t,
        &(worldPtr->_queryIndexMap),
        &bitsetPair
    )){
        size_t queryIndex = *hashMapGetPtr(
            BitsetPair,
            size_t,
            &(worldPtr->_queryIndexMap),
            &bitsetPair
        );
        queryPtr = arrayListGetPtr(WindQuery,
            &(worldPtr->_queryList),
            queryIndex
        );
        /*
         * free since the hashmap doesn't take
         * ownership if it already contained
         */
        bitsetPairFree(&bitsetPair);
    }
    /*
     * if hash map does not have key, make new query,
     * insert it into the query list, and then get
     * a pointer to it
     */
    else{
        /*
         * the bitset pair is now owned by the
         * query index map
         */
        queryPtr = windWorldInsertQuery(
            worldPtr,
            &bitsetPair
        );
    }
    return windQueryItr(queryPtr);
}

/*
 * Creates and returns a handle to the entity of the
 * specified ID; error if the ID is currently dead
 */
WindEntity windWorldMakeHandle(
    WindWorld *worldPtr,
    WindEntityIDType entityID
){
    assertFalse(
        _windEntitiesIsIDDead(
            &(worldPtr->_entities),
            entityID
        ),
        "error: try to make handle of dead entityID; "
        SRC_LOCATION
    );
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesIDGetMetadata(
            &(worldPtr->_entities),
            entityID
        );
    WindEntity toRet = {
        entityID,
        entityMetadataPtr->_generation
    };
    return toRet;
}

/*
 * Returns true if the entity described by the given
 * entity handle is alive, false otherwise
 */
bool windWorldIsHandleAlive(
    WindWorld *worldPtr,
    WindEntity handle
){
    return _windEntitiesIsAlive(
        &(worldPtr->_entities),
        handle
    );
}

/*
 * Returns true if the entity described by the given
 * ID is alive (ignoring generation), false otherwise
 */
bool windWorldIsIDAlive(
    WindWorld *worldPtr,
    WindEntityIDType entityID
){
    return _windEntitiesIsIDAlive(
        &(worldPtr->_entities),
        entityID
    );
}

/*
 * Returns true if the entity described by the given
 * entity handle contains a component of the specified
 * ID, false otherwise (including if entity is dead)
 */
bool _windWorldHandleContainsComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
){
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesGetMetadata(
            &(worldPtr->_entities),
            handle
        );
    return bitsetGet(
        &(entityMetadataPtr->_componentSet),
        componentID
    );
}

/*
 * Returns true if the entity described by the given
 * ID contains a component of the specified ID, false
 * otherwise (including if entity is dead)
 */
bool _windWorldIDContainsComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
){
    if(windWorldIsIDDead(worldPtr, entityID)){
        return false;
    }
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesIDGetMetadata(
            &(worldPtr->_entities),
            entityID
        );
    return bitsetGet(
        &(entityMetadataPtr->_componentSet),
        componentID
    );
}

/*
 * Returns a pointer to the archetype of the entity
 * specified by the given handle; error if the entity
 * is dead
 */
static _WindArchetype *windWorldHandleGetArchetype(
    WindWorld *worldPtr,
    WindEntity handle
){
    assertTrue(
        windWorldIsHandleAlive(
            worldPtr,
            handle
        ),
        "error: try to get archetype of dead entity; "
        SRC_LOCATION
    );
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesGetMetadata(
            &(worldPtr->_entities),
            handle
        );
    size_t index = *hashMapGetPtr(Bitset, size_t,
        &(worldPtr->_archetypeIndexMap),
        &(entityMetadataPtr->_componentSet)
    );
    assertTrue(
        index < worldPtr->_archetypeIndexMap.size,
        "error: archetype index out of bounds; "
        SRC_LOCATION
    );
    return arrayListGetPtr(_WindArchetype,
        &(worldPtr->_archetypeList),
        index
    );
}

/*
 * Initializes a new archetype and inserts it into the
 * given ECS world; error if a mapping for the given
 * Bitset already exists; does not take ownership of
 * the bitset passed in
 */
static _WindArchetype *windWorldInsertArchetype(
    WindWorld *worldPtr,
    Bitset *bitsetPtr
){
    assertFalse(
        hashMapHasKeyPtr(Bitset, size_t,
            &(worldPtr->_archetypeIndexMap),
            bitsetPtr
        ),
        "error: trying to insert archetype when "
        "mapping already exists; "
        SRC_LOCATION
    );
    /* archetype ctor makes a copy of the bitset */
    _WindArchetype newArchetype = _windArchetypeMake(
        bitsetPtr,
        worldPtr->_entities._numEntities,
        &(worldPtr->_components)
    );
    arrayListPushBack(_WindArchetype,
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
        windQueryTryAcceptArchetype(
            arrayListGetPtr(WindQuery,
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

    /* make a copy of the bitset for the hashmap */
    Bitset copyBitset = bitsetCopy(bitsetPtr);
    /*
     * map the bitset to the last list index;
     * the bitset is now owned by the hashmap
     */
    hashMapPutPtr(Bitset, size_t,
        &(worldPtr->_archetypeIndexMap),
        &copyBitset,
        &lastArchetypeListIndex
    );
    
    return arrayListBackPtr(_WindArchetype,
        &(worldPtr->_archetypeList)
    );
}

/*
 * Returns a pointer to the archetype of the entity
 * specified by the given ID; error if the entity
 * is dead
 */
static _WindArchetype *windWorldIDGetArchetype(
    WindWorld *worldPtr,
    WindEntityIDType entityID
){
    assertTrue(
        windWorldIsIDAlive(
            worldPtr,
            entityID
        ),
        "error: try to get archetype of dead entity; "
        SRC_LOCATION
    );
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesIDGetMetadata(
            &(worldPtr->_entities),
            entityID
        );
    size_t *indexPtr = hashMapGetPtr(Bitset, size_t,
        &(worldPtr->_archetypeIndexMap),
        &(entityMetadataPtr->_componentSet)
    );
    size_t index = 0;
    /*
     * if failed to retrieve index ptr, make a new
     * archetype for the entity
     */
    if(!indexPtr){
        index = worldPtr->_archetypeIndexMap.size;
        windWorldInsertArchetype(
            worldPtr,
            &(entityMetadataPtr->_componentSet)
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
    return arrayListGetPtr(_WindArchetype,
        &(worldPtr->_archetypeList),
        index
    );
}

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
){
    /* return NULL if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return NULL;
    }
    /* error if component ID is invalid */
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesGetMetadata(
            &(worldPtr->_entities),
            handle
        );
    assertTrue(
        bitsetGet(
            &(entityMetadataPtr->_componentSet),
            componentID
        ),
        "error: trying to get component entity lacks; "
        SRC_LOCATION
    );
    /* return NULL if component is marker */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            &(worldPtr->_components),
            componentID
        );
    if(componentMetadata._componentSize == 0){
        return NULL;
    }

    /* get ptr to archetype */
    _WindArchetype *archetypePtr
        = windWorldHandleGetArchetype(
            worldPtr,
            handle
        );
    /* ask archetype to get a ptr to the component */
    return __windArchetypeGetPtr(
        archetypePtr,
        componentID,
        handle.entityID
    );
}

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
){
    /* return NULL if entity dead */
    if(windWorldIsIDDead(worldPtr, entityID)){
        return NULL;
    }
    /* error if component ID is invalid */
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesIDGetMetadata(
            &(worldPtr->_entities),
            entityID
        );
    assertTrue(
        bitsetGet(
            &(entityMetadataPtr->_componentSet),
            componentID
        ),
        "error: trying to get component entity lacks; "
        SRC_LOCATION
    );
    /* return NULL if component is marker */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            &(worldPtr->_components),
            componentID
        );
    if(componentMetadata._componentSize == 0){
        return NULL;
    }

    /* get ptr to archetype */
    _WindArchetype *archetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* ask archetype to get a ptr to the component */
    return __windArchetypeGetPtr(
        archetypePtr,
        componentID,
        entityID
    );
}

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
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    return _windWorldIDAddComponent(
        worldPtr,
        componentID,
        handle.entityID,
        componentPtr
    );
}

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
){
    /* return false if entity dead */
    if(windWorldIsIDDead(worldPtr, entityID)){
        return false;
    }
    /* return false if entity already has component */
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesIDGetMetadata(
            &(worldPtr->_entities),
            entityID
        );
    if(bitsetGet(
        &(entityMetadataPtr->_componentSet),
        componentID
    )){
        return false;
    }
    /* get old archetype */
    _WindArchetype *oldArchetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* update component set */
    bitsetSet(
        &(entityMetadataPtr->_componentSet),
        componentID
    );
    /* get new archetype */
    _WindArchetype *newArchetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* move components from old archetype to new */
    _windArchetypeMoveEntity(
        oldArchetypePtr,
        entityID,
        newArchetypePtr
    );
    /*
     * add component to the entity in the new 
     * archetype (as long as it is not a marker)
     */
    __windArchetypeSetPtr(
        newArchetypePtr,
        componentID,
        entityID,
        componentPtr
    );
    
    return true;
}

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
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    WindComponentMetadata componentMetadata
        = windComponentsGet(
               &(worldPtr->_components),
               componentID
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
        handle,
        componentID,
        heapCopy
    };
    arrayListPushBack(AddComponentOrder,
        &(worldPtr->_addComponentQueue),
        order
    );
    return true;
}

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
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    return _windWorldIDSetComponent(
        worldPtr,
        componentID,
        handle.entityID,
        componentPtr
    );
}

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
){
    /* return false if entity dead */
    if(windWorldIsIDDead(worldPtr, entityID)){
        return false;
    }
    _WindArchetype *oldArchetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* if entity has component, replace it */
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesIDGetMetadata(
            &(worldPtr->_entities),
            entityID
        );
    if(bitsetGet(
        &(entityMetadataPtr->_componentSet),
        componentID
    )){
        WindComponentMetadata componentMetadata
            = windComponentsGet(
                &(worldPtr->_components),
                componentID
            );
        /* do nothing if component is marker */
        if(componentMetadata._componentSize == 0){
            return true;
        }
        void *componentPtrIntoArchetype
            = __windArchetypeGetPtr(
                oldArchetypePtr,
                componentID,
                entityID
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
    /* update component set */
    bitsetSet(
        &(entityMetadataPtr->_componentSet),
        componentID
    );
    /* get new archetype */
    _WindArchetype *newArchetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* move components from old archetype to new */
    _windArchetypeMoveEntity(
        oldArchetypePtr,
        entityID,
        newArchetypePtr
    );
    /*
     * add component to the entity in the new 
     * archetype (as long as it is not a marker)
     */
    __windArchetypeSetPtr(
        newArchetypePtr,
        componentID,
        entityID,
        componentPtr
    );
    
    return true;
}

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
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    WindComponentMetadata componentMetadata
        = windComponentsGet(
               &(worldPtr->_components),
               componentID
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
        handle,
        componentID,
        heapCopy
    };
    arrayListPushBack(SetComponentOrder,
        &(worldPtr->_setComponentQueue),
        order
    );
    return true;
}

/*
 * Removes the specified component from the entity
 * specified by the given handle; returns true if
 * successful, false otherwise
 */
bool _windWorldHandleRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    return _windWorldIDRemoveComponent(
        worldPtr,
        componentID,
        handle.entityID
    );
}

/*
 * Removes the specified component from the entity
 * specified by the given ID; returns true if
 * successful, false otherwise
 */
bool _windWorldIDRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
){
    /* return false if entity dead */
    if(windWorldIsIDDead(worldPtr, entityID)){
        return false;
    }
    /* return false if entity doesn't have component */
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesIDGetMetadata(
            &(worldPtr->_entities),
            entityID
        );
    if(!bitsetGet(
        &(entityMetadataPtr->_componentSet),
        componentID
    )){
        return false;
    }
    /* get old archetype */
    _WindArchetype *oldArchetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* run destructor if needed */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            &(worldPtr->_components),
            componentID
        );
    if(componentMetadata._componentSize != 0
        && componentMetadata._destructor
    ){
        void *componentPtrIntoArchetype
            = __windArchetypeGetPtr(
                oldArchetypePtr,
                componentID,
                entityID
            );
        componentMetadata._destructor(
            componentPtrIntoArchetype
        );
    }
    /* update component set */
    bitsetUnset(
        &(entityMetadataPtr->_componentSet),
        componentID
    );
    /* get new archetype */
    _WindArchetype *newArchetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* move components from old archetype to new */
    _windArchetypeMoveEntity(
        oldArchetypePtr,
        entityID,
        newArchetypePtr
    );
    return true;
}

/*
 * Queues an order to remove the specified component
 * from the entity specified by the given handle
 */
bool _windWorldHandleQueueRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    /* queue up the order */
    RemoveComponentOrder order = {
        handle,
        componentID
    };
    arrayListPushBack(RemoveComponentOrder,
        &(worldPtr->_removeComponentQueue),
        order
    );
    return true;
}

/*
 * Adds the specified entity to the given ECS world
 * and returns its handle; takes ownership of the
 * provided components but does not free the component
 * list
 */
WindEntity windWorldAddEntity(
    WindWorld *worldPtr,
    ArrayList *componentDataPairListPtr
){
    /* generate a new entity */
    WindEntity handle = _windEntitiesCreate(
        &(worldPtr->_entities)
    );
    /* grab entity metadata */
    _WindEntityMetadata *entityMetadataPtr
        = _windEntitiesGetMetadata(
            &(worldPtr->_entities),
            handle
        );
    assertNotNull(
        entityMetadataPtr,
        "error: failed to get entity metadata of new "
        "entity; " SRC_LOCATION
    );
    /* toggle the component set for each component */
    WindComponentIDType componentID = 0;
    for(size_t i = 0;
        i < componentDataPairListPtr->size;
        ++i
    ){
        componentID = arrayListGet(
            WindComponentDataPair,
            componentDataPairListPtr,
            i
        ).componentID;
        assertFalse(
            bitsetGet(
                &(entityMetadataPtr->_componentSet),
                componentID
            ),
            "error: duplicate component; "
            SRC_LOCATION
        );
        bitsetSet(
            &(entityMetadataPtr->_componentSet),
            componentID
        );
    }
    /* get the entity's archetype */
    _WindArchetype *archetypePtr
        = windWorldHandleGetArchetype(
            worldPtr,
            handle
        );
    /* shallow copy each component into archetype */
    WindComponentMetadata componentMetadata = {0};
    for(size_t i = 0;
        i < componentDataPairListPtr->size;
        ++i
    ){
        componentID = arrayListGet(
            WindComponentDataPair,
            componentDataPairListPtr,
            i
        ).componentID;
        componentMetadata = windComponentsGet(
            &(worldPtr->_components),
            componentID
        );
        /* skip markers */
        if(componentMetadata._componentSize == 0){
            continue;
        }
        /* shallow copy each component from its ptr */
        __windArchetypeSetPtr(
            archetypePtr,
            componentID,
            handle.entityID,
            arrayListGet(WindComponentDataPair,
                componentDataPairListPtr,
                i
            ).componentPtr
        );
    }
    return handle;
}

/*
 * Queues an order to add the specified entity to the
 * given ECS world; takes ownership of the provided
 * components and frees the component list when
 * the order is handled - the component list will be
 * shallow copied so the user must still free their
 * version of it
 */
void windWorldQueueAddEntity(
    WindWorld *worldPtr,
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
        WindComponentDataPair,
        numComponents
    );
    /* shallow copy every component */
    WindComponentMetadata componentMetadata = {0};
    WindComponentDataPair dataPair = {0};
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
        dataPair = arrayListGet(WindComponentDataPair,
            &(worldPtr->_addEntityQueue),
            i
        );
        componentMetadata = windComponentsGet(
            &(worldPtr->_components),
            dataPair.componentID
        );
        entireComponentDataSize
            += componentMetadata._componentSize;
    }
    /* 
     * base ptr to the block with all components;
     * allocate unless 
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
        dataPair = arrayListGet(WindComponentDataPair,
            &(worldPtr->_addEntityQueue),
            i
        );
        componentMetadata = windComponentsGet(
            &(worldPtr->_components),
            dataPair.componentID
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
            currentComponentPtr = voidPtrAdd(
                currentComponentPtr,
                componentMetadata._componentSize
            );
            dataPair.componentPtr
                = currentComponentPtr;
        }
        /* push to the copy list */
        arrayListPushBack(WindComponentDataPair,
            componentDataPairListCopyPtr,
            dataPair
        );
    } /* end of for loop */
}

/*
 * Removes the entity specified by the given handle
 * from the given ECS world; returns true if
 * successful, false otherwise
 */
bool windWorldHandleRemoveEntity(
    WindWorld *worldPtr,
    WindEntity handle
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    return windWorldIDRemoveEntity(
        worldPtr,
        handle.entityID
    );
}

/*
 * Removes the entity specified by the given ID
 * from the given ECS world; returns true if
 * successful, false otherwise
 */
bool windWorldIDRemoveEntity(
    WindWorld *worldPtr,
    WindEntityIDType entityID
){
    /* return false if entity dead */
    if(windWorldIsIDDead(worldPtr, entityID)){
        return false;
    }
    /* get archetype */
    _WindArchetype *archetypePtr
        = windWorldIDGetArchetype(
            worldPtr,
            entityID
        );
    /* clear entity metadata */
    _windEntitiesReclaim(
        &(worldPtr->_entities),
        windWorldMakeHandle(worldPtr, entityID)
    );
    /* clear entity component data */
    bool success = _windArchetypeRemoveEntity(
        archetypePtr,
        entityID
    );
    assertTrue(
        success,
        "error: failed to remove entity from "
        "archetype; " SRC_LOCATION
    );
    return true;
}

/*
 * Queues an order to remove the entity specified by
 * the given handle from the given ECS world, returns
 * true if successful, false otherwise (e.g. if the
 * entity is already dead)
 */
bool windWorldQueueHandleRemoveEntity(
    WindWorld *worldPtr,
    WindEntity handle
){
    /* return false if entity dead */
    if(windWorldIsHandleDead(worldPtr, handle)){
        return false;
    }
    RemoveEntityOrder order = {handle};
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
static void windWorldHandleRemoveEntityOrders(
    WindWorld *worldPtr
){
    WindEntity handleToRemove = {0};
    for(size_t i = 0;
        i < worldPtr->_removeEntityQueue.size;
        ++i
    ){
        handleToRemove = arrayListGet(
            RemoveEntityOrder,
            &(worldPtr->_removeEntityQueue),
            i
        ).handle;
        windWorldHandleRemoveEntity(
            worldPtr,
            handleToRemove
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
static void windWorldHandleRemoveComponentOrders(
    WindWorld *worldPtr
){
    RemoveComponentOrder order = {0};
    for(size_t i = 0;
        i < worldPtr->_removeComponentQueue.size;
        ++i
    ){
        order = arrayListGet(
            RemoveComponentOrder,
            &(worldPtr->_removeComponentQueue),
            i
        );
        _windWorldHandleRemoveComponent(
            worldPtr,
            order.componentID,
            order.handle
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
static void windWorldHandleAddEntityOrders(
    WindWorld *worldPtr
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
        windWorldAddEntity(
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
static void windWorldHandleAddComponentOrders(
    WindWorld *worldPtr
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
        _windWorldHandleAddComponent(
            worldPtr,
            orderPtr->componentID,
            orderPtr->handle,
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
static void windWorldHandleSetComponentOrders(
    WindWorld *worldPtr
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
        _windWorldHandleSetComponent(
            worldPtr,
            orderPtr->componentID,
            orderPtr->handle,
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
void windWorldHandleOrders(WindWorld *worldPtr){
    windWorldHandleRemoveEntityOrders(worldPtr);
    windWorldHandleRemoveComponentOrders(worldPtr);
    windWorldHandleAddEntityOrders(worldPtr);
    windWorldHandleAddComponentOrders(worldPtr);
    windWorldHandleSetComponentOrders(worldPtr);
}

/*
 * A version of bitsetFree that conforms to the
 * HashMap keyApply interface
 */
void _bitsetFree(void *voidPtr){
    bitsetFree(voidPtr);
}

/*
 * A version of bitsetPairFree that conforms to the
 * HashMap keyApply interface
 */
void _bitsetPairFree(void *voidPtr){
    bitsetPairFree(voidPtr);
}

/*
 * Frees the memory associated with the given ECS
 * world
 */
void windWorldFree(WindWorld *worldPtr){
    /*
     * handle orders so we don't have to deal with
     * manually clearing out the order queues
     */
    windWorldHandleOrders(worldPtr);

    arrayListApply(_WindArchetype,
        &(worldPtr->_archetypeList),
        _windArchetypeFree
    );
    arrayListFree(_WindArchetype,
        &(worldPtr->_archetypeList)
    );

    arrayListApply(WindQuery,
        &(worldPtr->_queryList),
        windQueryFree
    );
    arrayListFree(WindQuery, &(worldPtr->_queryList));

    hashMapKeyApply(Bitset, size_t,
        &(worldPtr->_archetypeIndexMap),
        _bitsetFree
    );
    hashMapFree(Bitset, size_t,
        &(worldPtr->_archetypeIndexMap)
    );

    hashMapKeyApply(BitsetPair, size_t,
        &(worldPtr->_queryIndexMap),
        _bitsetPairFree
    );
    hashMapFree(BitsetPair, size_t,
        &(worldPtr->_queryIndexMap)
    );

    _windEntitiesFree(&(worldPtr->_entities));
    windComponentsFree(&(worldPtr->_components));

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