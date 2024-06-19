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
             * unless component is a marker
             */
            if(componentMetadata._componentSize == 0){
                continue;
            }
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

/*
 * Errors if the componentID is invalid for the
 * specified archetype
 */
void __windArchetypeErrorIfBadComponentID(
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
void __windArchetypeErrorIfBadEntityID(
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
 * entityID is invalid; returns NULL if the component
 * is a marker
 */
void *__windArchetypeGetPtr(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID
){
    __windArchetypeErrorIfBadComponentID(
        archetypePtr,
        componentID
    );
    /* get a copy of the component metadata */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            archetypePtr->_componentsPtr,
            componentID
        );
    /* return NULL for markers */
    if(componentMetadata._componentSize == 0){
        return NULL;
    }
    /* get a ptr to the storage for the component */
    SparseSet *componentStoragePtr = arrayGetPtr(
        SparseSet,
        &(archetypePtr->_componentStorageArray),
        componentID
    );
    __windArchetypeErrorIfBadEntityID(
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
 * error if the componentID or the entityID is invalid;
 * does nothing if NULL is passed
 */
void __windArchetypeSetPtr(
    _WindArchetype *archetypePtr,
    WindComponentIDType componentID,
    WindEntityIDType entityID,
    void *componentPtr
){
    __windArchetypeErrorIfBadComponentID(
        archetypePtr,
        componentID
    );
    /* do nothing if NULL passed */
    if(!componentPtr){
        return;
    }
    /* get a ptr to the storage for the component */
    SparseSet *componentStoragePtr = arrayGetPtr(
        SparseSet,
        &(archetypePtr->_componentStorageArray),
        componentID
    );
    /* get a copy of the component metadata */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            archetypePtr->_componentsPtr,
            componentID
        );
    __windArchetypeErrorIfBadEntityID(
        componentStoragePtr,
        componentMetadata,
        entityID
    );

    /*
     * run component destructor if sparse set already
     * contains a mapping and if destructor exists
     */
    if(componentMetadata._destructor){
        if(_sparseSetContains(
            componentStoragePtr,
            entityID
            #ifdef _DEBUG
            , componentMetadata._typeName
            #endif
        )){
            void *oldComponentPtr = _sparseSetGetPtr(
                componentStoragePtr,
                entityID,
                componentMetadata._componentSize
                #ifdef _DEBUG
                , componentMetadata._typeName
                #endif
            );
            componentMetadata._destructor(
                oldComponentPtr
            );
        }
    }

    /*
     * copy data into sparse set, possibly 
     * overwritting the previously destroyed data
     */
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

/*
 * Moves the entity identified by the given entityID
 * to the specified other archetype; error if the
 * same archetype is provided or if the entityID is
 * invalid
 */
void _windArchetypeMoveEntity(
    _WindArchetype *archetypePtr,
    WindEntityIDType entityID,
    _WindArchetype *newArchetypePtr
){
    assertFalse(
        archetypePtr == newArchetypePtr,
        "same archetype passed for move; "
        SRC_LOCATION
    );
    WindComponentIDType numComponents
        = windComponentsNumComponents(
            archetypePtr->_componentsPtr
        );
    for(WindComponentIDType i = 0;
        i < numComponents;
        ++i
    ){
        /* if the component is in this archetype */
        if(bitsetGet(
            &(archetypePtr->_componentSet),
            i
        )){
            /* retrieve RTTI for the component */
            WindComponentMetadata componentMetadata
                = windComponentsGet(
                    archetypePtr->_componentsPtr,
                    i
                );
            /* do nothing if component is a marker */
            if(componentMetadata._componentSize == 0){
                continue;
            }
            /* get old component storage */
            SparseSet *componentStoragePtr
                = arrayGetPtr(
                    SparseSet,
                    &(archetypePtr
                        ->_componentStorageArray),
                    i
                );
            __windArchetypeErrorIfBadEntityID(
                componentStoragePtr,
                componentMetadata,
                entityID
            );

            /* get ptr to component */
            void *componentPtr = _sparseSetGetPtr(
                componentStoragePtr,
                entityID,
                componentMetadata._componentSize
                #ifdef _DEBUG
                , componentMetadata._typeName
                #endif
            );
            /*
             * if component is in new archetype, 
             * shallow copy it
             */
            if(bitsetGet(
                &(newArchetypePtr->_componentSet),
                i
            )){
                /* get new component storage */
                SparseSet *newComponentStoragePtr
                    = arrayGetPtr(
                        SparseSet,
                        &(newArchetypePtr
                            ->_componentStorageArray),
                        i
                    );
                /*
                 * sparse set shallow copy into new
                 * archetype
                 */
                _sparseSetSetPtr(
                    newComponentStoragePtr,
                    entityID,
                    componentPtr,
                    componentMetadata._componentSize
                    #ifdef _DEBUG
                    , componentMetadata._typeName
                    #endif
                );
            } /* END component in new archetype */
            /*
             * if component not in new archetype,
             * call destructor on it
             */
            else{
                componentMetadata._destructor(
                    componentPtr
                );
            } /* END component not in new archetype */
            /* remove component from old storage */
            _sparseSetRemove(
                componentStoragePtr,
                entityID,
                componentMetadata._componentSize
                #ifdef _DEBUG
                , componentMetadata._typeName
                #endif
            );
        } /* END component in old archetype */
    } /* END for over all components */
}

/*
 * Removes the entity identified by the given entityID;
 * returns true if entity successfully removed, false
 * if entity was not originally in the archetype
 */
bool _windArchetypeRemoveEntity(
    _WindArchetype *archetypePtr,
    WindEntityIDType entityID
){
    bool toRet = false;
    WindComponentIDType numComponents
        = windComponentsNumComponents(
            archetypePtr->_componentsPtr
        );
    for(WindComponentIDType i = 0;
        i < numComponents;
        ++i
    ){
        /* if the component is in this archetype */
        if(bitsetGet(
            &(archetypePtr->_componentSet),
            i
        )){
            /* retrieve RTTI for the component */
            WindComponentMetadata componentMetadata
                = windComponentsGet(
                    archetypePtr->_componentsPtr,
                    i
                );
            /* do nothing if component is a marker */
            if(componentMetadata._componentSize == 0){
                continue;
            }
            /* get component storage */
            SparseSet *componentStoragePtr
                = arrayGetPtr(
                    SparseSet,
                    &(archetypePtr
                        ->_componentStorageArray),
                    i
                );
            /*
             * sparse set remove will return true if
             * it successfully removes an element
             */
            if(_sparseSetRemove(
                componentStoragePtr,
                entityID,
                componentMetadata._componentSize
                #ifdef _DEBUG
                , componentMetadata._typeName
                #endif
            )){
                toRet = true;
            }
        } /* END component in old archetype */
    } /* END for over all components */
}

//todo: function to run destructor on every component
//in given storage, then free storage

/*
 * Frees the memory associated with the given
 * Archetype
 */
void _windArchetypeFree(_WindArchetype *archetypePtr){
    bitsetFree(&(archetypePtr->_componentSet));
    
    //todo: free component storage array

    /*
     * do not free componentsPtr since it is not
     * owned by the archetype
     */
    archetypePtr->_componentsPtr = NULL;
}