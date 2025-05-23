#ifndef VECS_COMPONENTLIST_H
#define VECS_COMPONENTLIST_H

#include "Constructure.h"

#include "Vecs_ComponentMetadata.h"

/* Stores RTTI for all components of an ECS world */
typedef struct VecsComponentList{
    Array _componentArray;
    /*
     * used to indicate whether a component of a
     * specific ID has already been specified
     */
    VecsComponentSet _validComponentTypes;
} VecsComponentList;

/*
 * Constructs and returns a new empty component list
 * by value
 */
VecsComponentList vecsComponentListMake();

/*
 * Inserts the specified component metadata into
 * the given component list at the specified ID; error
 * if the componentID has already been used
 */
void vecsComponentListInsert(
    VecsComponentList *componentListPtr,
    VecsComponentMetadata componentMetadata,
    VecsComponentSet componentID
);

/*
 * Returns a copy of the component metadata for the
 * specified ID
 */
VecsComponentMetadata vecsComponentListGetMetadata(
    VecsComponentList *componentListPtr,
    VecsComponentSet componentID
);

/*
 * Returns the maximum number of components a component
 * list can hold
 */
#define vecsComponentListMaxComponents(COMPONENTSPTR) \
    ((COMPONENTSPTR)->_componentArray.size)

/*
 * Frees the memory associated with the specified
 * WindComponents
 */
void vecsComponentListFree(
    VecsComponentList *componentListPtr
);

#endif