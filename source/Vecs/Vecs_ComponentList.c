#include "Vecs_ComponentList.h"

/*
 * numComponents is the max number of bits in a
 * component set
 */
#define numComponents (8 * sizeof(VecsComponentSet))

/*
 * Constructs and returns a new empty component list
 * by value
 */
VecsComponentList vecsComponentsMake(){
    VecsComponentList toRet = {0};
    toRet._componentArray = arrayMake(
        VecsComponentMetadata,
        numComponents
    );
    toRet._validComponentTypes = NULL;
    return toRet;
}

/*
 * Inserts the specified component metadata into the
 * given VecsComponentList at the specified ID; error
 * if the componentID has already been used or is out
 * of bounds
 */
void vecsComponentListInsert(
    VecsComponentList *componentListPtr,
    VecsComponentMetadata componentMetadata,
    VecsComponentSet componentID 
){
    assertFalse(
        vecsComponentSetContainsAny(
            componentListPtr->_validComponentTypes,
            componentID
        ),
        "component ID already set; "
        SRC_LOCATION
    );
    /* add the metadata to the array */
    arraySet(VecsComponentMetadata,
        &(componentsPtr->_componentArray),
        componentID,
        componentMetadata
    );
    /* set present in the component set */
    componentListPtr->_validComponentTypes
        = vecsComponentSetUnion(
            componentListPtr->_validComponentTypes,
            componentID
        );
}

/*
 * Returns a copy of the component metadata for the
 * specified ID; errors if id is out of bounds or not
 * registered
 */
WindComponentMetadata windComponentsGet(
    WindComponents *componentsPtr,
    WindComponentIDType componentID
){
    assertFalse(
        componentID >= windComponentsNumComponents(
            componentsPtr
        ),
        "component ID out of bounds; "
        SRC_LOCATION
    );
    assertTrue(
        bitsetGet(
            &(componentsPtr->_setComponentIDs),
            componentID
        ),
        "component ID not present; "
        SRC_LOCATION
    );
    return arrayGet(WindComponentMetadata,
        &(componentsPtr->_componentArray),
        componentID
    );
}

/*
 * Frees the memory associated with the specified
 * WindComponents
 */
void windComponentsFree(WindComponents *componentsPtr){
    /* component metadata itself doesn't need free */
    arrayFree(WindComponentMetadata,
        &(componentsPtr->_componentArray)
    );
    bitsetFree(&(componentsPtr->_setComponentIDs));
}