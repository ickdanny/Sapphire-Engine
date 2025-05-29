#ifndef VECS_ENTITY_H
#define VECS_ENTITY_H

#include <stdint.h>

/*
 * An entity contains both id and generation;
 * == and != are legal operations on entities
 * 
 * Layout:
 *      0  - 31 : entity id
 *      32 - 47 : generation
 *      48      : live flag
 */
typedef uint64_t VecsEntity;

#define idMask 0xFFFFFFFFULL
#define generationMask 0xFFFF00000000ULL
#define liveMask 0x1000000000000ULL

/*
 * Returns a new entity with specified id and
 * generation; the entity will not be flagged as live
 */
#define vecsEntityMake(entityId, generation) \
    ((((VecsEntity)entityId) & idMask) \
        | ((((VecsEntity)generation) << 32) \
            & generationMask))

/* Returns the id of the given entity */
#define vecsEntityId(entity) \
    /* bits 0-31 */ \
    (entity & idMask)

/* Returns the generation of the given entity */
#define vecsEntityGeneration(entity) \
    /* bits 32-47 */ \
    ((entity & generationMask) >> 32)

/*
 * Returns true if the entity is live, false otherwise
 */
#define vecsEntityIsLive(entity) (entity & liveMask)

/* Flags the given entity as live */
#define vecsEntityFlagLive(entity) (entity | liveMask)

/*
 * Increments the generation of the given entity and
 * sets the entity as not live; all other metadata
 * is lost
 */
#define vecsEntityIncrementGeneration(entity) \
    vecsEntityMake( \
        vecsEntityId(entity), \
        vecsEntityGeneration(entity) + 1 \
    )

#endif