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
}

/* Image loading callback */
static void loadImageIntoResources(
    const char *fileName,
    void *resourcesVoidPtr
){
    Resources *resourcesPtr = resourcesVoidPtr;

    TFSprite sprite = parseBitmapFile(fileName);
    WideString stringID = isolateFileName(fileName);
    if(hashMapHasKeyPtr(WideString, TFSprite,
        &(resourcesPtr->_imageMap),
        &stringID
    )){
        pgWarning(fileName);
        pgError("try to load multiple of same image");
    }
    hashMapPutPtr(WideString, TFSprite,
        &(resourcesPtr->_imageMap),
        &stringID,
        &sprite
    );
}

/* Midi loading callback */
static void loadMidiIntoResources(
    const char *fileName,
    void *resourcesVoidPtr
){
    Resources *resourcesPtr = resourcesVoidPtr;

    MidiSequence midi = parseMidiFile(fileName);
    WideString stringID = isolateFileName(fileName);
    if(hashMapHasKeyPtr(WideString, MidiSequence,
        &(resourcesPtr->_midiMap),
        &stringID
    )){
        pgWarning(fileName);
        pgError("try to load multiple of same midi");
    }
    hashMapPutPtr(WideString, MidiSequence,
        &(resourcesPtr->_midiMap),
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
    /* create and configure loader */
    toRet._loader = blResourceLoaderMake();
    BLResourceType imageType = blResourceTypeMake(
        "bmp",
        loadImageIntoResources,
        &toRet //todo: this is a DANGLING POINTER
    )

    //todo: configure loader
    toRet._imageMap = hashMapMake(WideString, TFSprite,
        initImageCapacity,
        constructureWideStringHash,
        constructureWideStringEquals
    );
    toRet._midiMap = hashMapMake(WideString, TFSprite,
        initMidiCapacity,
        constructureWideStringHash,
        constructureWideStringEquals
    );

    return toRet;
}

/*
 * Nonrecursively loads all the files in the specified
 * directory into the given Resources object
 */
Resources resoucesLoadDirectory(
    Resources *resourcesPtr,
    char *dirName
){
    //todo
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
        &(resourcesPtr->_imageMap),
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
        &(resourcesPtr->_midiMap),
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
        &(resourcesPtr->_imageMap),
        tfSpriteFree
    );
    hashMapKeyApply(WideString, TFSprite,
        &(resourcesPtr->_imageMap),
        wideStringFree
    );
    hashMapFree(WideString, TFSprite,
        &(resourcesPtr->_imageMap)
    );

    /* free midi map */
    hashMapApply(WideString, MidiSequence,
        &(resourcesPtr->_midiMap),
        midiSequenceFree
    );
    hashMapKeyApply(WideString, MidiSequence,
        &(resourcesPtr->_midiMap),
        wideStringFree
    );
    hashMapFree(WideString, MidiSequence,
        &(resourcesPtr->_midiMap)
    );

    //todo free other resource types
}