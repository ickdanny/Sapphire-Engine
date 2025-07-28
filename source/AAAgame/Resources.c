#include "Resources.h"

#define initImageCapacity 200
#define initMidiCapacity 20
#define initDialogueCapacity 20
#define initScriptCapacity 200

/*
 * Constructs and returns a new (empty) ScriptResources
 * object by value
 */
ScriptResources scriptResourcesMake(){
    ScriptResources toRet = {0};
    toRet._compiler = necroCompilerMake();
    toRet._scriptMap = hashMapMake(
        String, NecroObjectFunc*,
        initScriptCapacity,
        constructureStringHash,
        constructureStringEquals
    );
    return toRet;
}

/*
 * For freeing NecroObjectFunc* with hashmap since hashmap
 * apply provides double pointers
 */
static void freeScript(NecroObjectFunc** doublePtr){
    necroObjectFree((NecroObject*)(*doublePtr));
}

/*
 * Frees the memory associated with the specified
 * ScriptResources
 */
void scriptResourcesFree(
    ScriptResources *scriptResourcesPtr
){
    necroCompilerFree(&(scriptResourcesPtr->_compiler));

    /* free script map */
    hashMapApply(String, NecroObjectFunc*,
        &(scriptResourcesPtr->_scriptMap),
        freeScript
    );
    hashMapKeyApply(String, NecroObjectFunc*,
        &(scriptResourcesPtr->_scriptMap),
        stringFree
    );
    hashMapFree(String, NecroObjectFunc*,
        &(scriptResourcesPtr->_scriptMap)
    );

    memset(
        scriptResourcesPtr,
        0,
        sizeof(*scriptResourcesPtr)
    );
}

/*
 * Isolates the file name from the given file path
 * and returns it as a String by value
 */
String isolateFileName(const char *fileName){
    String toRet = stringMakeEmpty();

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
    
    /* convert chars 1 by 1 to the string */
    for(int i = startIndexInclusive;
        i < stopIndexExclusive;
        ++i
    ){
        stringPushBack(
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
    String stringId = isolateFileName(fileName);
    if(hashMapHasKeyPtr(String, TFSprite,
        imageMapPtr,
        &stringId
    )){
        pgWarning(fileName);
        pgError("try to load multiple of same image");
    }
    hashMapPutPtr(String, TFSprite,
        imageMapPtr,
        &stringId,
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
    String stringId = isolateFileName(fileName);
    if(hashMapHasKeyPtr(String, MidiSequence,
        midiMapPtr,
        &stringId
    )){
        pgWarning(fileName);
        pgError("try to load multiple of same midi");
    }
    hashMapPutPtr(String, MidiSequence,
        midiMapPtr,
        &stringId,
        &midi
    );
}

/* Dialogue loading callback */
static void loadDialogueIntoResources(
    const char *fileName,
    void *dialogueMapVoidPtr
){
    HashMap *dialogueMapPtr = dialogueMapVoidPtr;

    Dialogue dialogue = parseDialogueFile(fileName);
    String stringId = isolateFileName(fileName);
    if(hashMapHasKeyPtr(String, Dialogue,
        dialogueMapPtr,
        &stringId
    )){
        pgWarning(fileName);
        pgError(
            "try to load multiple of same dialogue"
        );
    }
    hashMapPutPtr(String, Dialogue,
        dialogueMapPtr,
        &stringId,
        &dialogue
    );
}

/* Script loading callback (.un) */
static void loadScriptIntoResources(
    const char *fileName,
    void *scriptResourcesVoidPtr
){
    ScriptResources *scriptResourcesPtr
        = scriptResourcesVoidPtr;
    HashMap *scriptMapPtr
        = &(scriptResourcesPtr->_scriptMap);
    NecroCompiler *compilerPtr
        = &(scriptResourcesPtr->_compiler);

    NecroObjectFunc *scriptPtr = necroCompilerCompileScript(
        compilerPtr,
        fileName
    );

    String stringId = isolateFileName(fileName);
    if(hashMapHasKeyPtr(String, NecroObjectFunc*,
        scriptMapPtr,
        &stringId
    )){
        pgWarning(fileName);
        pgError("try to load multiple of same script");
    }
    hashMapPutPtr(String, NecroObjectFunc*,
        scriptMapPtr,
        &stringId,
        &scriptPtr
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
    toRet._dialogueMapPtr = pgAlloc(
        1,
        sizeof(*(toRet._dialogueMapPtr))
    );
    toRet.scriptResourcesPtr = pgAlloc(
        1,
        sizeof(*(toRet.scriptResourcesPtr))
    );
    (*toRet._imageMapPtr) = hashMapMake(
        String, TFSprite,
        initImageCapacity,
        constructureStringHash,
        constructureStringEquals
    );
    (*toRet._midiMapPtr) = hashMapMake(
        String, MidiSequence,
        initMidiCapacity,
        constructureStringHash,
        constructureStringEquals
    );
    (*toRet._dialogueMapPtr) = hashMapMake(
        String, Dialogue,
        initDialogueCapacity,
        constructureStringHash,
        constructureStringEquals
    );
    (*toRet.scriptResourcesPtr)
        = scriptResourcesMake();

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
    BLResourceType dialogueType = blResourceTypeMake(
        "dlg",
        loadDialogueIntoResources,
        toRet._dialogueMapPtr
    );
    BLResourceType scriptType = blResourceTypeMake(
        "nec",
        loadScriptIntoResources,
        toRet.scriptResourcesPtr
    );
    blResourceLoaderRegisterType(
        &(toRet._loader),
        &imageType
    );
    blResourceLoaderRegisterType(
        &(toRet._loader),
        &midiType
    );
     blResourceLoaderRegisterType(
        &(toRet._loader),
        &dialogueType
    );
    blResourceLoaderRegisterType(
        &(toRet._loader),
        &scriptType
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
 * by the given String or NULL if no such image
 * exists
 */
TFSprite *resourcesGetSprite(
    Resources *resourcesPtr,
    String *stringPtr
){
    return hashMapGetPtr(String, TFSprite,
        resourcesPtr->_imageMapPtr,
        stringPtr
    );
}

/*
 * Returns a pointer to the midi resource specified
 * by the given String or NULL if no such midi
 * exists
 */
MidiSequence *resourcesGetMidi(
    Resources *resourcesPtr,
    String *stringPtr
){
    return hashMapGetPtr(String, MidiSequence,
        resourcesPtr->_midiMapPtr,
        stringPtr
    );
}

/*
 * Returns a pointer to the dialogue resource specified
 * by the given String or NULL if no such dialogue
 * exists
 */
Dialogue *resourcesGetDialogue(
    Resources *resourcesPtr,
    String *stringPtr
){
    return hashMapGetPtr(String, Dialogue,
        resourcesPtr->_dialogueMapPtr,
        stringPtr
    );
}

/*
 * Returns a pointer to the script resource specified
 * by the given String or NULL if no such script
 * exists
 */
NecroObjectFunc *resourcesGetScript(
    Resources *resourcesPtr,
    String *stringPtr
){
    NecroObjectFunc **returnedPtr = hashMapGetPtr(
        String,
        NecroObjectFunc*,
        &(resourcesPtr->scriptResourcesPtr
            ->_scriptMap),
        stringPtr
    );
    if(!returnedPtr){
        return NULL;
    }
    return *returnedPtr;
}

/*
 * Frees the memory associated with the specified
 * Resources object
 */
void resourcesFree(Resources *resourcesPtr){
    /* free loader*/
    blResourceLoaderFree(&(resourcesPtr->_loader));

    /* free image map */
    hashMapApply(String, TFSprite,
        resourcesPtr->_imageMapPtr,
        tfSpriteFree
    );
    hashMapKeyApply(String, TFSprite,
        resourcesPtr->_imageMapPtr,
        stringFree
    );
    hashMapFree(String, TFSprite,
        resourcesPtr->_imageMapPtr
    );
    pgFree(resourcesPtr->_imageMapPtr);

    /* free midi map */
    hashMapApply(String, MidiSequence,
        resourcesPtr->_midiMapPtr,
        midiSequenceFree
    );
    hashMapKeyApply(String, MidiSequence,
        resourcesPtr->_midiMapPtr,
        stringFree
    );
    hashMapFree(String, MidiSequence,
        resourcesPtr->_midiMapPtr
    );
    pgFree(resourcesPtr->_midiMapPtr);

    /* free dialogue map */
    hashMapApply(String, Dialogue,
        resourcesPtr->_dialogueMapPtr,
        dialogueFree
    );
    hashMapKeyApply(String, Dialogue,
        resourcesPtr->_dialogueMapPtr,
        stringFree
    );
    hashMapFree(String, Dialogue,
        resourcesPtr->_dialogueMapPtr
    );
    pgFree(resourcesPtr->_dialogueMapPtr);

    /* free script resources */
    scriptResourcesFree(
        resourcesPtr->scriptResourcesPtr
    );
    pgFree(resourcesPtr->scriptResourcesPtr);
}