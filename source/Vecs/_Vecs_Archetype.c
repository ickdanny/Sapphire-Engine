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
 * Clears all component data from the specified
 * archetype
 */
void _vecsArchetypeClear(_VecsArchetype *archetypePtr){

    //todo: reset entity metadata?
    /* assume all archetypes have component 0 */
    ArrayList *entityListPtr
        = &(archetypePtr->_componentStorageLists[0]);
    for(size_t i = 0; i < entityListPtr->size; ++i){
        //todo: kill all listed entities
    }

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

/*
 * Errors if the specified entity id is not within the
 * given archetype
 */
void _vecsArchetypeErrorIfBadEntity(
    _VecsArchetype *archetypePtr,
    VecsEntity entity
){
    //todo: component 0 for entity id to get metadata
}