#include "Resources.h"

#define initImageCapacity 200
#define initMidiCapacity 20

/*
 * Isolates the file name from the given file path
 * and returns it as a WideString by value
 */
WideString isolateFileName(const char *fileName){
    WideString toRet = wideStringMakeEmpty();

    /* find last index of the period */
    int stringLength = strlen(fileName);

    /* find index of last slash */
    int slashIndex = stringLength - 1;
    while(slashIndex >= 0
        && fileName[slashIndex] != '/'
    ){
        --slashIndex;
    }
    /* find index of last dot in the file name */
    int dotIndex = stringLength - 1;
    while(dotIndex > slashIndex
        && fileName[dotIndex] != '.'
    ){
        --dotIndex;
    }

    int startIndexInclusive = slashIndex + 1;
    /*
     * if start index is out of bounds, return
     * empty string
     */
    if(startIndexInclusive >= stringLength){
        return toRet;
    }
    int stopIndexExclusive = dotIndex;
    /* if failed to find dot, read until end  */
    if(dotIndex == slashIndex){
        stopIndexExclusive = stringLength;
    }
    
    /* convert chars 1 by 1 to the wide string */
    for(int i = startIndexInclusive;
        i < stopIndexExclusive;
        ++i
    ){
        wideStringPushBack(
            &toRet,
            fileName[i]
        );
    }

    return toRet;
}

/* Image loading callback */
static void loadImageIntoResources(
    const char *fileName,
    void *imageMapVoidPtr
){
    HashMap *imageMapPtr = imageMapVoidPtr;

    TFSprite sprite = parseBitmapFile(fileName);
    WideString stringID = isolateFileName(fileName);
    if(hashMapHasKeyPtr(WideString, TFSprite,
        imageMapPtr,
        &stringID
    )){
        pgWarning(fileName);
        pgError("try to load multiple of same image");
    }
    hashMapPutPtr(WideString, TFSprite,
        imageMapPtr,
        &stringID,
        &sprite
    );
}

/* Midi loading callback */
static void loadMidiIntoResources(
    const char *fileName,
    void *midiMapVoidPtr
){
    HashMap *midiMapPtr = midiMapVoidPtr;

    MidiSequence midi = parseMidiFile(fileName);
    WideString stringID = isolateFileName(fileName);
    if(hashMapHasKeyPtr(WideString, MidiSequence,
        midiMapPtr,
        &stringID
    )){
        pgWarning(fileName);
        pgError("try to load multiple of same midi");
    }
    hashMapPutPtr(WideString, MidiSequence,
        midiMapPtr,
        &stringID,
        &midi
    );
}

/*
 * Constructs and returns a new (empty) Resources
 * object by value
 */
Resources resourcesMake(){
    Resources toRet = {0};
    
    /* create maps */
    toRet._imageMapPtr = pgAlloc(
        1,
        sizeof(*(toRet._imageMapPtr))
    );
    toRet._midiMapPtr = pgAlloc(
        1,
        sizeof(*(toRet._midiMapPtr))
    );
    (*toRet._imageMapPtr) = hashMapMake(
        WideString, TFSprite,
        initImageCapacity,
        constructureWideStringHash,
        constructureWideStringEquals
    );
    (*toRet._midiMapPtr) = hashMapMake(
        WideString, MidiSequence,
        initMidiCapacity,
        constructureWideStringHash,
        constructureWideStringEquals
    );

    /* create and configure loader */
    toRet._loader = blResourceLoaderMake();
    BLResourceType imageType = blResourceTypeMake(
        "bmp",
        loadImageIntoResources,
        toRet._imageMapPtr
    );
    BLResourceType midiType = blResourceTypeMake(
        "mid",
        loadMidiIntoResources,
        toRet._midiMapPtr
    );
    blResourceLoaderRegisterType(
        &(toRet._loader),
        &imageType
    );
    blResourceLoaderRegisterType(
        &(toRet._loader),
        &midiType
    );

    return toRet;
}

/*
 * Nonrecursively loads all the files in the specified
 * directory into the given Resources object
 */
void resourcesLoadDirectory(
    Resources *resourcesPtr,
    char *directoryName
){
    blResourceLoaderParseDirectory(
        &(resourcesPtr->_loader),
        directoryName
    );
}

/*
 * Returns a pointer to the image resource specified
 * by the given WideString or NULL if no such image
 * exists
 */
TFSprite *resourcesGetSprite(
    Resources *resourcesPtr,
    WideString *wideStringPtr
){
    return hashMapGetPtr(WideString, TFSprite,
        resourcesPtr->_imageMapPtr,
        wideStringPtr
    );
}

/*
 * Returns a pointer to the midi resource specified
 * by the given WideString or NULL if no such midi
 * exists
 */
MidiSequence *resourcesGetMidi(
    Resources *resourcesPtr,
    WideString *wideStringPtr
){
    return hashMapGetPtr(WideString, MidiSequence,
        resourcesPtr->_midiMapPtr,
        wideStringPtr
    );
}

//todo get other resources (e.g. dialogue)

/*
 * Frees the memory associated with the specified
 * Resources object
 */
void resourcesFree(Resources *resourcesPtr){
    /* free loader*/
    blResourceLoaderFree(&(resourcesPtr->_loader));

    /* free image map */
    hashMapApply(WideString, TFSprite,
        resourcesPtr->_imageMapPtr,
        tfSpriteFree
    );
    hashMapKeyApply(WideString, TFSprite,
        resourcesPtr->_imageMapPtr,
        wideStringFree
    );
    hashMapFree(WideString, TFSprite,
        resourcesPtr->_imageMapPtr
    );
    pgFree(resourcesPtr->_imageMapPtr);

    /* free midi map */
    hashMapApply(WideString, MidiSequence,
        resourcesPtr->_midiMapPtr,
        midiSequenceFree
    );
    hashMapKeyApply(WideString, MidiSequence,
        resourcesPtr->_midiMapPtr,
        wideStringFree
    );
    hashMapFree(WideString, MidiSequence,
        resourcesPtr->_midiMapPtr
    );
    pgFree(resourcesPtr->_midiMapPtr);

    //todo free other resource types
}