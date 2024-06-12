#include "Game.h"

/* Constructs and returns a new Game by value */
Game gameMake(
    Settings *settingsPtr,
    TFWindow *windowPtr,
    TFKeyTable *keyTablePtr,
    MidiHub *midiHubPtr
){
    Game toRet = {
        settingsPtr,
        windowPtr,
        keyTablePtr,
        midiHubPtr
    };
    return toRet;
}

static void updateSceneList(Game *gamePtr){
    //todo game updateSceneList
}

/* updates the key table by stepping the tick */
static void updateInput(Game *gamePtr){
    tfKeyTableStepTick(gamePtr->keyTablePtr);
}

static void updateMusic(Game *gamePtr){
    //todo game updateMusic
}

static void updateSettings(Game *gamePtr){
    //todo game updateSettings
}

/* Updates the internal state of the game once */
void gameUpdate(Game *gamePtr){
    //todo: game update

    updateSceneList(gamePtr);
    updateInput(gamePtr);
    updateMusic(gamePtr);
    updateSettings(gamePtr);
}

/* Renders the game to the screen */
void gameRender(Game *gamePtr){
    //todo: game render
}

/* 
 * Frees the memory associated with the 
 * specified game
 */
void gameFree(Game *gamePtr){
    //todo: game free
}