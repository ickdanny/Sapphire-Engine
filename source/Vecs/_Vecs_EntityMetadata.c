#include "_Vecs_EntityMetadata.h"

/*
 * Constructs and returns a new entity metadata object
 * by value
 */
_VecsEntityMetadata _vecsEntityMetadataMake(
    VecsEntity entityId
){
    return (_VecsEntityMetadata){
        ._componentSet = vecsEmptyComponentSet,
        ._initializedComponentSet
            = vecsEmptyComponentSet,
        ._archetypePtr = NULL,
        ._indexInArchetype = SIZE_MAX,
        ._canonicalEntity = vecsEntityMake(entityId, 0)
    };
}

/* Flags the entity metadata as live */
void _vecsEntityMetadataFlagLive(
    _VecsEntityMetadata *metadataPtr
){
    metadataPtr->_canonicalEntity = vecsEntityFlagLive(
        metadataPtr->_canonicalEntity
    );
}

/*
 * Advances the specified entity metadata to the next
 * generation and marks the entity as not live
 */
void _vecsEntityMetadataIncrementGeneration(
    _VecsEntityMetadata *metadataPtr
){
    metadataPtr->_componentSet = vecsEmptyComponentSet;
    metadataPtr->_initializedComponentSet
        = vecsEmptyComponentSet;
    metadataPtr->_archetypePtr = NULL;
    metadataPtr->_indexInArchetype = SIZE_MAX;
    metadataPtr->_canonicalEntity
        = vecsEntityIncrementGeneration(
            metadataPtr->_canonicalEntity
        );
}