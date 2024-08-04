#ifndef WIND_COMPONENTS_H
#define WIND_COMPONENTS_H

#include <stddef.h>
#include <stdint.h>

#include "Constructure.h"

typedef uint32_t WindComponentIDType;

/*
 * Each defined component must have an accompanying
 * global symbol TYPENAME##ID defined
 */
#define windComponentGetID(TYPENAME) TYPENAME##ID

/* The function prototype for component destructors */
typedef void (*WindComponentDestructorType)(void*);

/* Stores RTTI for components */
typedef struct WindComponentMetadata{
    /* the size of the component in bytes */
    size_t _componentSize;
    /*
     * a pointer to the destructor for the component;
     * can be NULL
     */
    WindComponentDestructorType _destructor;

    #ifdef _DEBUG
    /* 
     * Should only ever point to a string literal,
     * thus should not be freed
     */
    const char *_typeName;
    #endif
} WindComponentMetadata;

/*
 * Constructs and returns a new component metadata
 * by value
 */
WindComponentMetadata _windComponentMetadataMake(
    size_t componentSize,
    WindComponentDestructorType destructor
    #ifdef _DEBUG 
    , const char *typeName 
    #endif
);

#ifndef _DEBUG
/*
 * Constructs and returns a new component metadata
 * by value
 */
#define windComponentMetadataMake( \
    TYPENAME, \
    DESTRUCTOR \
) \
    _windComponentMetadataMake( \
        sizeof(TYPENAME), \
        DESTRUCTOR \
    )
#else
/*
 * Constructs and returns a new component metadata
 * by value
 */
#define windComponentMetadataMake( \
    TYPENAME, \
    DESTRUCTOR \
) \
    _windComponentMetadataMake( \
        sizeof(TYPENAME), \
        DESTRUCTOR, \
        #TYPENAME \
    )
#endif

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