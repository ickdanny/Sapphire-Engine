#include "Vecs_Query.h"

#define archetypeIndexListInitCapacity 10

/*
 * Errors if accept and reject component sets overlap
 */
static void errorIfAcceptRejectOverlap(
    VecsComponentSet acceptComponentSet,
    VecsComponentSet rejectComponentSet
){
    assertFalse(
        vecsComponentSetContainsAny(
            acceptComponentSet,
            rejectComponentSet
        ),
        "error: query accept and reject overlap; "
        SRC_LOCATION
    );
}

/*
 * Constructs and returns a new query with the
 * specified accept and reject component sets;
 * iterates over the given archetype list and accepts
 * all fitting archetypes automatically
 */
VecsQuery vecsQueryMake(
    VecsComponentSet acceptComponentSet,
    VecsComponentSet rejectComponentSet,
    ArrayList *archetypeListPtr
){
    errorIfAcceptRejectOverlap(
        acceptComponentSet,
        rejectComponentSet
    );
    assertNotNull(
        archetypeListPtr,
        "error: null passed to vecsQueryMake; "
        SRC_LOCATION
    );
    VecsQuery toRet = {
        ._acceptComponentSet = acceptComponentSet,
        ._rejectComponentSet = rejectComponentSet,
        ._archetypeListPtr = archetypeListPtr,
        ._archetypeIndexList = arrayListMake(size_t,
            archetypeIndexListInitCapacity
        ),
        ._modificationCount = 0
    };

    /* try to accept every current archetype */
    for(size_t i = 0; i < archetypeListPtr->size; ++i){
        _vecsQueryTryAcceptArchetype(&toRet, i);
    }
    return toRet;
}

/*
 * Adds the specified archetype into the given query
 * if it fits the query restrictions, otherwise does
 * nothing; returns true if the archetype was accepted,
 * false otherwise
 */
bool _vecsQueryTryAcceptArchetype(
    VecsQuery *queryPtr,
    size_t archetypeIndex
){
    _VecsArchetype *archetypePtr = arrayListGetPtr(
        _VecsArchetype,
        queryPtr->_archetypeListPtr,
        archetypeIndex
    );

    assertNotNull(
        archetypePtr,
        "error: retrieved null for archetype; "
        SRC_LOCATION
    );

    /*
     * reject if archetype has a component which the
     * query rejects
     */
    if(vecsComponentSetContainsAny(
        archetypePtr->_componentSet,
        queryPtr->_rejectComponentSet
    )){
        return false;
    }

    /*
     * reject if archetype does not have all components
     * required by the query
     */
    if(!vecsComponentSetContainsAll(
        archetypePtr->_componentSet,
        queryPtr->_acceptComponentSet
    )){
        return false;
    }

    arrayListPushBack(size_t,
        &(queryPtr->_archetypeIndexList),
        archetypeIndex
    );
    ++(queryPtr->_modificationCount);
    return true;
}

/*
 * Frees all memory associated with the specified
 * query
 */
void vecsQueryFree(VecsQuery *queryPtr){
    queryPtr->_acceptComponentSet
        = vecsEmptyComponentSet;
    queryPtr->_rejectComponentSet
        = vecsEmptyComponentSet;
    queryPtr->_archetypeListPtr = NULL;
    arrayListFree(size_t,
        &(queryPtr->_archetypeIndexList)
    );

    /*
     * increase modification count in case iterators
     * still out there
     */
    ++(queryPtr->_modificationCount);
}

/*
 * Gets a pointer to the nth archetype in the
 * specified query; returns NULL if the archetype
 * index is out of bounds
 */
static _VecsArchetype *_vecsQueryGetArchetypePtr(
    VecsQuery *queryPtr,
    size_t archetypeIndex
){
    /* return null if out of bounds */
    if(archetypeIndex 
        >= queryPtr->_archetypeIndexList.size
    ){
        return NULL;
    }

    size_t indexInArchetypeList
        = arrayListGet(size_t,
            &(queryPtr->_archetypeIndexList),
            archetypeIndex
        );

    return arrayListGetPtr(_VecsArchetype,
        queryPtr->_archetypeListPtr,
        indexInArchetypeList
    );
}

/*
 * If the current archetype itr is out of entities,
 * advance until the next non-empty itr is reached;
 * returns true if such an itr was found, false if
 * the whole query itr is out of entities (in this
 * case, also sets the queryPtr to NULL)
 */
static bool _vecsQueryItrSkipEmptyArchetypes(
    VecsQueryItr *itrPtr
){
    if(!itrPtr || !(itrPtr->_queryPtr)){
        return false;
    }
    _VecsArchetype *archetypePtr = NULL;
    while(!_vecsArchetypeItrHasEntity(
        &(itrPtr->_archetypeItr)
    )){
        ++(itrPtr->_currentArchetypeIndex);
        archetypePtr = _vecsQueryGetArchetypePtr(
            itrPtr->_queryPtr,
            itrPtr->_currentArchetypeIndex
        );
        /*
         * if run out of archetypes, indicate no
         * entities and return
         */
        if(!archetypePtr){
            itrPtr->_queryPtr = NULL;
            return false;
        }
        itrPtr->_archetypeItr = _vecsArchetypeItr(
            archetypePtr
        );
    }
    return true;
}

/*
 * Throws error if concurrent modification detected
 * for the specified query iterator
 */
static void errorIfConcurrentModification(
    VecsQueryItr *itrPtr
){
    if(!itrPtr || !(itrPtr->_queryPtr)){
        return;
    }
    assertTrue(
        itrPtr->_storedModificationCount
            == itrPtr->_queryPtr
                ->_modificationCount,
        "error: query concurrent modification; "
        SRC_LOCATION
    );
}

/* Returns an iterator over the specified query */
VecsQueryItr vecsQueryItr(VecsQuery *queryPtr){
    VecsQueryItr toRet = {0};
    /*
     * if the query has no archetypes, set _queryPtr
     * to NULL to indicate no entities left and return
     */
    if(arrayListIsEmpty(
        &(queryPtr->_archetypeIndexList)
    )){
        toRet._queryPtr = NULL;
        return toRet;
    }
    toRet._queryPtr = queryPtr;
    toRet._currentArchetypeIndex = 0;
    toRet._storedModificationCount
        = queryPtr->_modificationCount;
    
    /*
     * init archetype itr to the first archetype; at
     * least one archetype is known to be present
     * since the case of 0 archetypes was checked for
     * earlier
     */
    toRet._archetypeItr = _vecsArchetypeItr(
        _vecsQueryGetArchetypePtr(queryPtr, 0)
    );
    /*
     * if we keep getting empty archetypes,
     * try new ones
     */
    _vecsQueryItrSkipEmptyArchetypes(&toRet);
    return toRet;
}

/*
 * Returns true if the specified query iterator
 * has more elements, false otherwise
 */
bool vecsQueryItrHasEntity(VecsQueryItr *itrPtr){
    errorIfConcurrentModification(itrPtr);
    /*
     * if the current archetype itr has entities,
     * _vecsQuerySkipEmptyArchetypes will return
     * true; otherwise, it will skip to the next
     * non-empty archetype and return false only
     * if there are no such archetypes left
     */
    return _vecsQueryItrSkipEmptyArchetypes(itrPtr);
}

/*
 * Advances the query itr to point to the next entity
 */
void vecsQueryItrAdvance(VecsQueryItr *itrPtr){
    if(!itrPtr || !(itrPtr->_queryPtr)){
        return;
    }
    errorIfConcurrentModification(itrPtr);

    _vecsArchetypeItrAdvance(&(itrPtr->_archetypeItr));
    _vecsQueryItrSkipEmptyArchetypes(itrPtr);
}

/*
 * Returns a pointer to the component specified by
 * the given component id of the entity currently being
 * pointed to by the given query iterator; error
 * if the component id is invalid; returns NULL if the
 * component is a marker
 */
void *_vecsQueryItrGetPtr(
    VecsQueryItr *itrPtr,
    VecsComponentId componentId
){
    assertTrue(
        vecsQueryItrHasEntity(itrPtr),
        "error: query itr has no entities left; "
        SRC_LOCATION
    );

    /* error if component id is invalid for query */
    assertTrue(
        vecsComponentSetContainsId(
            itrPtr->_queryPtr->_acceptComponentSet,
            componentId
        ),
        "error: trying to get component type not in "
        "the query accept set; "
        SRC_LOCATION
    );

    /*
     * return the component from archetype itr, which
     * yields NULL if the component is a marker
     */
    return __vecsArchetypeItrGetPtr(
        &(itrPtr->_archetypeItr), 
        componentId
    );
}