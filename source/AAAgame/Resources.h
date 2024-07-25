#ifndef RESOURCES_H
#define RESOURCES_H

#include "BLoader.h"
#include "Trifecta.h"
#include "MokyoMidi.h"
#include "Unknown.h"
#include "Dialogue.h"

/* Stores file resources for scripts */
typedef struct ScriptResources{
    /* compiler for scripts */
    UNCompiler _compiler;
    /* map of UNObjectFunc* */
    HashMap _scriptMap;
    /* set of user functions detected while loading */
    UNUserFuncSet userFuncSet;
} ScriptResources;

/*
 * Constructs and returns a new (empty) ScriptResources
 * object by value
 */
ScriptResources scriptResourcesMake();

/*
 * Frees the memory associated with the specified
 * ScriptResources
 */
void scriptResourcesFree(
    ScriptResources *scriptResourcesPtr
);

/* Stores all file resources for the game */
typedef struct Resources{
    BLResourceLoader _loader;
    /* ptr to map of TFSprite */
    HashMap *_imageMapPtr;
    /* ptr to map of MidiSequence */
    HashMap *_midiMapPtr;
    /* ptr to map of Dialogue */
    HashMap *_dialogueMapPtr;
    /* ptr to script resources */
    ScriptResources *scriptResourcesPtr;
} Resources;

/*
 * Constructs and returns a new (empty) Resources
 * object by value
 */
Resources resourcesMake();

/*
 * Nonrecursively loads all the files in the specified
 * directory into the given Resources object
 */
void resourcesLoadDirectory(
    Resources *resourcesPtr,
    char *directoryName
);

/*
 * Returns a pointer to the image resource specified
 * by the given String or NULL if no such image
 * exists
 */
TFSprite *resourcesGetSprite(
    Resources *resourcesPtr,
    String *stringPtr
);

/*
 * Returns a pointer to the midi resource specified
 * by the given String or NULL if no such midi
 * exists
 */
MidiSequence *resourcesGetMidi(
    Resources *resourcesPtr,
    String *stringPtr
);

/*
 * Returns a pointer to the dialogue resource specified
 * by the given String or NULL if no such dialogue
 * exists
 */
Dialogue *resourcesGetDialogue(
    Resources *resourcesPtr,
    String *stringPtr
);

/*
 * Returns a pointer to the script resource specified
 * by the given String or NULL if no such script
 * exists
 */
UNObjectFunc *resourcesGetScript(
    Resources *resourcesPtr,
    String *stringPtr
);

/*
 * Frees the memory associated with the specified
 * Resources object
 */
void resourcesFree(Resources *resourcesPtr);

#endif