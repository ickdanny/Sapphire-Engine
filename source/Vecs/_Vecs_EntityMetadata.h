#ifndef VECS_ENTITYMETADATA_H
#define VECS_ENTITYMETADATA_H

#include <stdlib.h>

#include "Vecs_Entity.h"
#include "Vecs_Component.h"

/*
 * Forward declaration for archetype needed to have
 * entity metadata store a pointer to the enclosing
 * archetype
 */
struct _VecsArchetype;

/* Represents the current state of a single entity */
typedef struct _VecsEntityMetadata{
    VecsComponentSet _componentSet;

    struct _VecsArchetype *_archetypePtr;

    /* the index of the entity in its archetype */
    size_t *_indexInArchetype;

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