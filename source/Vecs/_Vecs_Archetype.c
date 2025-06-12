#include "_Vecs_Archetype.h"

/*
 * Constructs and returns a new archetype by value
 */
_VecsArchetype _vecsArchetypeMake(
    VecsComponentSet componentSet,
    size_t initEntityCapacity,
    VecsComponentList *componentListPtr,
    _VecsEntityList *entityListPtr
){
    /*
     * all archetypes must have component 0 (entity id)
     * as vecs relies on this internally
     */
    assertTrue(
        vecsComponentSetContainsId(
            componentSet,
            VecsEntityId
        ),
        "error: all archetypes must contain component "
        "0 - VecsEntity; \n"
        SRC_LOCATION
    );

    _VecsArchetype toRet = {
        ._componentSet = componentSet,
        ._componentListPtr = componentListPtr,
        ._entityListPtr = entityListPtr
    };

    /* initialize each component array */
    for(VecsComponentId i = 0;
        i < vecsMaxNumComponents;
        ++i
    ){
        /* skip if component not within archetype */
        if(!vecsComponentSetContainsId(
            componentSet,
            i
        )){
            continue;
        }

        /* retrieve RTTI for the component */
        VecsComponentMetadata componentMetadata
            = vecsComponentListGetMetadata(
                componentListPtr,
                i
            );
        
        /* skip if component is marker */
        if(componentMetadata._componentSize == 0){
            continue;
        }

        /* allocate a new arraylist for component */
        #ifdef _DEBUG
        toRet._componentStorageLists[i]
            = _arrayListMake(
                initEntityCapacity,
                componentMetadata._componentSize,
                componentMetadata._typeName
            );
        #else
        toRet._componentStorageLists[i]
            = _arrayListMake(
                initEntityCapacity,
                componentMetadata._componentSize
            );
        #endif
    }
}

/*
 * Frees all entity data in the specified component
 * storage using the provided RTTI but does not free
 * the storage itself
 */
static void componentStorageListClear(
    ArrayList *componentStorageListPtr,
    VecsComponentMetadata componentMetadata
){
    /* run destructor on all elements in storage */
    if(componentMetadata._destructor){
        for(size_t i = 0;
            i < componentStorageListPtr->size;
            ++i
        ){
            componentMetadata._destructor(
                _arrayListGetPtr(
                    componentStorageListPtr,
                    i,
                    componentMetadata._componentSize
                    #ifdef _DEBUG
                    , componentMetadata._typeName
                    #endif
                )
            );
        }
    }

    _arrayListClear(
        componentStorageListPtr,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}

/*
 * Reclaims all entities present in the archetype;
 * should be used when clearing all data
 */
static void __vecsArchetypeReclaimAllEntities(
    _VecsArchetype *archetypePtr
){
    /*
     * assume all archetypes have component 0 for
     * entities - use it to kill the entities in the
     * entity list
     */
    ArrayList *entityListPtr
        = &(archetypePtr
            ->_componentStorageLists[VecsEntityId]);
    for(size_t i = 0; i < entityListPtr->size; ++i){
        _vecsEntityListReclaim(
            archetypePtr->_entityListPtr,
            arrayListGet(VecsEntity,
                entityListPtr,
                i
            )
        );
    }
}

/*
 * Clears all component data from the specified
 * archetype
 */
void _vecsArchetypeClear(_VecsArchetype *archetypePtr){
    __vecsArchetypeReclaimAllEntities(archetypePtr);

    /* clear component data */
    for(VecsComponentId i = 0;
        i < vecsMaxNumComponents;
        ++i
    ){
        /* skip if component not within archetype */
        if(!vecsComponentSetContainsId(
            archetypePtr->_componentSet,
            i
        )){
            continue;
        }

        /* retrieve RTTI for the component */
        VecsComponentMetadata componentMetadata
            = vecsComponentListGetMetadata(
                archetypePtr->_componentListPtr,
                i
            );
        
        /* skip if component is marker */
        if(componentMetadata._componentSize == 0){
            continue;
        }

        componentStorageListClear(
            &(archetypePtr->_componentStorageLists[i]),
            componentMetadata
        );
    }
}

/*
 * Errors if the specified component id is invalid for
 * the given archetype
 */
void _vecsArchetypeErrorIfBadComponent(
    _VecsArchetype *archetypePtr,
    VecsComponentId componentId
){
    assertTrue(
        vecsComponentSetContainsId(
            archetypePtr->_componentSet,
            componentId
        ),
        "error: component not present in archetype; "
        SRC_LOCATION
    );
}

/*
 * Errors if the specified entity id is not within the
 * given archetype
 */
void _vecsArchetypeErrorIfBadEntity(
    _VecsArchetype *archetypePtr,
    VecsEntity entity
){
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            archetypePtr->_entityListPtr,
            entity
        );
    assertNotNull(
        entityMetadataPtr,
        "failed to retrieve entity metadata ptr; "
        SRC_LOCATION
    );
    assertTrue(
        archetypePtr->_componentSet 
            == entityMetadataPtr->_archetypePtr
                ->_componentSet,
        "entity points to a different archetype; "
        SRC_LOCATION
    );
}

/*
 * Returns a pointer to the component specified by
 * the given component id of the specified entity;
 * error if the component or the entity is invalid;
 * returns NULL if the component is a marker
 */
void *__vecsArchetypeGetPtr(
    _VecsArchetype *archetypePtr,
    VecsComponentId componentId,
    VecsEntity entity
){
    _vecsArchetypeErrorIfBadComponent(
        archetypePtr,
        componentId
    );
    _vecsArchetypeErrorIfBadEntity(
        archetypePtr,
        entity
    );

    VecsComponentMetadata componentMetadata
        = vecsComponentListGetMetadata(
            archetypePtr->_componentListPtr,
            componentId
        );
    
    /* return NULL if component is a marker */
    if(componentMetadata._componentSize == 0){
        return NULL;
    }

    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            archetypePtr->_entityListPtr,
            entity
        );
    size_t index
        = entityMetadataPtr->_indexInArchetype;

    ArrayList *componentStorageListPtr
        = &(archetypePtr
            ->_componentStorageLists[componentId]);

    return _arrayListGetPtr(
        componentStorageListPtr,
        index,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}

/*
 * Sets the component specified by the given
 * component id of the entity specified by the given
 * entity id to the value stored in the given void ptr;
 * error if the component id is invalid; does nothing
 * if NULL is passed or if the component is a marker;
 * assumes entity index has already been assigned
 */
void __vecsArchetypeSetPtr(
    _VecsArchetype *archetypePtr,
    VecsComponentId componentId,
    VecsEntity entity,
    void *componentPtr
){
    _vecsArchetypeErrorIfBadComponent(
        archetypePtr,
        componentId
    );
    _vecsArchetypeErrorIfBadEntity(
        archetypePtr,
        entity
    );

    /* do nothing if NULL passed */
    if(!componentPtr){
        return;
    }

    VecsComponentMetadata componentMetadata
        = vecsComponentListGetMetadata(
            archetypePtr->_componentListPtr,
            componentId
        );
    
    /* do nothing if component is a marker */
    if(componentMetadata._componentSize == 0){
        return;
    }

    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            archetypePtr->_entityListPtr,
            entity
        );
    size_t index
        = entityMetadataPtr->_indexInArchetype;

    ArrayList *componentStorageListPtr
        = &(archetypePtr
            ->_componentStorageLists[componentId]);

    void *componentSlotPtr = _arrayListGetPtr(
        componentStorageListPtr,
        index,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );

    /*
     * case 1: this operation replaces a preexisting
     * component
     */
    if(vecsComponentSetContainsId(
        entityMetadataPtr->_initializedComponentSet,
        componentId
    )){
        /* run component destructor if needed */
        if(componentMetadata._destructor){
            componentMetadata._destructor(
                componentSlotPtr
            );
        }
    }
    /*
     * case 2: this operation adds a new component
     */
    else{
        /*
         * mark the component as initialized for the
         * entity
         */
        entityMetadataPtr->_initializedComponentSet
            = vecsComponentSetAddId(
                entityMetadataPtr
                    ->_initializedComponentSet,
                componentId
            );
    }

    _arrayListSetPtr(
        componentStorageListPtr,
        index,
        componentPtr,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );

    ++(archetypePtr->_modificationCount);
}

/*
 * Removes the component at the specified index of the
 * given component list by copying in the last element;
 * does not run the destructor
 */
static void componentStorageListFastRemove(
    ArrayList *componentStorageListPtr,
    VecsComponentMetadata componentMetadata,
    size_t index
){
    assertTrue(
        index < componentStorageListPtr->size,
        "Error: index out of bounds for component "
        "list remove; "
        SRC_LOCATION
    );
    assertFalse(
        arrayListIsEmpty(componentStorageListPtr),
        "Error: component list is empty; "
        SRC_LOCATION
    );

    /*
     * special case: removing the final element does
     * not require a copy
     */
    if(index == componentStorageListPtr->size - 1){
        _arrayListPopBack(
            componentStorageListPtr
            #ifdef _DEBUG
            , componentMetadata._typeName
            #endif
        );
        return;
    }

    /* normal case: copy final element into index */
    _arrayListSetPtr(
        componentStorageListPtr,
        index,
        _arrayListBackPtr(
            componentStorageListPtr,
            componentMetadata._componentSize
            #ifdef _DEBUG
            , componentMetadata._typeName
            #endif
        ),
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
    _arrayListPopBack(
        componentStorageListPtr
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}

/*
 * Moves the entity identified by the given entity id
 * to the specified other archetype; error if the
 * same archetype is provided or if the entity is
 * invalid
 */
void _vecsArchetypeMoveEntity(
    _VecsArchetype *srcArchetypePtr,
    _VecsArchetype *destArchetypePtr,
    VecsEntity entity
){
    assertFalse(
        srcArchetypePtr == destArchetypePtr,
        "same archetype passed for move; "
        SRC_LOCATION
    );

    _vecsArchetypeErrorIfBadEntity(
        srcArchetypePtr,
        entity
    );

    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            srcArchetypePtr->_entityListPtr,
            entity
        );
    size_t srcIndex
        = entityMetadataPtr->_indexInArchetype;

    /*
     * the entity will be pushed to the back of the
     * destination archetype, whose index can be
     * retrieved from the always-present entity
     * component storage
     */
    size_t destIndex
        = destArchetypePtr
            ->_componentStorageLists[VecsEntityId]
                .size;

    /* iterate over all possible components */
    for(VecsComponentId i = 0;
        i < vecsMaxNumComponents;
        ++i
    ){
        /*
         * skip if component not within archetype, or
         * equivalently, if entity lacks component
         */
        if(!vecsComponentSetContainsId(
            srcArchetypePtr->_componentSet,
            i
        )){
            continue;
        }

        VecsComponentMetadata componentMetadata
            = vecsComponentListGetMetadata(
                srcArchetypePtr->_componentListPtr,
                i
            );
    
        /* skip if component is a marker */
        if(componentMetadata._componentSize == 0){
            return;
        }

        ArrayList *srcComponentStorageListPtr
            = &(srcArchetypePtr
                ->_componentStorageLists[i]);

        void *srcComponentPtr = _arrayListGetPtr(
            srcComponentStorageListPtr,
            index,
            componentMetadata._componentSize
            #ifdef _DEBUG
            , componentMetadata._typeName
            #endif
        );

        /*
         * if component is in new archetype,
         * shallow copy it
         */
        if(vecsComponentSetContainsId(
            destArchetypePtr->_componentSet,
            i
        )){
            ArrayList *destComponentStorageListPtr
                = &(destArchetypePtr
                    ->_componentStorageLists[i]);
            _arrayListPushBackPtr(
                destComponentStorageListPtr,
                srcComponentPtr,
                componentMetadata._componentSize
            );
        }

        /*
         * if component not in new archetype, call
         * destructor on it if needed
         */
        else{
            if(componentMetadata._destructor){
                componentMetadata._destructor(
                    srcComponentPtr
                );
            }
            entityMetadataPtr->_initializedComponentSet
                = vecsComponentSetRemoveId(
                    entityMetadataPtr
                        ->_initializedComponentSet,
                    i
                );
        }

        /*
         * remove component from old storage
         * efficiently by copying over the last element
         */
        componentStorageListFastRemove(
            srcComponentStorageListPtr,
            componentMetadata,
            srcIndex
        );
    }

    /*
     * update entity metadata to reflect new archetype
     */
    entityMetadataPtr->_componentSet
        = destArchetypePtr->_componentSet;
    /* initialized component set updated in the loop */
    entityMetadataPtr->_archetypePtr
        = destArchetypePtr;
    entityMetadataPtr->_indexInArchetype = destIndex;

    ++(srcArchetypePtr->_modificationCount);
    ++(destArchetypePtr->_modificationCount);
}

/*
 * Removes the entity identified by the given entity
 * id; returns true if entity successfully removed,
 * false if entity was not originally in the archetype
 */
bool _vecsArchetypeRemoveEntity(
    _VecsArchetype *archetypePtr,
    VecsEntity entity
){
    _VecsEntityMetadata *entityMetadataPtr
        = _vecsEntityListGetMetadata(
            archetypePtr->_entityListPtr,
            entity
        );
    if(!entityMetadataPtr){
        return false;
    }
    if(archetypePtr->_componentSet 
        != entityMetadataPtr->_archetypePtr
            ->_componentSet
    ){
        return false;
    }

    size_t index
        = entityMetadataPtr->_indexInArchetype;

    /* get info about last entity */
    ArrayList *entityStorageListPtr
        = &(archetypePtr
            ->_componentStorageLists[VecsEntityId]);
    
    VecsComponentMetadata entityComponentMetadata
        = vecsComponentListGetMetadata(
            archetypePtr->_componentListPtr,
            VecsEntityId
        );

    size_t lastIndex = entityStorageListPtr->size - 1;
    VecsEntity *lastEntityPtr = _arrayListGetPtr(
        entityStorageListPtr,
        lastIndex,
        entityComponentMetadata._componentSize
        #ifdef _DEBUG
        , entityComponentMetadata->_typeName
        #endif
    );
    VecsEntity lastEntity = *lastEntityPtr;

    /*
     * if reached here, entity exists in archetype;
     * remove all its components by iterating
     */
    for(VecsComponentId i = 0;
        i < vecsMaxNumComponents;
        ++i
    ){
        /*
         * skip if component not within archetype, or
         * equivalently, if entity lacks component
         */
        if(!vecsComponentSetContainsId(
            archetypePtr->_componentSet,
            i
        )){
            continue;
        }
        VecsComponentMetadata componentMetadata
            = vecsComponentListGetMetadata(
                archetypePtr->_componentListPtr,
                i
            );
    
        /* skip if component is a marker */
        if(componentMetadata._componentSize == 0){
            return;
        }

        ArrayList *componentStorageListPtr
            = &(archetypePtr
                ->_componentStorageLists[i]);

        /* run destructor if needed */
        if(componentMetadata._destructor){
            void *componentPtr = _arrayListGetPtr(
                componentStorageListPtr,
                index,
                componentMetadata._componentSize
                #ifdef _DEBUG
                , componentMetadata._typeName
                #endif
            );
            assertNotNull(
                componentPtr,
                "Error: expect successful component "
                "retrieval; "
                SRC_LOCATION
            );
            componentMetadata._destructor(
                componentPtr
            );
        }

        /*
         * remove component from old storage
         * efficiently by copying over the last element
         */
        componentStorageListFastRemove(
            componentStorageListPtr,
            componentMetadata,
            index
        );
    }

    /* update entity metadata for removed entity */
    _vecsEntityMetadataIncrementGeneration(
        entityMetadataPtr
    );

    /*
     * update entity metadata for entity which was
     * moved from the tail of the list, if needed
     */
    if(index != lastIndex){
        _VecsEntityMetadata *lastEntityMetadataPtr
            = _vecsEntityListGetMetadata(
                archetypePtr->_entityListPtr,
                lastEntity
            );
        lastEntityMetadataPtr->_indexInArchetype
            = index;
    }

    ++(archetypePtr->_modificationCount);
    return true;
}

/*
 * Frees all the memory associated with the specified
 * component storage using the provided RTTI
 */
static void componentStorageListFree(
    ArrayList *componentStorageListPtr,
    VecsComponentMetadata componentMetadata
){
    /*
     * run destructor on all elements in storagel this
     * call also updates entity metadata
     */
    componentStorageListClear(
        componentStorageListPtr,
        componentMetadata
    );

    /* free the storage */
    _arrayListFree(
        componentStorageListPtr
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}

/*
 * Frees all the component storage lists owned by the
 * given archetype
 */
static void __vecsArchetypeFreeComponentStorageLists(
    _VecsArchetype *archetypePtr
){
    for(VecsComponentId i = 0;
        i < vecsMaxNumComponents;
        ++i
    ){
        /*
         * skip if component not within archetype
         */
        if(!vecsComponentSetContainsId(
            archetypePtr->_componentSet,
            i
        )){
            continue;
        }

        VecsComponentMetadata componentMetadata
            = vecsComponentListGetMetadata(
                archetypePtr->_componentListPtr,
                i
            );
        /* skip if component is a marker */
        if(componentMetadata._componentSize == 0){
            return;
        }

        ArrayList *componentStorageListPtr
            = &(archetypePtr
                ->_componentStorageLists[i]);

        componentStorageListFree(
            componentStorageListPtr,
            componentMetadata
        );
    }
}

/*
 * Frees the memory associated with the given
 * archetype
 */
void _vecsArchetypeFree(_VecsArchetype *archetypePtr){
    /* component set does not need to be freed */

    /*
     * free component storage and update entity
     * metadata
     */
    __vecsArchetypeFreeComponentStorageLists(
        archetypePtr
    );

    /*
     * the array of arraylists does not need to be
     * freed 
     */

    archetypePtr->_componentListPtr = NULL;
    archetypePtr->_entityListPtr = NULL;

    /*
     * increase modification count in case iterators
     * still out there
     */
    ++(archetypePtr->_modificationCount);
}

/* Returns an iterator over the specified archetype */
_VecsArchetypeItr _vecsArchetypeItr(
    _VecsArchetype *archetypePtr
){
    return (_VecsArchetypeItr) {
        ._archetypePtr = archetypePtr,
        ._currentIndex = 0,
        ._storedModificationCount
            = archetypePtr->_modificationCount
    };
}

/*
 * Throws error if concurrent modification detected
 * for the specified archetype iterator
 */
static void errorIfConcurrentModification(
    _VecsArchetypeItr *itrPtr
){
    if(!itrPtr || !(itrPtr->_archetypePtr)){
        return;
    }
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
bool _vecsArchetypeItrHasEntity(
    _VecsArchetypeItr *itrPtr
){
    errorIfConcurrentModification(itrPtr);

    _VecsArchetype *archetypePtr
        = itrPtr->_archetypePtr;

    /*
     * check the storage list for entity; entity is
     * present in every archetype
     */
    ArrayList *componentStorageListPtr
        = &(archetypePtr
            ->_componentStorageLists[VecsEntityId]);
    
    return itrPtr->_currentIndex
        < componentStorageListPtr->size;
}

/*
 * Advances the specified archetype itr to point to the
 * next entity
 */
void _vecsArchetypeItrAdvance(
    _VecsArchetypeItr *itrPtr
){
    errorIfConcurrentModification(itrPtr);
    ++(itrPtr->_currentIndex);
}

/*
 * Returns a pointer to the component specified by
 * the given component id of the entity currently being
 * pointed to by the given archetype iterator; error
 * if the component id is invalid; returns NULL if the
 * component is a marker
 */
void *__vecsArchetypeItrGetPtr(
    _VecsArchetypeItr *itrPtr,
    VecsComponentId componentId
){
    errorIfConcurrentModification(itrPtr);
    assertTrue(
        _vecsArchetypeItrHasEntity(itrPtr),
        "error: archetype itr out of entities; "
        SRC_LOCATION
    );
        
    _VecsArchetype *archetypePtr
        = itrPtr->_archetypePtr;
    _vecsArchetypeErrorIfBadComponent(
        archetypePtr,
        componentId
    );

    VecsComponentMetadata componentMetadata
        = vecsComponentListGetMetadata(
            archetypePtr->_componentListPtr,
            componentId
        );

    if(componentMetadata._componentSize == 0){
        return NULL;
    }

    ArrayList *componentStorageListPtr
        = &(archetypePtr
            ->_componentStorageLists[componentId]);

    return _arrayListGetPtr(
        componentStorageListPtr,
        itrPtr->_currentIndex,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata->_typeName
        #endif
    );
}