#ifndef SCENE_H
#define SCENE_H

#include "WindECS.h"

#include "MenuCommand.h"
#include "GameBuilderCommand.h"

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
    scene_continues,
    scene_credits,
    scene_numScenes,
} SceneID;

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
     * use; loading screen system
     */
    int timer1;

    /*
     * Flag to signal a dialogue read handled by input
     * parser system
     */
    bool readDialogueFlag;

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

    //todo scene messages struct
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