#ifndef VECS_ENTITY_H
#define VECS_ENTITY_H

#include <stdint.h>

/*
 * An entity contains both id and generation;
 * == and != are legal operations on entities
 */
typedef uint64_t VecsEntity;

/* Returns the id of the given entity */
#define vecsEntityID(entity) \
    /* bits 0-31 */ \
    (entity & 0xFFFFFFFFULL)

/* Returns the generation of the given entity */
#define vecsEntityGeneration(entity) \
    /* bits 32-47 */ \
    ((entity >> 32) & 0xFFFFULL)

#endif