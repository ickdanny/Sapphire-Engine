#include "_WindECS_Entities.h"

/*
 * Constructs and returns a new _WindEntityMetadata
 * by value
 */
_WindEntityMetadata _windEntityMetadataMake(
    size_t numComponents
){
    _WindEntityMetadata toRet = {0};
    toRet._componentSet = bitsetMake(numComponents);
    toRet._generation = 0;
    return toRet;
}

/*
 * Prepares the state of the specified
 * _WindEntityMetadata for the next generation
 */
void _windEntityMetadataReset(
    _WindEntityMetadata *metadataPtr
){
    bitsetClear(&(metadataPtr->_componentSet));
    ++(metadataPtr->_generation);
}

/*
 * Frees the memory associated with the specified
 * _WindEntityMetadata
 */
void _windEntityMetadataFree(
    _WindEntityMetadata *metadataPtr
){
    bitsetFree(&(metadataPtr->_componentSet));
    metadataPtr->_generation = 0;
}

/*
 * Constructs and returns a new _WindEntities by value
 */
_WindEntities _windEntitiesMake(
    WindEntityIDType numEntityIDs,
    size_t numComponents
){
    _WindEntities toRet = {0};
    toRet._currentEntityIDs = bitsetMake(numEntityIDs);
    /* init all metadata */
    toRet._entityMetadata = arrayMake(
        _WindEntityMetadata,
        numEntityIDs
    );
    for(size_t i = 0; i < numEntityIDs; ++i){
        arraySet(_WindEntityMetadata,
            &(toRet._entityMetadata),
            i,
            _windEntityMetadataMake(numComponents)
        );
    }
    toRet._nextCreatedEntityID = 0;
    toRet._numEntities = 0;
    return toRet;
}

/* Creates a new entity and returns its handle */
WindEntity _windEntitiesCreate(
    _WindEntities *entitiesPtr
){
    /* error out if max entities reached */
    if(entitiesPtr->_numEntities
        >= entitiesPtr->_entityMetadata.size
    ){
        pgError("entity count max reached");
    }

    /*
     * if the next entity ID is still alive, linear
     * search the bitset for the first dead entity
     */
    if(bitsetGet(
        &(entitiesPtr->_currentEntityIDs),
        entitiesPtr->_nextCreatedEntityID
    )){
        entitiesPtr->_nextCreatedEntityID
            = bitsetFirstUnset(
                &(entitiesPtr->_currentEntityIDs)
            );
    }

    /* 
     * make the entity alive by simply setting the
     * bit; the component set was zeroed during
     * either initialization or reclaimation
     */
    WindEntityIDType entityID
        = entitiesPtr->_nextCreatedEntityID;
    bool prevBit = bitsetSet(
        &(entitiesPtr->_currentEntityIDs),
        entityID
    );
    assertFalse(
        prevBit,
        "new entity should have been prev bit false"
        SRC_LOCATION
    );

    ++(entitiesPtr->_nextCreatedEntityID);
    /* mod the next entity ID if >= num entities */
    if(entitiesPtr->_nextCreatedEntityID
        >= entitiesPtr->_entityMetadata.size
    ){
        entitiesPtr->_nextCreatedEntityID = 0;
    }

    WindEntity toRet = {0};
    toRet.entityID = entityID;
    toRet._generation = arrayGet(_WindEntityMetadata,
        &(entitiesPtr->_entityMetadata),
        entityID
    )._generation;
    return toRet;
}

/*
 * Reclaims an entity and puts its ID back into the
 * pool; error if the entity is not alive
 */
void _windEntitiesReclaim(
    _WindEntities *entitiesPtr,
    WindEntity toReclaim
){
    /* make sure generation matches */
    _WindEntityMetadata *metadataPtr = arrayGetPtr(
        _WindEntityMetadata,
        &(entitiesPtr->_entityMetadata),
        toReclaim.entityID
    );
    WindEntityGenerationType storedGeneration
        = metadataPtr->_generation;
    assertTrue(
        toReclaim._generation == storedGeneration,
        "generation mismatch for entity reclaim; "
        SRC_LOCATION
    );
    /* reset the metadata */
    _windEntityMetadataReset(metadataPtr);
    /* unset the bit representing the entity */
    bool prevBit = bitsetUnset(
        &(entitiesPtr->_currentEntityIDs),
        toReclaim.entityID
    );
    assertTrue(
        prevBit,
        "entity must be alive for reclaim; "
        SRC_LOCATION
    );
}

/*
 * Returns true if the specified entity is alive,
 * false otherwise
 */
bool _windEntitiesIsAlive(
    _WindEntities *entitiesPtr,
    WindEntity entity
){
    return bitsetGet(
        &(entitiesPtr->_currentEntityIDs),
        entity.entityID
    );
}

/*
 * Returns a pointer to the metadata for the specified
 * entity, or NULL if that entity is not alive
 */
_WindEntityMetadata *_windEntitiesGetMetadata(
    _WindEntities *entitiesPtr,
    WindEntity entity
){
    return arrayGetPtr(
        _WindEntityMetadata,
        &(entitiesPtr->_entityMetadata),
        entity.entityID
    );
}