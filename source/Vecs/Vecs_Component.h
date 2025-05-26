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
 * A human-readable component id ranging from
 * 0 to vecsMaxComponentId (inclusive)
 */
typedef uint32_t VecsComponentId;

/* the max number of bits in a component set */
#define vecsMaxNumComponents \
    (8 * sizeof(VecsComponentSet))

/* The max component id inclusive */
#define vecsMaxComponentId (vecsMaxNumComponents - 1)

/*
 * Each defined component must have an accompanying
 * global symbol TYPENAME##ID defined
 */
#define vecsComponentGetID(TYPENAME) TYPENAME##ID

/* * * * * * * * * * * * * * * * * * * * * * *
 *                                           *
 * Component set to component set operations *
 *                                           *
 * * * * * * * * * * * * * * * * * * * * * * */

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
#define vecsComponentSetContainsAll(left, right) \
    (right == \
        vecsComponentSetIntersection(left, right))

/* * * * * * * * * * * * * * * * * * * * * * *
 *                                           *
 * Component set to component id operations  *
 *                                           *
 * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Converts the given component id to a component set
 * containing exactly that component
 */
#define vecsComponentSetFromId(id) (0x1ULL << id)

/*
 * Adds the specified component id to the given
 * component set
 */
#define vecsComponentSetAddId(set, id) \
    vecsComponentSetUnion( \
        set, \
        vecsComponentSetFromId(id) \
    )

/*
 * Removes the specified component id from the given
 * component set
 */
#define vecsComponentSetRemoveId(set, id) \
    vecsComponentSetDifference( \
        set, \
        vecsComponentSetFromId(id) \
    )

/*
 * Returns true if the specified component set contains
 * the given component id, false otherwise
 */
#define vecsComponentSetContainsId(set, id) \
    vecsComponentSetContainsAll( \
        set, \
        vecsComponentSetFromId(id) \
    )

#endif