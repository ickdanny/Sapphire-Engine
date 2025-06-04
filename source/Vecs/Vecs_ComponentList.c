#include "Vecs_ComponentList.h"

#include "Vecs_Entity.h"

/*
 * Constructs and returns a new empty component list
 * by value
 */
VecsComponentList vecsComponentsMake(){
    VecsComponentList toRet = {0};
    toRet._componentArray = arrayMake(
        VecsComponentMetadata,
        vecsMaxNumComponents
    );
    toRet._validComponentTypes = vecsEmptyComponentSet;

    /* insert metadata for component 0 - entity id */
    vecsComponentListInsert(
        &toRet,
        vecsComponentMetadataMake(VecsEntity, NULL),
        VecsEntityId
    );

    return toRet;
}

/*
 * Inserts the specified component metadata into the
 * given VecsComponentList at the specified id; error
 * if the component id has already been used or is out
 * of bounds
 */
void vecsComponentListInsert(
    VecsComponentList *componentListPtr,
    VecsComponentMetadata componentMetadata,
    VecsComponentId componentId
){
    assertFalse(
        componentId > vecsMaxComponentId,
        "component Id out of bounds; "
        SRC_LOCATION
    );
    assertFalse(
        vecsComponentSetContainsId(
            componentListPtr->_validComponentTypes,
            componentId
        ),
        "component Id already set; "
        SRC_LOCATION
    );
    /* add the metadata to the array */
    arraySet(VecsComponentMetadata,
        &(componentListPtr->_componentArray),
        componentId,
        componentMetadata
    );
    /* set present in the component set */
    componentListPtr->_validComponentTypes
        = vecsComponentSetAddId(
            componentListPtr->_validComponentTypes,
            componentId
        );
}

/*
 * Returns a copy of the component metadata for the
 * specified id; errors if id is out of bounds or not
 * registered
 */
VecsComponentMetadata vecsComponentListGetMetadata(
    VecsComponentList *componentListPtr,
    VecsComponentId componentId
){
    assertFalse(
        componentId > vecsMaxComponentId,
        "component id out of bounds; "
        SRC_LOCATION
    );
    assertTrue(
        vecsComponentSetContainsId(
            componentListPtr->_validComponentTypes,
            componentId
        ),
        "component id not present; "
        SRC_LOCATION
    );
    return arrayGet(VecsComponentMetadata,
        &(componentListPtr->_componentArray),
        componentId
    );
}

/*
 * Frees the memory associated with the specified
 * VecsComponentList
 */
void vecsComponentListFree(
    VecsComponentList *componentListPtr
){
    /* component metadata itself doesn't need free */
    arrayFree(VecsComponentMetadata,
        &(componentListPtr->_componentArray)
    );
    memset(
        componentListPtr,
        0,
        sizeof(*componentListPtr)
    );
}