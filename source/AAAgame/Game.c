#include "Game.h"
#include "Components.h"

/* Constructs and returns a new Game by value */
Game gameMake(
    Settings *settingsPtr,
    Resources *resourcesPtr,
    TFWindow *windowPtr,
    TFKeyTable *keyTablePtr,
    MidiHub *midiHubPtr,
    void *userPtr
){
    Game toRet = {0};
    toRet.componentsPtr = componentsMake();
    toRet.scenes = scenesMake(toRet.componentsPtr),
    toRet.settingsPtr = settingsPtr;
    toRet.resourcesPtr = resourcesPtr;
    toRet.windowPtr = windowPtr;
    toRet.keyTablePtr = keyTablePtr;
    toRet.midiHubPtr = midiHubPtr;
    toRet.userPtr = userPtr;
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

    static int i;
    ++i;
    if(i % 120 == 0){
        gamePtr->fullscreenCallback(gamePtr->userPtr);
    }
}

/* Renders the game to the screen */
void gameRender(Game *gamePtr){
    //todo: game render
}

/*
 * Sets the exit callback which will be passed the
 * user ptr of the specified game
 */
void gameSetExitCallback(
    Game *gamePtr,
    void(*exitCallback)(void*)
){
    gamePtr->exitCallback = exitCallback;
}

/*
 * Sets the fullscreen callback which will be passed
 * the user ptr of the specified game
 */
void gameSetFullscreenCallback(
    Game *gamePtr,
    void(*fullscreenCallback)(void*)
){
    gamePtr->fullscreenCallback = fullscreenCallback;
}

/*
 * Sets the write settings callback which will be
 * passed the user ptr of the specified game
 */
void gameSetWriteSettingsCallback(
    Game *gamePtr,
    void(*writeSettingsCallback)(void*)
){
    gamePtr->writeSettingsCallback
        = writeSettingsCallback;
}

/* 
 * Frees the memory associated with the 
 * specified game
 */
void gameFree(Game *gamePtr){
    //todo: free game stuff
    windComponentsFree(gamePtr->componentsPtr);
    pgFree(gamePtr->componentsPtr);
    scenesFree(&(gamePtr->scenes));
    memset(gamePtr, 0, sizeof(*gamePtr));
}