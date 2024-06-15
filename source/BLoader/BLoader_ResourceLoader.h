#ifndef BLOADER_RESOURCELOADER_H
#define BLOADER_RESOURCELOADER_H

#include <stddef.h>

#include <Constructure.h>

typedef void (*BLParsingFunction)(const char*, void*);

/*
 * A resource type maps a file extention to a parsing
 * function; the parsing function will be provided
 * the file name as a C string and also a user-defined
 * pointer which is set in the resource type
 */
typedef struct BLResourceType{
    char *_fileExtension;
    BLParsingFunction _parsingFunction;
    void *_userPtr;
} BLResourceType;

/*
 * Constructs and returns a new BLResourceType 
 * by value; the file extension passed as a C string
 * should be a compile-time constant or should
 * otherwise not be modified after this function
 * is called
 */
BLResourceType blResourceTypeMake(
    char *fileExtension,
    BLParsingFunction parsingFunction,
    void *userPtr
);

/* 
 * A resource loader can be configured to load
 * resources from specified directories
 */
typedef struct BLResourceLoader{
    HashMap _fileExtensionToTypeMap;
} BLResourceLoader;

/*
 * Constructs and returns a new BLResourceLoader
 * by value
 */
BLResourceLoader blResourceLoaderMake();

/*
 * Registers the specified BLResourceType with the
 * given BLResourceLoader; returns true on success,
 * false on failure
 */
bool blResourceLoaderRegisterType(
    BLResourceLoader *loaderPtr,
    BLResourceType *typePtr
);

/*
 * Loads all files within the specified directory
 * using the given BLResourceLoader
 */
void blResourceLoaderParseDirectory(
    BLResourceLoader *loaderPtr,
    const char *directoryName
);

/*
 * Frees the memory associated with the specified
 * BLResourceLoader
 */
void blResourceLoaderFree(BLResourceLoader *loaderPtr);


#endif