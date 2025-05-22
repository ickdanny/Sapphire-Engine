#ifndef VECS_COMPONENT_H
#define VECS_COMPONENT_H

#include <stdint.h>

/*
 * VECS supports up to 64 components, each one having
 * id of the form 2^k
 */
typedef uint64_t VecsComponentSet;

/*
 * Each defined component must have an accompanying
 * global symbol TYPENAME##ID defined
 */
#define vecsComponentGetID(TYPENAME) TYPENAME##ID

#endif