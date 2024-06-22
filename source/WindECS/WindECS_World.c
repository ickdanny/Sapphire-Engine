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
} AddEntityOrder;

/*
 * frees the memory associated with the given
 * AddEntityOrder
 */
static void addEntityOrderFree(
    AddEntityOrder *orderPtr
){
    /*
     * the component list was 1-level copied during
     * queueing, and, since this function is to
     * be called only when the entity has been
     * moved to its archetype, the deeper data
     * is not our responsibility to free. Thus,
     * we only have to free the arraylist itself
     */
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
 * Returns a new query iterator; error if the accept
 * and reject sets intersect; does not take ownership
 * of the arguments
 */
WindQueryItr windWorldRequestQueryItr(
    WindWorld *worldPtr,
    Bitset *acceptComponentSetPtr,
    Bitset *rejectComponentSetPtr
){
    if(!worldPtr || !acceptComponentSetPtr){
        //todo
    }
    //todo if reject bitset empty, use the empty bitset
    //todo: hashmap only makes shallow copies, so 
    //when inserting we need to make deep copies
}

/*
 * Creates and returns a handle to the entity of the
 * specified ID
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
 * ID is alive (ignoring generation), false otherwise
 */
bool windWorldIsIDAlive(
    WindWorld *worldPtr,
    WindEntityIDType entityID
);

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
 * ID contains a component of the specified ID, false
 * otherwise (including if entity is dead)
 */
bool _windWorldIDContainsComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
);

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
 * Adds the given component to the entity specified
 * by the given handle; returns true if successful,
 * false otherwise
 */
bool _windWorldHandleAddComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Adds the given component to the entity specified
 * by the given ID; returns true if successful,
 * false otherwise
 */
bool _windWorldIDAddComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID,
    void *componentPtr
);

/*
 * Queues an order to add the given component to the
 * entity specified by the given handle
 */
bool _windWorldHandleQueueAddComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Sets the given component to the entity specified
 * by the given handle; returns true if successful,
 * false otherwise
 */
bool _windWorldHandleSetComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Sets the given component to the entity specified
 * by the given ID; returns true if successful,
 * false otherwise
 */
bool _windWorldIDSetComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID,
    void *componentPtr
);

/*
 * Queues an order to set the given component of the
 * entity specified by the given handle to the 
 * provided value
 */
bool _windWorldHandleQueueSetComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle,
    void *componentPtr
);

/*
 * Removes the specified component from the entity
 * specified by the given handle; returns true if
 * successful, false otherwise
 */
bool _windWorldHandleRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
);

/*
 * Removes the specified component from the entity
 * specified by the given ID; returns true if
 * successful, false otherwise
 */
bool _windWorldIDRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
);

/*
 * Queues an order to remove the specified component
 * from the entity specified by the given handle
 */
bool _windWorldHandleQueueRemoveComponent(
    WindWorld *worldPtr,
    WindComponentIDType componentID,
    WindEntity handle
);

/* Used for adding entities */
typedef struct WindComponentDataPair{
    WindComponentIDType componentID;

    /*
     * ptr to the component which will be copied into
     * the ECS world
     */
    void *componentPtr;
} WindComponentDataPair;

/*
 * Adds the specified entity to the given ECS world
 * and returns its handle; takes ownership of the
 * provided components and frees the component list
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
 * shallow copied so the user must still free their
 * version of it
 */
WindEntity windWorldQueueAddEntity(
    WindWorld *worldPtr,
    ArrayList *componentDataPairListPtr
);

/*
 * Removes the entity specified by the given handle
 * from the given ECS world; returns true if
 * successful, false otherwise
 */
bool windWorldHandleRemoveEntity(
    WindWorld *worldPtr,
    WindEntity handle
);

/*
 * Removes the entity specified by the given ID
 * from the given ECS world; returns true if
 * successful, false otherwise
 */
bool windWorldIDRemoveEntity(
    WindWorld *worldPtr,
    WindEntityIDType entityID
);

/*
 * Queues an order to remove the entity specified by
 * the given handle from the given ECS world
 */
void windWorldQueueHandleRemoveEntity(
    WindWorld *worldPtr,
    WindEntity handle
);

/*
 * Handles all queued orders given to the ECS world
 * since the last time this function was called
 */
void windWorldHandleOrders(WindWorld *worldPtr){
    //todo: handle orders
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