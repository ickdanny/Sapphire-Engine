#include "Vecs_ComponentMetadata.h"

/*
 * Constructs and returns a new component metadata
 * by value
 */
VecsComponentMetadata _vecsComponentMetadataMake(
    size_t componentSize,
    VecsComponentDestructor destructor
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
){
    VecsComponentMetadata toRet = {
        componentSize,
        destructor
        #ifdef _DEBUG
        , typeName
        #endif
    };

    return toRet;
}