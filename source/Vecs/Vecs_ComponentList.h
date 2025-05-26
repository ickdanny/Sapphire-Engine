#ifndef VECS_COMPONENTLIST_H
#define VECS_COMPONENTLIST_H

#include "Constructure.h"

#include "Vecs_ComponentMetadata.h"

/* Stores RTTI for all components of an ECS world */
typedef struct VecsComponentList{
    Array _componentArray;
    /*
     * used to indicate whether a component of a
     * specific id has already been specified
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
 * the given component list at the specified id; error
 * if the component id has already been used
 */
void vecsComponentListInsert(
    VecsComponentList *componentListPtr,
    VecsComponentMetadata componentMetadata,
    VecsComponentId componentId
);

/*
 * Returns a copy of the component metadata for the
 * specified id
 */
VecsComponentMetadata vecsComponentListGetMetadata(
    VecsComponentList *componentListPtr,
    VecsComponentId componentId
);

/*
 * Frees the memory associated with the specified
 * VecsComponentList
 */
void vecsComponentListFree(
    VecsComponentList *componentListPtr
);

#endif