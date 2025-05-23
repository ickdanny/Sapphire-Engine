#ifndef VECS_COMPONENTMETADATA_H
#define VECS_COMPONENTMETADATA_H

#include <stddef.h>

#include "Vecs_Component.h"

/* The function prototype for component destructors */
typedef void (*VecsComponentDestructor)(void*);

/* Stores RTTI for components */
typedef struct VecsComponentMetadata{
    /* the size of the component in bytes */
    size_t _componentSize;
    /*
     * a pointer to the destructor for the component;
     * can be NULL
     */
    VecsComponentDestructor _destructor;

    #ifdef _DEBUG
    /* 
     * Should only ever point to a string literal,
     * thus should not be freed
     */
    const char *_typeName;
    #endif
} VecsComponentMetadata;

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
);

#ifndef _DEBUG
/*
 * Constructs and returns a new component metadata
 * by value; destructor is nullable
 */
#define vecsComponentMetadataMake( \
    TYPENAME, \
    DESTRUCTOR \
) \
    _vecsComponentMetadataMake( \
        sizeof(TYPENAME), \
        DESTRUCTOR \
    )
#else
/*
 * Constructs and returns a new component metadata
 * by value; destructor is nullable
 */
#define vecsComponentMetadataMake( \
    TYPENAME, \
    DESTRUCTOR \
) \
    _vecsComponentMetadataMake( \
        sizeof(TYPENAME), \
        DESTRUCTOR, \
        #TYPENAME \
    )
#endif

#endif