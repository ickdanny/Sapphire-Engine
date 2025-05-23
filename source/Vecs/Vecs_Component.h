#ifndef VECS_COMPONENT_H
#define VECS_COMPONENT_H

#include <stdint.h>

//todo: make the common case fast by adding a componentID type
// which is just the numeric value i.e. 0-63

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

/* Calculates the set union of two component sets */
#define vecsComponentSetUnion(left, right) \
    (left | right)

/*
 * Calculates the set intersection of two component
 * sets
 */
#define vecsComponentSetIntersection(left, right) \
    (left & right)

/*
 * Calculates the set difference of two component sets
 */
#define vecsComponentSetDifference(left, right) \
    (left & (~right))

/*
 * Returns true if the first component set contains any
 * of the components specified in the second, false
 * otherwise
 */
#define vecsComponentSetContainsAny(left, right) \
    (!!vecsComponentSetIntersection(left, right))

/*
 * Returns true if the first component set contains all
 * of the components specified in the second, false
 * otherwise
 */
#define vecsComponentSetContainsAll \
    (right == \
        vecsComponentSetIntersection(left, right))

#endif