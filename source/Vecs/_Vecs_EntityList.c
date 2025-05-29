#include "_Vecs_EntityList.h"

/*
 * Constructs and returns a new empty entity list
 * by value
 */
_VecsEntityList _vecsEntityListMake(
    size_t maxEntities
){
    _VecsEntityList toRet = {
        ._entityMetadataArray = arrayMake(
            _VecsEntityMetadata,
            maxEntities
        ),
        ._nextCreatedEntityId = 0,
        ._numEntities = 0
    };

    /*
     * initializes each entity metadata as
     * generation 0, not live, and with id equal to
     * array index
     */
    for(size_t i = 0; i < maxEntities; ++i){
        arraySet(_VecsEntityMetadata,
            &(toRet._entityMetadataArray),
            i,
            _vecsEntityMetadataMake(i)
        );
    }

    return toRet;
}

/* Clears all entity data from the given entity list */
void _vecsEntityListClear(
    _VecsEntityList *entityListPtr
){
    //todo
}