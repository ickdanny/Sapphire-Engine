#ifndef VECS_ENTITYMETADATA_H
#define VECS_ENTITYMETADATA_H

#include "Vecs_Entity.h"
#include "Vecs_Component.h"

/* Represents the current state of a single entity */
typedef struct _VecsEntityMetadata{
    VecsComponentSet _componentSet;

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

/*
 * Advances the specified entity metadata to the next
 * generation
 */
void _vecsEntityMetadataIncrementGeneration(
    _VecsEntityMetadata *metadataPtr
);

/*
 * Frees the memory associated with the specified
 * entity metadata
 */
void _vecsEntityMetadataFree(
    _VecsEntityMetadata *metadataPtr
);

#endif