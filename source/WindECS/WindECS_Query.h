#ifndef WINDECS_QUERY_H
#define WINDECS_QUERY_H

#include "Constructure.h"

/*
 * A Query provides access to a specific set Entities
 */
typedef struct WindQuery{
    /* 
     * Each set bit in the bitset represents a
     * component type which this query will require
     * entities have
     */
    Bitset _allComponentSet;
    /* 
     * Each set bit in the bitset represents a
     * component type which this query will require
     * entities not have
     */
    Bitset _noComponentSet;

    //todo: archetype array list?
} WindQuery;

//todo query itr
typedef struct WindQueryItr{

} WindQueryItr;

#endif