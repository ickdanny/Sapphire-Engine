#ifndef GAME_H
#define GAME_H

#include "Trifecta.h"
#include "MokyoMidi.h"

#include "Settings.h"

/* Represents the internal state of the game */
typedef struct Game{
    /* a weak ptr to the game settings */
    Settings *settingsPtr;
    //todo: resources

    /* a weak ptr to the window */
    TFWindow *windowPtr;
    /* a weak ptr to the key table */
    TFKeyTable *keyTablePtr;
    /* a weak ptr to the midi hub */
    MidiHub *midiHubPtr;
    //todo: struct game
} Game;

/* Constructs and returns a new Game by value */
Game gameMake(
    Settings *settingsPtr,
    TFWindow *windowPtr,
    TFKeyTable *keyTablePtr,
    MidiHub *midiHubPtr
);

/* 
 * Updates the internal state of the specified 
 * game once
 */
void gameUpdate(Game *gamePtr);

/* Renders the specified game to the screen */
void gameRender(Game *gamePtr);

/* 
 * Frees the memory associated with the 
 * specified game
 */
void gameFree(Game *gamePtr);

#endif