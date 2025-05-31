#ifndef VECS_ENTITYMETADATA_H
#define VECS_ENTITYMETADATA_H

#include "Vecs_Entity.h"
#include "Vecs_Component.h"

/* Represents the current state of a single entity */
typedef struct _VecsEntityMetadata{
    VecsComponentSet _componentSet;

    //todo: possibly ptr to archetype

    /* Used to detect generational differences */
    VecsEntity _canonicalEntity;
} _VecsEntityMetadata;

/*
 * Constructs and returns a new entity metadata object
 * by value
 */
_VecsEntityMetadata _vecsEntityMetadataMake(
    VecsEntity entityId
);

/* Flags the entity metadata as live */
void _vecsEntityMetadataFlagLive(
    _VecsEntityMetadata *metadataPtr
);

/*
 * Advances the specified entity metadata to the next
 * generation and marks the entity as not live
 */
void _vecsEntityMetadataIncrementGeneration(
    _VecsEntityMetadata *metadataPtr
);

#endif