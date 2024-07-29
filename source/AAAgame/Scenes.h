#ifndef SCENE_H
#define SCENE_H

#include "WindECS.h"
#include "ZMath.h"

#include "Config.h"
#include "MenuCommand.h"
#include "GameBuilderCommand.h"
#include "PlayerData.h"
#include "Dialogue.h"

/* used to identify the different kinds of scenes */
typedef enum SceneID{
    scene_main,
    scene_difficulty,
    scene_stage,
    scene_music,
    scene_options,
    scene_loading,
    scene_game,
    scene_dialogue,
    scene_pause,
    scene_continue,
    scene_credits,
    scene_numScenes,
} SceneID;

/* Used for the collision messages */
typedef struct Collision{
    WindEntity sourceHandle;
    WindEntity targetHandle;
} Collision;

/* Stores messages for intersystem communication */
typedef struct SceneMessages{
    /*
     * Flag to indicate that the scene has been
     * initialized, set by init system
     */
    bool initFlag;

    /*
     * List of MenuNavigationCommand handled by input
     * parser system
     */
    ArrayList menuNavigationCommands;

    /*
     * Menu command to be executed when the back button
     * is pressed, set by init system
     */
    MenuCommand backMenuCommand;

    /*
     * Scene ID associated with the back menu command,
     * may not always be present; set by init system
     */
    SceneID backSceneID;

    /*
     * Handle to currently selected element, set by
     * init system and used by menu navigation system
     */
    WindEntity currentElement;

    /*
     * Info about changing the selected element handled
     * by menu navigation system
     */
    struct{
        bool newElementSelected;
        WindEntity prevElement;
    } elementChanges;

    /*
     * Indicates the next game builder command to be
     * handled; gb_none means nothing new; set by
     * menu navigation system, cleared by game builder
     * system
     */
    GameBuilderCommand gameBuilderCommand;

    /*
     * Timer used by systems; -1 if not currently in
     * use; loading screen system, continue system,
     * credits system
     */
    int timer1;

    /*
     * Flag to signal a dialogue read handled by input
     * parser system
     */
    bool readDialogueFlag;

    /* data for the dialogue system */
    struct{
        /* ptr to the current dialogue */
        Dialogue *dialoguePtr;
        /* index of the next dialogue instruction */
        int nextDialogueIndex;

        WindEntity leftImageHandle;
        WindEntity rightImageHandle;
        WindEntity textHandle;
    } dialogueData;

    /* A PRNG set by init system if needed */
    ZMT prng;

    /*
     * List of GameCommand, handled by input parser
     * system
     */
    ArrayList gameCommands;

    /*
     * Set whenever player enters a new state, handled
     * by player state system
     */
    struct{
        WindEntity playerHandle;
        PlayerState state;
    } playerStateEntry;

    /*
     * List of WindEntity for player hits, handled
     * by collision handler system
     */
    ArrayList playerHits;

    /*
     * List of dead WindEntity, set by collision
     * handler system and player death detector system,
     * cleared by message cleanup system
     */
    ArrayList deaths;

    /*
     * Flag to signal boss death (i.e. phase change),
     * set by death handler system, cleared by
     * boss script
     */
    bool bossDeathFlag;

    /*
     * Flag to signal clearing the screen of bullets,
     * handled by clear system
     */
    bool clearFlag;

    /*
     * Flag to signal pausing the game, set by
     * continue and pause system
     */
    bool pauseFlag;

    /*
     * Flag to signal the player successfully
     * completing a stage, handled by the stage script
     */
    bool winFlag;

    /*
     * Signal to add lives to the player, handled by
     * player life add system
     */
    int livesToAdd;

    /*
     * Signal to add bombs to the player, handled by
     * player bomb add system
     */
    int bombsToAdd;

    /*
     * Used by the overlay system to hold handles to
     * ui elements
     */
    struct{
        WindEntity lifeHandles[config_maxLives];
        /*
         * index of last visible life element; -1 if
         * no elements currently visible
         */
        int lifeIndex;
        WindEntity bombHandles[config_maxLives];

        /*
         * index of last visible bomb element; -1 if
         * no elements currently visible
         */
        int bombIndex;
        WindEntity powerHandle;
    } overlayData;

    /* 
     * List of player Collision handled by collision
     * detector system
     */
    ArrayList playerCollisionList;

    /* 
     * List of enemy Collision handled by collision
     * detector system
     */
    ArrayList enemyCollisionList;

    /* 
     * List of bullet Collision handled by collision
     * detector system
     */
    ArrayList bulletCollisionList;

    /* 
     * List of pickup Collision handled by collision
     * detector system
     */
    ArrayList pickupCollisionList;

    /*
     * Flag for scripts to use for cross-entity
     * communication
     */
    unsigned int userFlag1;
} SceneMessages;

/*
 * Constructs and returns a new SceneMessages by
 * value
 */
SceneMessages sceneMessagesMake();

/* Clears the given SceneMessages */
void sceneMessagesClear(
    SceneMessages *messagesPtr
);

/*
 * Frees the memory associated with the specified
 * SceneMessgaes
 */
void sceneMessagesFree(
    SceneMessages *messagesPtr
);

/* A scene encapsulates an ECS world */
typedef struct Scene{
    SceneID id;
    bool _refresh;
    bool updateTransparent;
    bool renderTransparent;

    WindWorld ecsWorld;
    SceneMessages messages;
} Scene;

/* Constructs and returns a new Scene by value */
Scene sceneMake(
    SceneID id,
    size_t entityCapacity,
    WindComponents *componentsPtr,
    bool refresh,
    bool updateTransparent,
    bool renderTransparent
);

/*
 * Refreshes a scene if it was set to need refreshing,
 * does nothing otherwise
 */
void sceneRefresh(Scene *scenePtr);

/* Frees the memory associated with the given Scene */
void sceneFree(Scene *scenePtr);

/*
 * A scenes object stores all the scenes for the game
 * and also keeps track of which scenes are currently
 * active and in what order
 */
typedef struct Scenes{
    /*
     * an array of Scene indexed by SceneID initialized
     * during construction
     */
    Array _sceneStorage;
    /*
     * used as a stack of scenes, storing SceneID
     */
    ArrayList _sceneStack;
} Scenes;

/*
 * Constructs and returns a new Scenes object by value
 */
Scenes scenesMake(WindComponents *componentsPtr);

/*
 * Pushes the requested scene onto the top of the scene
 * stack in the given Scenes object
 */
void scenesPush(Scenes *scenesPtr, SceneID sceneID);

/*
 * Pops the top scene off the scene stack in the given
 * Scenes object
 */
void scenesPop(Scenes *scenesPtr);

/*
 * Pops scenes off the scene stack in the given Scenes
 * object until the requested scene is reached; error
 * if stack underflow
 */
void scenesPopTo(
    Scenes *scenesPtr,
    SceneID sceneToFind
);

/*
 * Returns the current number of scenes on the stack in
 * the specified Scenes object
 */
int scenesCurrentCount(Scenes *scenesPtr);

/*
 * Returns a pointer to the scene at the ith position
 * on the scene stack in the specified Scenes object
 * (where higher is closer to the top, zero indexed);
 * error if invalid index
 */
Scene *scenesGetScene(Scenes *scenesPtr, int i);

/*
 * Returns a pointer to the scene currently at the top
 * of the scene stack in the specified Scenes object;
 * error if the stack is empty
 */
#define scenesGetTop(SCENESPTR) \
    scenesGetScene( \
        (SCENESPTR), \
        scenesCurrentCount((SCENESPTR)) - 1 \
    )

/*
 * Frees the memory associated with the given Scenes
 * object
 */
void scenesFree(Scenes *scenesPtr);

#endif