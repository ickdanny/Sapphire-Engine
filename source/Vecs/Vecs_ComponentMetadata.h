#ifndef VECS_COMPONENTMETADATA_H
#define VECS_COMPONENTMETADATA_H

#include <stddef.h>
#include <stdint.h>

#include "Constructure.h"

//todo: put component set in separate file

/*
 * VECS supports up to 64 components, each one having
 * id of the form 2^k
 */
typedef uint64_t VecsComponentSet;

/*
 * Each defined component must have an accompanying
 * global symbol TYPENAME##ID defined
 */
#define vecsComponentGetID(TYPENAME) TYPENAME##ID

/* The function prototype for component destructors */
typedef void VecsComponentDestructorType(void*);

/* Stores RTTI for components */
typedef struct VecsComponentMetadata{
    /* the size of the component in bytes */
    size_t _componentSize;
    /*
     * a pointer to the destructor for the component;
     * can be NULL
     */
    VecsComponentDestructorType _destructor;

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
    VecsComponentDestructorType destructor
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

// todo: put component list in separate file

/* Stores RTTI for all components of an ECS world */
typedef struct WindComponents{
    Array _componentArray;
    /*
     * used to indicate whether a component of a
     * specific ID has already been specified
     */
    Bitset _setComponentIDs;
} WindComponents;

/*
 * Constructs and returns a new empty WindComponents
 * by value
 */
WindComponents windComponentsMake(
    WindComponentIDType numComponents
);

/*
 * Inserts the specified WindComponentMetadata into
 * the given WindComponents at the specified ID; error
 * if the componentID has already been used
 */
void windComponentsInsert(
    WindComponents *componentsPtr,
    WindComponentMetadata componentMetadata,
    WindComponentIDType componentID 
);

/*
 * Returns a copy of the component metadata for the
 * specified ID
 */
WindComponentMetadata windComponentsGet(
    WindComponents *componentsPtr,
    WindComponentIDType componentID
);

/* Returns the number of components */
#define windComponentsNumComponents(COMPONENTSPTR) \
    ((COMPONENTSPTR)->_componentArray.size)

/*
 * Frees the memory associated with the specified
 * WindComponents
 */
void windComponentsFree(WindComponents *componentsPtr);

#endif