#include "WindECS_Query.h"

#define archetypeIndexListInitCapacity 10

/*
 * Constructs and returns a new WindQuery with the
 * specified component set and blocked component
 * set (NULL can be passed for the blocked set);
 * the new query makes copies of the passed bitsets
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
    bitsetFree(&(queryPtr->_rejectComponentSet));

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

/* Returns an iterator over the specified query */
WindQueryItr windQueryItr(WindQuery *queryPtr){
    WindQueryItr toRet = {0};
    /*
     * if the query has no archetypes, set _queryPtr
     * to NULL to indicate no entities left
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
    
    //todo so init archetype itr to first at construction?
}

/*
 * Returns true if the specified query iterator
 * has more elements, false otherwise
 */
bool windQueryItrHasEntity(WindQueryItr *itrPtr){
    if(!itrPtr || !(itrPtr->_queryPtr)){
        return false;
    }
    //todo actually check
}

/*
 * Advances the query itr to point to the next entity
 */
void windQueryItrAdvance(WindQueryItr *itrPtr){
    //todo
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
    WindComponentIDType componentID //todo: check for invalid in the query not just archetype
){
    //todo
}

/*
 * Returns the ID of the etity curently being pointed
 * to by the given query iterator; error if the query
 * is out of entities
 */
WindEntityIDType windQueryItrCurrentID(
    WindQueryItr *itrPtr
){
    //todo
}