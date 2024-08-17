#include "BLoader_ResourceLoader.h"

#include "Trifecta.h"

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
){
    BLResourceType toRet = {
        fileExtension,
        parsingFunction,
        userPtr
    };
    return toRet;
}

/*
 * Constructs and returns a new BLResourceLoader
 * by value
 */
BLResourceLoader blResourceLoaderMake(){
    BLResourceLoader toRet = {0};
    toRet._fileExtensionToTypeMap = hashMapMake(
        char *,
        BLResourceType,
        10,
        cStringHash,
        cStringEquals
    );
    return toRet;
}

/*
 * Registers the specified BLResourceType with the
 * given BLResourceLoader; returns true on success,
 * false on failure
 */
bool blResourceLoaderRegisterType(
    BLResourceLoader *loaderPtr,
    BLResourceType *typePtr
){
    /* return false if invalid pointers */
    if(!loaderPtr || !typePtr){
        return false;
    }
    /* return false if file extension already used */
    if(hashMapHasKey(char *, BLResourceType,
        &(loaderPtr->_fileExtensionToTypeMap),
        typePtr->_fileExtension
    )){
        return false;
    }

    /* put new mapping */
    hashMapPutPtr(char *, BLResourceType,
        &(loaderPtr->_fileExtensionToTypeMap),
        &(typePtr->_fileExtension),
        typePtr
    );
    return true;
}

/* 
 * Returns a pointer to the start of the file extension
 * within the given string, or NULL if no period is
 * present
 */
static char *findFileExtension(char *string){
    int stringLength = strlen(string);

    /* find index of last slash */
    int slashIndex = stringLength - 1;
    while(slashIndex >= 0
        && string[slashIndex] != '/'
    ){
        --slashIndex;
    }
    /* find index of last dot in the file name */
    int dotIndex = stringLength - 1;
    while(dotIndex > slashIndex
        && string[dotIndex] != '.'
    ){
        --dotIndex;
    }
    /* if failed to find dot, return */
    if(dotIndex == slashIndex){
        return NULL;
    }
    /* 
     * if dot's next character is null, we have found
     * the current directory so return NULL
     */
    if(string[dotIndex + 1] == '\0'){
        return NULL;
    }
    /*
     * otherwise return a ptr to the start of the file
     * extension
     */
    return string + (dotIndex + 1);
}

/*
 * Loads all files within the specified directory
 * using the given BLResourceLoader
 */
void blResourceLoaderParseDirectory(
    BLResourceLoader *loaderPtr,
    const char *directoryName
){
    /* used to store file names from the directory */
    #define bufferSize 1024
    static char fileNameBuffer[bufferSize] = {0};

    TFDirectory directory
        = tfDirectoryOpen(directoryName);
    
    while(true){
        tfDirectoryGetNextFileName(
            &directory,
            fileNameBuffer,
            bufferSize
        );
        /* stop looping if ran out of files */
        if(fileNameBuffer[0] == '\0'){
            break;
        }

        /* get file extension of every file */
        char *fileExtensionPtr
            = findFileExtension(fileNameBuffer);
        /*
         * if failed to get file extension, go to next
         * directory entry
         */
        if(!fileExtensionPtr){
            continue;
        }

        /*
         * get the resource type for the extension 
         * via the map
         */
        BLResourceType *typePtr = hashMapGetPtr(
            char *,
            BLResourceType,
            &(loaderPtr->_fileExtensionToTypeMap),
            &(fileExtensionPtr)
        );
        /*
         * if failed to get type ptr, go to next
         * directory entry (it's a file type that
         * we don't know how to load)
         */
        if(!typePtr){
            continue;
        }
        /* call the parsing function */
        typePtr->_parsingFunction(
            fileNameBuffer,
            typePtr->_userPtr
        );
    }

    tfDirectoryFree(&directory);

    #undef bufferSize
}

/*
 * Frees the memory associated with the specified
 * BLResourceLoader
 */
void blResourceLoaderFree(BLResourceLoader *loaderPtr){
    /*
     * Resource Types do not own any of their data,
     * and the key cStrings are not owned by the
     * hashmap (they are string literals), so no need
     * to apply a free func to the hashmap
     */
    hashMapFree(
        char *,
        BLResourceType,
        &(loaderPtr->_fileExtensionToTypeMap)
    );
}