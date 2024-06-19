#include "_WindECS_Archetype.h"

#define initDenseCapacity 20

/*
 * Constructs and returns a new _WindArchetype by
 * value; the archetype will make a copy of the
 * component set
 */
_WindArchetype _windArchetypeMake(
    const Bitset *componentSetPtr,
    WindEntityIDType numEntities,
    WindComponents *componentsPtr
){
    _WindArchetype toRet = {0};
    toRet._componentSet = bitsetCopy(componentSetPtr);
    toRet._componentsPtr = componentsPtr;
    WindComponentIDType numComponents
        = windComponentsNumComponents(componentsPtr);
    toRet._componentStorageArray = arrayMake(
        SparseSet,
        numComponents
    );

    /* initialize each sparse set using RTTI */
    for(WindComponentIDType i = 0;
        i < numComponents;
        ++i
    ){
        /* if the component is in this archetype */
        if(bitsetGet(&(toRet._componentSet), i)){
            /* retrieve RTTI for the component */
            WindComponentMetadata componentMetadata
                = windComponentsGet(componentsPtr, i);
            /*
             * initialize new sparse set for component
             */
            arraySet(SparseSet,
                &(toRet._componentStorageArray),
                i,
                _sparseSetMake(
                    numEntities,
                    initDenseCapacity,
                    componentMetadata._componentSize
                    #ifdef _DEBUG
                    , componentMetadata._typeName
                    #endif
                );
            );
        }
    }
    return toRet;
}

//todo: these can't be static since need to use in value set
/*
 * Errors if the componentID is invalid for the
 * specified archetype
 */
static void errorIfBadComponentID(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID
){
    assertTrue(
        bitsetGet(
            &(archetypePtr->_componentSet),
            componentID
        ),
        "error: component not present in archetype; "
        SRC_LOCATION
    );
}

/*
 * Errors if the entityID is invalid for the specified
 * sparse set of the given component type
 */
static void errorIfBadEntityID(
    SparseSet *componentStoragePtr,
    WindComponentMetadata componentMetadata,
    WindEntityIDType entityID
){
    assertTrue(
        _sparseSetContains(
            componentStoragePtr,
            entityID
            #ifdef _DEBUG
            , componentMetadata._typeName
            #endif
        ),
        "error: entityID invalid for archetype; "
        SRC_LOCATION
    );
}

/*
 * Returns a pointer to the component specified by
 * the given componentID of the entity specified by
 * the given entityID; error if the componentID or the
 * entityID is invalid
 */
void *__windArchetypeGetPtr(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
){
    errorIfBadComponentID(
        archetypePtr,
        componentID
    );
    /* get a ptr to the storage for the component */
    SparseSet *componentStoragePtr = arrayGetPtr(
        SparseSet,
        &(archetypePtr->_componentStorageArray),
        componentID
    );
    /* get a copy of the component metadata */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            &(archetypePtr->_componentsPtr),
            componentID
        );
    errorIfBadEntityID(
        componentStoragePtr,
        componentMetadata,
        entityID
    );

    /* return ptr into the sparse set */
    return _sparseSetGetPtr(
        componentStoragePtr,
        entityID,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}

/*
 * Sets the component specified by the given
 * componentID of the entity specified by the given
 * entityID to the value stored in the given void ptr;
 * error if the componentID or the entityID is invalid
 */
void __windArchetypeSetPtr(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID,
    void *componentPtr
){
    errorIfBadComponentID(
        archetypePtr,
        componentID
    );
    /* get a ptr to the storage for the component */
    SparseSet *componentStoragePtr = arrayGetPtr(
        SparseSet,
        &(archetypePtr->_componentStorageArray),
        componentID
    );
    /* get a copy of the component metadata */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            &(archetypePtr->_componentsPtr),
            componentID
        );
    errorIfBadEntityID(
        componentStoragePtr,
        componentMetadata,
        entityID
    );

    //todo: run destructor if present

    /* copy data into sparse set */
    _sparseSetSetPtr(
        componentStoragePtr,
        entityID,
        componentPtr,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}