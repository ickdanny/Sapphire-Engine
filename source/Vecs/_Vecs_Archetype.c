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
static void __componentStorageClear(
    ArrayList *componentStoragePtr,
    VecsComponentMetadata componentMetadata
){
    /* run destructor on all elements in storage */
    if(componentMetadata._destructor){
        for(size_t i = 0;
            i < componentStoragePtr->size;
            ++i
        ){
            componentMetadata._destructor(
                _arrayListGetPtr(
                    componentStoragePtr,
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
        componentStoragePtr,
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

        __componentStorageClear(
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
 * the given component id of the entity specified by
 * the given entity id; error if the component or the
 * entity is invalid; returns NULL if the component
 * is a marker
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

    ArrayList *componentListPtr
        = &(archetypePtr
            ->_componentStorageLists[componentId]);

    return _arrayListGetPtr(
        componentListPtr,
        index,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typename
        #endif
    );
}

/*
 * Sets the component specified by the given
 * component id of the entity specified by the given
 * entity id to the value stored in the given void ptr;
 * error if the component id is invalid; does nothing
 * if NULL is passed or if the component is a marker
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

    ArrayList *componentListPtr
        = &(archetypePtr
            ->_componentStorageLists[componentId]);

    void *componentSlotPtr = _arrayListGetPtr(
        componentListPtr,
        index,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typename
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
        componentListPtr,
        index,
        componentPtr,
        componentMetadata._componentSize
        #ifdef _DEBUG
        , componentMetadata._typename
        #endif
    );

    ++(archetypePtr->_modificationCount);
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

        ArrayList *srcComponentListPtr
            = &(srcArchetypePtr
                ->_componentStorageLists[i]);

        void *srcComponentPtr = _arrayListGetPtr(
            srcComponentListPtr,
            index,
            componentMetadata._componentSize
            #ifdef _DEBUG
            , componentMetadata._typename
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
            ArrayList *destComponentListPtr
                = &(destArchetypePtr
                    ->_componentStorageLists[i]);
            _arrayListPushBackPtr(
                destComponentListPtr,
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
        //todo: implement this
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

}




/*
 * Frees all the memory associated with the specified
 * component storage using the provided RTTI
 */
static void __componentStorageFree(
    ArrayList *componentStoragePtr,
    VecsComponentMetadata componentMetadata
){
    /* run destructor on all elements in storage */
    __componentStorageClear(
        componentStoragePtr,
        componentMetadata
    );

    /* free the storage */
    _arrayListFree(
        componentStoragePtr
        #ifdef _DEBUG
        , componentMetadata._typeName
        #endif
    );
}