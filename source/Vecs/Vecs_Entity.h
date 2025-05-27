#ifndef VECS_ENTITY_H
#define VECS_ENTITY_H

#include <stdint.h>

/*
 * An entity contains both id and generation;
 * == and != are legal operations on entities
 * 
 * Layout:
 *      0  - 31: entity id
 *      32 - 47: generation
 */
typedef uint64_t VecsEntity;

/*
 * Returns a new entity with specified id and
 * generation
 */
#define vecsEntityMake(entityId, generation) \
    ((((VecsEntity)entityId) & 0xFFFFFFFFULL) \
        | ((((VecsEntity)generation) & 0xFFFFULL) \
            << 32))

/* Returns the id of the given entity */
#define vecsEntityId(entity) \
    /* bits 0-31 */ \
    (entity & 0xFFFFFFFFULL)

/* Returns the generation of the given entity */
#define vecsEntityGeneration(entity) \
    /* bits 32-47 */ \
    ((entity >> 32) & 0xFFFFULL)

/* Increments the generation of the given entity */
#define vecsEntityIncrementGeneration(entity) \
    vecsEntityMake( \
        vecsEntityId(entity), \
        vecsEntityGeneration(entity) + 1 \
    )

#endif