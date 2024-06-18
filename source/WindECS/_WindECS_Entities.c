#include "_WindECS_Entities.h"

/*
 * Constructs and returns a new _WindEntities by value
 */
_WindEntities _windEntitiesMake(
    WindEntityIDType numEntityIDs
){
    _WindEntities toRet = {0};
    toRet._currentEntityIDs = bitsetMake(numEntityIDs);
    toRet._entityMetadata = arrayMake(
        _WindEntityMetadata,
        numEntityIDs
    );
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

    //todo: bitset should have firstUnset function
}

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
 * Returns a pointer to the metadata for the specified
 * entity, or NULL if that entity is not alive
 */
_WindEntityMetadata *_windEntitiesGetMetadata(
    _WindEntities *entitiesPtr,
    WindEntity entity
);