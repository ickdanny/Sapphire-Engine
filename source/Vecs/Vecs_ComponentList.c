#include "Vecs_ComponentList.h"

//todo: implement this

/*
 * Constructs and returns a new empty component list
 * by value
 */
VecsComponentList windComponentsMake(
    WindComponentIDType numComponents
){
    WindComponents toRet = {0};
    toRet._componentArray = arrayMake(
        WindComponentMetadata,
        numComponents
    );
    toRet._setComponentIDs = bitsetMake(numComponents);
    return toRet;
}

/*
 * Inserts the specified WindComponentMetadata into
 * the given WindComponents at the specified ID; error
 * if the componentID has already been used or is out
 * of bounds
 */
void windComponentsInsert(
    WindComponents *componentsPtr,
    WindComponentMetadata componentMetadata,
    WindComponentIDType componentID 
){
    assertFalse(
        componentID >= windComponentsNumComponents(
            componentsPtr
        ),
        "component ID out of bounds; "
        SRC_LOCATION
    );
    assertFalse(
        bitsetGet(
            &(componentsPtr->_setComponentIDs),
            componentID
        ),
        "component ID already set; "
        SRC_LOCATION
    );
    /* add the metadata to the array */
    arraySet(WindComponentMetadata,
        &(componentsPtr->_componentArray),
        componentID,
        componentMetadata
    );
    /* set present in the bitset */
    bitsetSet(
        &(componentsPtr->_setComponentIDs),
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