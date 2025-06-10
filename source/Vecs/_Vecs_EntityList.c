#include "_Vecs_EntityList.h"

/*
 * Constructs and returns a new empty entity list
 * by value
 */
_VecsEntityList _vecsEntityListMake(
    size_t maxEntities
){
    _VecsEntityList toRet = {
        ._entityMetadataArray = arrayMake(
            _VecsEntityMetadata,
            maxEntities
        ),
        ._nextCreatedEntityId = 0,
        ._numEntities = 0
    };

    /*
     * initializes each entity metadata as
     * generation 0, not live, and with id equal to
     * array index
     */
    for(size_t i = 0; i < maxEntities; ++i){
        arraySet(_VecsEntityMetadata,
            &(toRet._entityMetadataArray),
            i,
            _vecsEntityMetadataMake(i)
        );
    }

    return toRet;
}

/* Clears all entity data from the given entity list */
void _vecsEntityListClear(
    _VecsEntityList *entityListPtr
){
    arrayApply(_VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        _vecsEntityMetadataIncrementGeneration
    );
    entityListPtr->_numEntities = 0;
    entityListPtr->_nextCreatedEntityId = 0;
}

/*
 * Allocates a new entity with no components and
 * returns it
 */
VecsEntity _vecsEntityListAllocate(
    _VecsEntityList *entityListPtr
){
    /* error out if max entities reached */
    if(entityListPtr->_numEntities
        >= entityListPtr->_entityMetadataArray.size
    ){
        pgError("entity list count max reached");
    }

    /* find id of next dead entity to set live */
    if(entityListPtr->_nextCreatedEntityId
            >= entityListPtr
                ->_entityMetadataArray.size)
    {
        entityListPtr->_nextCreatedEntityId = 0;
    }
    while(vecsEntityIsLive(
        arrayGet(
            _VecsEntityMetadata,
            &(entityListPtr->_entityMetadataArray),
            entityListPtr->_nextCreatedEntityId
        )._canonicalEntity
    )){
        ++(entityListPtr->_nextCreatedEntityId);
        if(entityListPtr->_nextCreatedEntityId
            >= entityListPtr
                ->_entityMetadataArray.size)
        {
            entityListPtr->_nextCreatedEntityId = 0;
        }
    }

    //todo: what to do about archetype and index?
    _vecsEntityMetadataFlagLive(arrayGetPtr(
        _VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        entityListPtr->_nextCreatedEntityId
    ));

    VecsEntity toRet = (arrayGet(_VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        entityListPtr->_nextCreatedEntityId
    ))._canonicalEntity;

    ++(entityListPtr->_nextCreatedEntityId);
    if(entityListPtr->_nextCreatedEntityId
        >= entityListPtr
            ->_entityMetadataArray.size)
    {
        entityListPtr->_nextCreatedEntityId = 0;
    }

    ++(entityListPtr->_numEntities);
    return toRet;
}

/*
 * Reclaims an entity and puts its id back into the
 * pool; error if the entity is not live
 */
void _vecsEntityListReclaim(
    _VecsEntityList *entityListPtr,
    VecsEntity toReclaim
){
    _VecsEntityMetadata *metadataPtr = arrayGetPtr(
        _VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        vecsEntityId(toReclaim)
    );

    /* make sure generation matches */
    assertTrue(
        vecsEntityGeneration(toReclaim)
            == vecsEntityGeneration(
                metadataPtr->_canonicalEntity
            ),
        "generation mismatch for entity reclaim; "
        SRC_LOCATION
    );

    _vecsEntityMetadataIncrementGeneration(
        metadataPtr
    );
    --(entityListPtr->_numEntities);
}

/*
 * Returns true if the specified entity is live,
 * false otherwise
 */
bool _vecsEntityListIsEntityLive(
    _VecsEntityList *entityListPtr,
    VecsEntity entity
){
    if(vecsEntityIsDead(entity)){
        return false;
    }

    _VecsEntityMetadata *metadataPtr = arrayGetPtr(
        _VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        vecsEntityId(entity)
    );

    if(vecsEntityIsDead(
        metadataPtr->_canonicalEntity
    )){
        return false;
    }

    if(vecsEntityGeneration(entity)
        != vecsEntityGeneration(
            metadataPtr->_canonicalEntity
        )
    ){
        return false;
    }

    return true;
}

/*
 * Returns true if an entity with the same id as the
 * provided entity is currently live, false otherwise
 */
bool _vecsEntityListIsIdLive(
    _VecsEntityList *entityListPtr,
    VecsEntity entityId
){
    _VecsEntityMetadata *metadataPtr = arrayGetPtr(
        _VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        vecsEntityId(entityId)
    );

    return vecsEntityIsLive(
        metadataPtr->_canonicalEntity
    );
}

/*
 * Returns a pointer to the metadata for the specified
 * entity, or NULL if that entity is not live
 */
_VecsEntityMetadata *_vecsEntityListGetMetadata(
    _VecsEntityList *entityListPtr,
    VecsEntity entity
){
    if(_vecsEntityListIsEntityDead(entityListPtr, entity)){
        return NULL;
    }
    return arrayGetPtr(_VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        vecsEntityId(entity)
    );
}

/*
 * Returns a pointer to the metadata for the specified
 * entity id, or NULL if no such entity is live
 */
_VecsEntityMetadata *_vecsEntityListIdGetMetadata(
    _VecsEntityList *entityListPtr,
    VecsEntity entityId
){
    if(_vecsEntityListIsIdDead(
        entityListPtr,
        entityId
    )){
        return NULL;
    }
    return arrayGetPtr(_VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray),
        vecsEntityId(entityId)
    );
}

/*
 * Frees the memory associated with the given
 * entity list
 */
void _vecsEntityListFree(
    _VecsEntityList *entityListPtr
){
    arrayFree(_VecsEntityMetadata,
        &(entityListPtr->_entityMetadataArray)
    );
    memset(entityListPtr, 0, sizeof(*entityListPtr));
}