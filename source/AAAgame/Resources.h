#ifndef RESOURCES_H
#define RESOURCES_H

#include "BLoader.h"
#include "Trifecta.h"
#include "MokyoMidi.h"

/* Stores all file resources for the game */
typedef struct Resources{
    BLResourceLoader _loader;
    HashMap _imageMap;
    HashMap _midiMap;
    //todo other fields for other resources
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
Resources resoucesLoadDirectory(
    Resources *resourcesPtr,
    char *dirName
);

/*
 * Returns a pointer to the image resource specified
 * by the given WideString or NULL if no such image
 * exists
 */
TFSprite *resourcesGetSprite(
    Resources *resourcesPtr,
    WideString *wideStringPtr
);

/*
 * Returns a pointer to the midi resource specified
 * by the given WideString or NULL if no such midi
 * exists
 */
MidiSequence *resourcesGetMidi(
    Resources *resourcesPtr,
    WideString *wideStringPtr
);

//todo get other resources (e.g. dialogue)

/*
 * Frees the memory associated with the specified
 * Resources object
 */
void resourcesFree(Resources *resourcesPtr);

#endif