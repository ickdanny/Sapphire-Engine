#include "Vecs_EntityMetadata.h"

/*
 * Constructs and returns a new entity metadata object
 * by value
 */
_VecsEntityMetadata _vecsEntityMetadataMake(
    VecsEntity entityId
){
    return (_VecsEntityMetadata){
        ._componentSet = vecsEmptyComponentSet,
        ._canonicalEntity = vecsEntityMake(entityId, 0)
    };
}

/*
 * Advances the specified entity metadata to the next
 * generation
 */
void _vecsEntityMetadataIncrementGeneration(
    _VecsEntityMetadata *metadataPtr
){
    metadataPtr->_componentSet = vecsEmptyComponentSet;
    metadataPtr->_canonicalEntity
        = vecsEntityIncrementGeneration(
            metadataPtr->_canonicalEntity
        );
}