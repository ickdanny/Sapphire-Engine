#ifndef GAME_H
#define GAME_H

#include "Trifecta.h"
#include "MokyoMidi.h"

#include "Resources.h"
#include "Scenes.h"
#include "Settings.h"
#include "GameState.h"
#include "PlayerData.h"

/* Stores messages for interscene communication */
typedef struct GameMessages{    
    /* 
     * Queue of SceneId to be pushed onto the stack;
     * handled by game in update scene list
     */
    ArrayList sceneEntryList;
    /*
     * SceneId to be popped to, -1 if no message;
     * handled by game in update scene list
     */
    SceneId sceneExitToId;
    /*
     * Flag to indicate exiting the game; handled by
     * game in update
     */
    bool exitFlag;

    /*
     * String name of music track to begin playback,
     * empty if no message; handled by game in update
     * music
     */
    String startMusicString;
    /*
     * Flag to indicate stopping music playback;
     * handled by game in update music
     */
    bool stopMusicFlag;

    /*
     * Flag to indicate writing settings to file;
     * handled by game in update settings
     */
    bool writeSettingsFlag;
    /*
     * Flag to indicate toggling sound on or off;
     * handled by game in update settings
     */
    bool toggleSoundFlag;
    /*
     * Flag to indicate toggling fullscreen on or off;
     * handled by game in update settings
     */
    bool toggleFullscreenFlag;

    /*
     * String name of dialogue to display, empty if
     * no message; set by script system, cleared by
     * dialogue system
     */
    String startDialogueString;

    /*
     * Flag to indicate the ending of dialogue; set by
     * dialogue system, cleared by script system
     */
    bool endDialogueFlag;

    /*
     * Stores metadata about the current game, set by
     * game builder system
     */
    GameState gameState;

    /*
     * set and cleared by continue system and script
     * system, also used and cleared by init system
     */
    struct{
        PlayerData data;
        bool isPresent;
    } playerData;
} GameMessages;

/*
 * Constructs and returns a new GameMessages by
 * value
 */
GameMessages gameMessagesMake();

/*
 * Frees the memory associated with the specified
 * GameMessages
 */
void gameMessagesFree(GameMessages *messagesPtr);

/* Represents the internal state of the game */
typedef struct Game{
    /* Holds RTTI for components, owned by the game */
    VecsComponentList *componentsPtr;
    /* Scenes of the game, owned by the game */
    Scenes scenes;
    /* messages are owned by the game */
    GameMessages messages;

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

/*  Updates the internal state of the specified game */
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
 * Frees the memory associated with the 
 * specified game
 */
void gameFree(Game *gamePtr);

#endif