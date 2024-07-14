#include "Game.h"

#include "Components.h"
#include "Config.h"
#include "SceneUpdater.h"
#include "SceneRenderer.h"

/*
 * Constructs and returns a new GameMessages by
 * value
 */
GameMessages gameMessagesMake(){
    GameMessages toRet = {0};
    toRet.sceneEntryList = arrayListMake(SceneID, 5);
    toRet.sceneExitToID = -1;
    toRet.exitFlag = false;
    toRet.startMusicString = stringMakeAndReserve(20);
    toRet.stopMusicFlag = false;
    toRet.writeSettingsFlag = false;
    toRet.toggleSoundFlag = false;
    toRet.toggleFullscreenFlag = false;
    toRet.startDialogueString
        = stringMakeAndReserve(20);
    toRet.endDialogueFlag = false;

    return toRet;
}

/*
 * Frees the memory associated with the specified
 * GameMessages
 */
void gameMessagesFree(GameMessages *messagesPtr){
    arrayListFree(SceneID,
        &(messagesPtr->sceneEntryList)
    );
    stringFree(&(messagesPtr->startMusicString));
    stringFree(&(messagesPtr->startDialogueString));
    memset(messagesPtr, 0, sizeof(*messagesPtr));
}

/* Initializes the scenes of the specified Game */
static void gameInitScenes(Game *gamePtr){
    scenesPush(&(gamePtr->scenes), scene_main);
}

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
    toRet.scenes = scenesMake(toRet.componentsPtr);
    toRet.messages = gameMessagesMake();
    toRet.settingsPtr = settingsPtr;
    toRet.resourcesPtr = resourcesPtr;
    toRet.windowPtr = windowPtr;
    toRet.keyTablePtr = keyTablePtr;
    toRet.midiHubPtr = midiHubPtr;
    toRet.userPtr = userPtr;

    gameInitScenes(&toRet);
    
    return toRet;
}

/*
 * Handles pushing and popping scenes onto and off
 * the scene stack
 */
static void updateSceneList(Game *gamePtr){
    /* lock input to prevent new scenes from seeing */
    tfKeyTableLockAll(gamePtr->keyTablePtr);

    /* pop scenes off the stack first */
    if(gamePtr->messages.sceneExitToID != -1){
        scenesPopTo(
            &(gamePtr->scenes),
            gamePtr->messages.sceneExitToID
        );

        gamePtr->messages.sceneExitToID = -1;
    }

    /*
     * push new scenes onto the stack and update them
     * once
     */
    ArrayList *sceneEntryListPtr
        = &(gamePtr->messages.sceneEntryList);
    if(!arrayListIsEmpty(sceneEntryListPtr)){
        Scenes *scenesPtr = &(gamePtr->scenes);
        for(size_t i = 0;
            i < sceneEntryListPtr->size;
            ++i
        ){
            scenesPush(
                scenesPtr,
                arrayListGet(SceneID,
                    sceneEntryListPtr,
                    i
                )
            );
            updateScene(
                gamePtr,
                scenesGetTop(scenesPtr)
            );
        }

        arrayListClear(SceneID, sceneEntryListPtr);
    }
}

/* Updates the key table by stepping the tick */
static void updateInput(Game *gamePtr){
    tfKeyTableStepTick(gamePtr->keyTablePtr);
}

/* Handles starting and stopping music playback */
static void updateMusic(Game *gamePtr){
    /* stop music if flagged */
    if(gamePtr->messages.stopMusicFlag){
        midiHubStop(gamePtr->midiHubPtr);

        gamePtr->messages.stopMusicFlag = false;
    }

    /* start music if requested */
    String *startMusicStringPtr
        = &(gamePtr->messages.startMusicString);
    if(!stringIsEmpty(startMusicStringPtr)){
        MidiSequence *midiSequencePtr
            = resourcesGetMidi(
                gamePtr->resourcesPtr,
                startMusicStringPtr
            );
        if(!midiSequencePtr){
            pgWarning(
                startMusicStringPtr->_ptr
            );
            pgError(
                "failed to find midi by name; "
                SRC_LOCATION
            );
        }
        midiHubStart(
            gamePtr->midiHubPtr,
            midiSequencePtr
        );

        stringClear(startMusicStringPtr);
    }
}

/* Handles updating the settings for the game */
static void updateSettings(Game *gamePtr){
    /* toggle sound on and off */
    if(gamePtr->messages.toggleSoundFlag){
        gamePtr->settingsPtr->muted
            = !gamePtr->settingsPtr->muted;
        midiHubToggleMute(gamePtr->midiHubPtr);
        //todo: start playback of track 01 if unmuted

        gamePtr->messages.toggleSoundFlag = false;
    }

    /* toggle fullscreen on and off */
    if(gamePtr->messages.toggleFullscreenFlag){
        gamePtr->settingsPtr->fullscreen
            = !gamePtr->settingsPtr->fullscreen;
        tfWindowToggleFullscreen(gamePtr->windowPtr);

        gamePtr->messages.toggleFullscreenFlag = false;
    }

    /* write settings to file if requested */
    if(gamePtr->messages.writeSettingsFlag){
        writeSettingsToFile(
            gamePtr->settingsPtr,
            config_settingsFileName
        );

        gamePtr->messages.writeSettingsFlag = false;
    }
}

/*  Updates the internal state of the specified game */
void gameUpdate(Game *gamePtr){
    /* update scenes from top of the stack down */
    Scenes *scenesPtr = &(gamePtr->scenes);
    for(int i = scenesCurrentCount(scenesPtr) - 1;
        i >= 0;
        --i
    ){
        Scene *scenePtr = scenesGetScene(
            scenesPtr,
            i
        );
        updateScene(gamePtr, scenePtr);

        if(!scenePtr->updateTransparent){
            break;
        }
    }

    if(gamePtr->messages.exitFlag){
        gamePtr->exitCallback(gamePtr->userPtr);
        return;
    }

    updateSceneList(gamePtr);
    updateInput(gamePtr);
    updateMusic(gamePtr);
    updateSettings(gamePtr);
}

/* Renders the specified game to the screen */
void gameRender(Game *gamePtr){
    Scenes *scenesPtr = &(gamePtr->scenes);
    int indexOfHighestSceneToDraw
        = scenesCurrentCount(scenesPtr) - 1;
    assertTrue(
        indexOfHighestSceneToDraw >=0,
        "Error: no scenes to draw; " SRC_LOCATION
    );

    /* find bottom scene */
    int indexOfLowestSceneToDraw
        = indexOfHighestSceneToDraw;
    while(indexOfLowestSceneToDraw > 0
        && scenesGetScene(
            scenesPtr,
            indexOfLowestSceneToDraw
        )->renderTransparent
    ){
        --indexOfLowestSceneToDraw;
    }

    /* draw scenes from bottom up */
    for(int i = indexOfLowestSceneToDraw;
        i <= indexOfHighestSceneToDraw;
        ++i
    ){
        Scene *scenePtr = scenesGetScene(scenesPtr, i);
        renderScene(gamePtr, scenePtr);
        tfWindowClearDepth(gamePtr->windowPtr);
    }
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
 * Frees the memory associated with the 
 * specified game
 */
void gameFree(Game *gamePtr){
    gameMessagesFree(&(gamePtr->messages));
    scenesFree(&(gamePtr->scenes));
    windComponentsFree(gamePtr->componentsPtr);
    pgFree(gamePtr->componentsPtr);
    memset(gamePtr, 0, sizeof(*gamePtr));
}