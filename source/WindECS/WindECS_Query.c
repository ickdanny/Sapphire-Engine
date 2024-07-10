#include "WindECS_Query.h"

#define archetypeIndexListInitCapacity 10

/*
 * throws an error if the accept component set and the
 * reject component set have any overlap
 */
static void errorIfAcceptRejectOverlap(
    Bitset *acceptComponentSetPtr,
    Bitset *rejectComponentSetPtr
){
    assertNotNull(
        acceptComponentSetPtr,
        "null accept ptr; " SRC_LOCATION
    );
    assertNotNull(
        acceptComponentSetPtr,
        "null reject ptr; " SRC_LOCATION
    );
    Bitset tempBitset = bitsetCopy(
        acceptComponentSetPtr
    );
    bitsetAnd(&tempBitset, rejectComponentSetPtr);
    assertTrue(
        bitsetNone(&tempBitset),
        "error: query accept and reject overlap; "
        SRC_LOCATION
    );
    bitsetFree(&tempBitset);
}

/*
 * Constructs and returns a new WindQuery with the
 * specified component set and blocked component
 * set (NULL can be passed for the blocked set);
 * the new query makes copies of the passed bitsets;
 * iterates over the given archetype list and accepts
 * all fitting archetypes automatically
 */
WindQuery windQueryMake(
    Bitset *acceptComponentSetPtr,
    Bitset *rejectComponentSetPtr,
    ArrayList *archetypeListPtr
){
    assertNotNull(
        acceptComponentSetPtr,
        "error: query needs a non-null component set; "
        SRC_LOCATION
    );
    WindQuery toRet = {0};
    toRet._acceptComponentSet
        = bitsetCopy(acceptComponentSetPtr);
    if(rejectComponentSetPtr){
        errorIfAcceptRejectOverlap(
            acceptComponentSetPtr,
            rejectComponentSetPtr
        );
        toRet._rejectComponentSet
            = bitsetCopy(rejectComponentSetPtr);
        toRet._hasRejectComponentSet = true;
    }
    else{
        toRet._hasRejectComponentSet = false;
    }
    toRet._archetypeListPtr = archetypeListPtr;
    toRet._archetypeIndexList = arrayListMake(
        size_t,
        archetypeIndexListInitCapacity
    );
    toRet._modificationCount = 0;

    /* try to accept every current archetype */
    for(size_t i = 0; i < archetypeListPtr->size; ++i){
        windQueryTryAcceptArchetype(&toRet, i);
    }
    return toRet;
}

/*
 * Adds the specified archetype into the given query
 * if it fits the query restrictions, otherwise does
 * nothing; returns true if the archetype was accepted,
 * false otherwise
 */
bool windQueryTryAcceptArchetype(
    WindQuery *queryPtr,
    size_t archetypeIndex
){
    _WindArchetype *archetypePtr = arrayListGetPtr(
        _WindArchetype,
        queryPtr->_archetypeListPtr,
        archetypeIndex
    );
    Bitset tempBitset = {0};
    bool reject = false;
    /*
     * reject if archetype has a component which
     * the query rejects
     */
    if(queryPtr->_hasRejectComponentSet){
        tempBitset = bitsetCopy(
            &(queryPtr->_rejectComponentSet)
        );
        bitsetAnd(
            &tempBitset,
            &(archetypePtr->_componentSet)
        );
        if(bitsetAny(&tempBitset)){
            reject = true;
        }
        bitsetFree(&tempBitset);
    }
    /*
     * if not rejected, test if the archetype has all
     * the accept components
     */
    if(!reject){
        tempBitset = bitsetCopy(
            &(queryPtr->_acceptComponentSet)
        );
        bitsetAnd(
            &tempBitset,
            &(archetypePtr->_componentSet)
        );
        /*
         * if the result of bitwise AND between the
         * query accept component set and the
         * archetype's component set is not just the
         * accept component set again, it means that
         * at least one component is missing and the
         * archetype should be rejected
         */
        reject = !bitsetEquals(
            &tempBitset,
            &(queryPtr->_acceptComponentSet)
        );
        bitsetFree(&tempBitset);
    }
    /*
     * if still not rejected, add the archetype
     * index and advance the query modification count
     */
    if(!reject){
        arrayListPushBack(size_t,
            &(queryPtr->_archetypeIndexList),
            archetypeIndex
        );
        ++(queryPtr->_modificationCount);
    }

    return !reject;
}

/*
 * Frees all memory associated with the specified
 * WindQuery
 */
void windQueryFree(WindQuery *queryPtr){
    /* free bitsets */
    bitsetFree(&(queryPtr->_acceptComponentSet));
    if(queryPtr->_hasRejectComponentSet){
        bitsetFree(&(queryPtr->_rejectComponentSet));
    }

    /* let go of the archetype list weak ptr */
    queryPtr->_archetypeListPtr = NULL;

    /*
     * free the archetypeIndex arraylist but not the
     * archetypes themselves since they are not
     * owned by the query
     */
    arrayListFree(size_t,
        &(queryPtr->_archetypeIndexList)
    );

    /*
     * increase modification count in case iterators
     * still out there
     */
    ++(queryPtr->_modificationCount);

    /*
     * Set _hasRejectComponentSet to false just in case
     */
    queryPtr->_hasRejectComponentSet = false;
}

/*
 * Gets a pointer to the nth archetype in the
 * specified query; returns NULL if the archetype
 * index is out of bounds
 */
static _WindArchetype *_windQueryGetArchetypePtr(
    WindQuery *queryPtr, size_t archetypeIndex
){
    /* return null if out of bounds */
    if(archetypeIndex 
        >= queryPtr->_archetypeIndexList.size
    ){
        return NULL;
    }
    return arrayListGetPtr(_WindArchetype,
        queryPtr->_archetypeListPtr,
        arrayListGet(size_t,
            &(queryPtr->_archetypeIndexList),
            archetypeIndex
        )
    );
}

/*
 * If the current archetype itr is out of entities,
 * advance until the next non-empty itr is reached;
 * returns true if such an itr was found, false if
 * the whole query itr is out of entities (in this
 * case, also sets the queryPtr to NULL)
 */
static bool _windQuerySkipEmptyArchetypes(
    WindQueryItr *itrPtr
){
    if(!itrPtr || !(itrPtr->_queryPtr)){
        return false;
    }
    _WindArchetype *archetypePtr = NULL;
    while(!_windArchetypeItrHasEntity(
        &(itrPtr->_archetypeItr)
    )){
        ++(itrPtr->_currentArchetypeIndex);
        archetypePtr = _windQueryGetArchetypePtr(
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
        itrPtr->_archetypeItr = _windArchetypeItr(
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
    WindQueryItr *itrPtr
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
WindQueryItr windQueryItr(WindQuery *queryPtr){
    WindQueryItr toRet = {0};
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
    toRet._archetypeItr = _windArchetypeItr(
        _windQueryGetArchetypePtr(queryPtr, 0)
    );
    /*
     * if we keep getting empty archetypes,
     * try new ones
     */
    _windQuerySkipEmptyArchetypes(&toRet);
    return toRet;
}

/*
 * Returns true if the specified query iterator
 * has more elements, false otherwise
 */
bool windQueryItrHasEntity(WindQueryItr *itrPtr){
    errorIfConcurrentModification(itrPtr);
    /*
     * if the current archetype itr has entities,
     * _windQuerySkipEmptyArchetypes will return
     * true; otherwise, it will skip to the next
     * non-empty archetype and return false only
     * if there are no such archetypes left
     */
    return _windQuerySkipEmptyArchetypes(itrPtr);
}

/*
 * Advances the query itr to point to the next entity
 */
void windQueryItrAdvance(WindQueryItr *itrPtr){
    /* if null passed or itr is invalid, do nothing */
    if(!itrPtr || !(itrPtr->_queryPtr)){
        return;
    }
    errorIfConcurrentModification(itrPtr);

    _windArchetypeItrAdvance(&(itrPtr->_archetypeItr));
    _windQuerySkipEmptyArchetypes(itrPtr);
}

/*
 * Returns a pointer to the component specified by
 * the given componentID of the entity currently being
 * pointed to by the given query iterator; error
 * if the componentID is invalid; returns NULL if the
 * component is a marker
 */
void *_windQueryItrGetPtr(
    WindQueryItr *itrPtr,
    WindComponentIDType componentID
){
    assertTrue(
        windQueryItrHasEntity(itrPtr),
        "error: query itr has no entities left; "
        SRC_LOCATION
    );

    /* error if component ID is invalid for query */
    assertTrue(
        bitsetGet(
            &(itrPtr->_queryPtr->_acceptComponentSet),
            componentID
        ),
        "error: trying to get component type not in "
        "the query accept set; "
        SRC_LOCATION
    );

    /*
     * return the component from archetype itr, which
     * yields NULL if the component is a marker
     */
    return __windArchetypeItrGetPtr(
        &(itrPtr->_archetypeItr), 
        componentID
    );
}

/*
 * Returns the ID of the etity curently being pointed
 * to by the given query iterator; error if the query
 * is out of entities
 */
WindEntityIDType windQueryItrCurrentID(
    WindQueryItr *itrPtr
){
    assertTrue(
        windQueryItrHasEntity(itrPtr),
        "error: query itr has no entities left; "
        SRC_LOCATION
    );
    return _windArchetypeItrCurrentID(
        &(itrPtr->_archetypeItr)
    );
}