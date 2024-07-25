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
    toRet._generation = 1;
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
    /* to avoid generation being 0, add 2 */
    metadataPtr->_generation += 2;
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

/* Clears all entity data from the given Entities */
void _windEntitiesClear(
    _WindEntities *entitiesPtr
){
    /* clear all entity IDs from the bitset */
    bitsetClear(&(entitiesPtr->_currentEntityIDs));
    /* reset all entity metadata */
    arrayApply(_WindEntityMetadata,
        &(entitiesPtr->_entityMetadata),
        _windEntityMetadataReset
    );
    /* set entity count to 0 */
    entitiesPtr->_numEntities = 0;
    /*
     * not strictly necessary, but set next entity id
     * to 0
     */
    entitiesPtr->_nextCreatedEntityID = 0;
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
    if(entityID >= entitiesPtr->_entityMetadata.size){
        pgError("Entity ID over max; " SRC_LOCATION);
    }
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

    ++(entitiesPtr->_numEntities);
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

    --(entitiesPtr->_numEntities);
}

/*
 * Returns true if the specified entity is alive,
 * false otherwise
 */
bool _windEntitiesIsAlive(
    _WindEntities *entitiesPtr,
    WindEntity entity
){
    /* if the ID isn't alive, return false */
    if(_windEntitiesIsIDDead(
        entitiesPtr,
        entity.entityID
    )){
        return false;
    }
    /* if generation is wrong, return false */
    _WindEntityMetadata *entityMetadataPtr
        = arrayGetPtr(_WindEntityMetadata,
            &(entitiesPtr->_entityMetadata),
            entity.entityID
        );
    if(!entityMetadataPtr){
        return false;
    }
    if(entityMetadataPtr->_generation
        != entity._generation
    ){
        return false;
    }

    return true;
}

/*
 * Returns true if an entity with the specified ID is
 * currently alive, false otherwise
 */
bool _windEntitiesIsIDAlive(
    _WindEntities *entitiesPtr,
    WindEntityIDType entityID
){
    return bitsetGet(
        &(entitiesPtr->_currentEntityIDs),
        entityID
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
    if(_windEntitiesIsDead(entitiesPtr, entity)){
        return NULL;
    }
    return arrayGetPtr(
        _WindEntityMetadata,
        &(entitiesPtr->_entityMetadata),
        entity.entityID
    );
}

/*
 * Returns a pointer to the metadata for the specified
 * entity, or NULL if that entity is not alive
 */
_WindEntityMetadata *_windEntitiesIDGetMetadata(
    _WindEntities *entitiesPtr,
    WindEntityIDType entityID
){
    if(_windEntitiesIsIDDead(entitiesPtr, entityID)){
        return NULL;
    }
    return arrayGetPtr(
        _WindEntityMetadata,
        &(entitiesPtr->_entityMetadata),
        entityID
    );
}

/*
 * Frees the memory associated with the given
 * _WindEntities
 */
void _windEntitiesFree(_WindEntities *entitiesPtr){
    bitsetFree(&(entitiesPtr->_currentEntityIDs));
    
    arrayApply(_WindEntityMetadata,
        &(entitiesPtr->_entityMetadata),
        _windEntityMetadataFree
    );
    arrayFree(_WindEntityMetadata,
        &(entitiesPtr->_entityMetadata)
    );
    memset(entitiesPtr, 0, sizeof(*entitiesPtr));
}