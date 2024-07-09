#ifndef GAME_H
#define GAME_H

#include "Trifecta.h"
#include "MokyoMidi.h"

#include "Resources.h"
#include "Scenes.h"
#include "Settings.h"

/* Represents the internal state of the game */
typedef struct Game{
    /* Holds RTTI for components, owned by the game */
    WindComponents *componentsPtr;
    /* Scenes of the game, owned by the game */
    Scenes scenes;
    //todo: game messages

    /* a weak ptr to the game settings */
    Settings *settingsPtr;
    /* a weak ptr to the game resources */
    Resources *resourcesPtr;

    /* a weak ptr to the window */
    TFWindow *windowPtr;
    /* a weak ptr to the key table */
    TFKeyTable *keyTablePtr;
    /* a weak ptr to the midi hub */
    MidiHub *midiHubPtr;

    /* the user pointer passed to callbacks */
    void *userPtr;
    /* callback for when the game exits */
    void (*exitCallback)(void*);
    /* callback for when the game toggles fullscreen */
    void (*fullscreenCallback)(void*);
    /* callback for when the game saves settings */
    void (*writeSettingsCallback)(void*);
} Game;

/* Constructs and returns a new Game by value */
Game gameMake(
    Settings *settingsPtr,
    Resources *resourcesPtr,
    TFWindow *windowPtr,
    TFKeyTable *keyTablePtr,
    MidiHub *midiHubPtr,
    void *userPtr
);

/* 
 * Updates the internal state of the specified 
 * game once
 */
void gameUpdate(Game *gamePtr);

/* Renders the specified game to the screen */
void gameRender(Game *gamePtr);

/*
 * Sets the exit callback which will be passed the
 * user ptr of the specified game
 */
void gameSetExitCallback(
    Game *gamePtr,
    void(*exitCallback)(void*)
);

/*
 * Sets the fullscreen callback which will be passed
 * the user ptr of the specified game
 */
void gameSetFullscreenCallback(
    Game *gamePtr,
    void(*fullscreenCallback)(void*)
);

/*
 * Sets the write settings callback which will be
 * passed the user ptr of the specified game
 */
void gameSetWriteSettingsCallback(
    Game *gamePtr,
    void(*writeSettingsCallback)(void*)
);

/* 
 * Frees the memory associated with the 
 * specified game
 */
void gameFree(Game *gamePtr);

#endif