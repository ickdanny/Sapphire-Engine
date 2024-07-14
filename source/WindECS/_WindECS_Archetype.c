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
 * Frees all entity data in the specified component
 * storage using the provided RTTI but does not free
 * the storage itself
 */
static void __componentStorageClear(
    SparseSet *componentStoragePtr,
    WindComponentMetadata componentMetadata
){
    /* run destructor on all elements in storage */
    if(componentMetadata._destructor){
        SparseSetItr itr = _sparseSetItr(
            componentStoragePtr
            #ifdef _DEBUG
            , componentMetadata._typeName
            #endif
        );
        while(_sparseSetItrHasNext(
            &itr
            #ifdef _DEBUG
            , componentMetadata._typeName
            #endif
        )){
            componentMetadata._destructor(
                _sparseSetItrNextPtr(
                    &itr,
                    componentMetadata._componentSize
                    #ifdef _DEBUG
                    , componentMetadata._typeName
                    #endif
                )
            );
        }
    }
}

/*
 * Frees all the memory associated with the specified
 * component storage using the provided RTTI
 */
static void __componentStorageFree(
    SparseSet *componentStoragePtr,
    WindComponentMetadata componentMetadata
){
    /* run destructor on all elements in storage */
    __componentStorageClear(
        componentStoragePtr,
        componentMetadata
    );

    /* free the storage */
    _sparseSetFree(
        componentStoragePtr
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}

/* Clears all entities from the specified archetype */
void _windArchetypeClear(_WindArchetype *archetypePtr){
    WindComponentIDType numComponents
        = windComponentsNumComponents(
            archetypePtr->_componentsPtr
        );

    //todo: temp
    printf("clearing archetype %p\n", archetypePtr);
    static char buffer[100] = {0};
    printBitset(&archetypePtr->_componentSet, buffer, 100);
    printf("set: %s\n", buffer);

    /* free each sparse set using RTTI */
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
            /* skip markers */
            if(componentMetadata._componentSize == 0){
                continue;
            }
            /*
             * clear the component storage for that
             * component ID
             */
            __componentStorageClear(
                arrayGetPtr(SparseSet,
                    &(archetypePtr
                        ->_componentStorageArray),
                    i
                ),
                componentMetadata
            );
        }
    }

    /* increase modification count for iterators */
    ++(archetypePtr->_modificationCount);
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
 * error if the componentID is invalid; does nothing
 * if NULL is passed or if the component is a marker
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
    /* get a copy of the component metadata */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            archetypePtr->_componentsPtr,
            componentID
        );
    /* do nothing if component is a marker */
    if(componentMetadata._componentSize == 0){
        return;
    }
    /* get a ptr to the storage for the component */
    SparseSet *componentStoragePtr = arrayGetPtr(
        SparseSet,
        &(archetypePtr->_componentStorageArray),
        componentID
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

    /* increment modification count */
    ++(archetypePtr->_modificationCount);
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

    /*
     * increase modification count for both
     * archetypes
     */
    ++(archetypePtr->_modificationCount);
    ++(newArchetypePtr->_modificationCount);
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

    if(toRet){
        /* increase modification count */
        ++(archetypePtr->_modificationCount);
    }
    return toRet;
}

/*
 * Frees all component storages owned by the given
 * Archetype
 */
void __windArchetypeFreeComponentStorages(
    _WindArchetype *archetypePtr
){
    WindComponentIDType numComponents
        = windComponentsNumComponents(
            archetypePtr->_componentsPtr
        );

    /* free each sparse set using RTTI */
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
            /* skip markers */
            if(componentMetadata._componentSize == 0){
                continue;
            }
            /*
             * free the component storage for that
             * component ID
             */
            __componentStorageFree(
                arrayGetPtr(SparseSet,
                    &(archetypePtr
                        ->_componentStorageArray),
                    i
                ),
                componentMetadata
            );
        }
    }
}

/*
 * Frees the memory associated with the given
 * Archetype
 */
void _windArchetypeFree(_WindArchetype *archetypePtr){
    /* free all entities (i.e. components) */
    __windArchetypeFreeComponentStorages(archetypePtr);

    /* free the component bitset */
    bitsetFree(&(archetypePtr->_componentSet));

    /* free the whole indexing array */
    arrayFree(SparseSet,
        &(archetypePtr->_componentStorageArray)
    );

    /*
     * do not free componentsPtr since it is not
     * owned by the archetype
     */
    archetypePtr->_componentsPtr = NULL;

    /*
     * increase modification count in case iterators
     * still out there
     */
    ++(archetypePtr->_modificationCount);
}

/* Returns an iterator over the specified archetype */
_WindArchetypeItr _windArchetypeItr(
    _WindArchetype *archetypePtr
){
    _WindArchetypeItr toRet = {
        archetypePtr,
        0,
        archetypePtr->_modificationCount,
        SIZE_MAX /* default to no present component */
    };

    /* find a present component if possible */
    WindComponentIDType numComponents
        = windComponentsNumComponents(
            archetypePtr->_componentsPtr
        );
    for(size_t i = 0; i < numComponents; ++i){
        if(bitsetGet(
            &(archetypePtr->_componentSet),
            i
        )){
            WindComponentMetadata metadata
                = windComponentsGet(
                    archetypePtr->_componentsPtr,
                    i
                );
            if(metadata._componentSize != 0){
                toRet._presentComponentIndex = i;
                break;
            }
        }
    }
    return toRet;
}

/*
 * Throws error if concurrent modification detected
 * for the specified archetype iterator
 */
static void errorIfConcurrentModification(
    _WindArchetypeItr *itrPtr
){
    assertTrue(
        itrPtr->_storedModificationCount
            == itrPtr->_archetypePtr
                ->_modificationCount,
        "error: archetype concurrent modification; "
        SRC_LOCATION
    );
}

/*
 * Returns true if the specified archetype iterator
 * has more elements, false otherwise
 */
bool _windArchetypeItrHasEntity(
    _WindArchetypeItr *itrPtr
){
    errorIfConcurrentModification(itrPtr);
    /* 
     * if the archetype has no non-marker components,
     * return false as iteration is not possible
     * in this case
     */
    if(itrPtr->_presentComponentIndex == SIZE_MAX){
        return false;
    }
    _WindArchetype *archetypePtr
        = itrPtr->_archetypePtr;
    SparseSet *presentComponentStoragePtr
        = arrayGetPtr(
            SparseSet,
            &(archetypePtr->_componentStorageArray),
            itrPtr->_presentComponentIndex
        );
    /*
     * return true if the index is in bounds for
     * the dense array 
     */
    return itrPtr->_currentIndex
        < presentComponentStoragePtr->_size;
}

/*
 * Advances the archetype itr to point to the next
 * entity
 */
void _windArchetypeItrAdvance(
    _WindArchetypeItr *itrPtr
){
    errorIfConcurrentModification(itrPtr);
    ++(itrPtr->_currentIndex);
}

/*
 * Returns a pointer to the component specified by
 * the given componentID of the entity currently being
 * pointed to by the given archetype iterator; error
 * if the componentID is invalid or if the iterator
 * is out of entities; returns NULL if the component
 * is a marker
 */
void *__windArchetypeItrGetPtr(
    _WindArchetypeItr *itrPtr,
    WindComponentIDType componentID
){
    errorIfConcurrentModification(itrPtr);
    /* error if itr is out of entities */
    assertTrue(
        _windArchetypeItrHasEntity(itrPtr),
        "error: archetype itr out of entities; "
        SRC_LOCATION
    );
    /*
     * error if component ID is not valid for the
     * archetype or is a marker
     */
    _WindArchetype *archetypePtr
        = itrPtr->_archetypePtr;
    assertTrue(
        bitsetGet(
            &(archetypePtr->_componentSet),
            componentID
        ),
        "error: archetype does not have component; "
        SRC_LOCATION
    );
    /* return null if the request is for a marker */
    WindComponentMetadata componentMetadata
        = windComponentsGet(
            archetypePtr->_componentsPtr,
            componentID
        );
    if(componentMetadata._componentSize == 0){
        return NULL;
    }
    /*
     * otherwise reach into the sparse set and 
     * return a pointer to the next dense element
     */
    SparseSet *componentStoragePtr = arrayGetPtr(
        SparseSet,
        &(archetypePtr->_componentStorageArray),
        componentID
    );
    return voidPtrAdd(
        componentStoragePtr->_densePtr,
        itrPtr->_currentIndex
            * componentMetadata._componentSize
    );
}

/*
 * Returns the ID of the etity curently being pointed
 * to by the given archetype iterator; error if the
 * archetype is out of entities
 */
WindEntityIDType _windArchetypeItrCurrentID(
    _WindArchetypeItr *itrPtr
){
    assertNotNull(itrPtr, "null passed " SRC_LOCATION);
    assertNotNull(
        itrPtr->_archetypePtr,
        "archetype itr has null archetype ptr; "
        SRC_LOCATION
    );
    errorIfConcurrentModification(itrPtr);
    /* 
     * if the archetype has no non-marker components,
     * error
     */
    assertFalse(
        itrPtr->_presentComponentIndex == SIZE_MAX,
        "error: cannot get entity ID from archetype "
        "itr if the archetype is all markers; "
        SRC_LOCATION
    );
    _WindArchetype *archetypePtr
        = itrPtr->_archetypePtr;
    SparseSet *presentComponentStoragePtr
        = arrayGetPtr(
            SparseSet,
            &(archetypePtr->_componentStorageArray),
            itrPtr->_presentComponentIndex
        );
    /* if the entity is valid into the dense array */
    if(itrPtr->_currentIndex
        < presentComponentStoragePtr->_size
    ){
        /* 
         * get the sparse index AKA the entity ID from
         * the reflect array
         */
        return presentComponentStoragePtr->_reflectPtr[
            itrPtr->_currentIndex
        ];
    }
    /* otherwise error */
    else{
        pgError(
            "error: no more entities in archetype "
            "itr to get the ID of; " SRC_LOCATION
        );
        return 0;
    }
}