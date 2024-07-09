#ifndef SCENE_H
#define SCENE_H

#include "WindECS.h"

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
    //todo scene messages struct
} SceneMessages;

/*
 * Constructs and returns a new SceneMessages by
 * value
 */
SceneMessages sceneMessagesMake();

/* Clears the given SceneMessages */
void sceneMessagesClear(
    SceneMessages *sceneMessagePtr
);

/*
 * Frees the memory associated with the specified
 * SceneMessgaes
 */
void sceneMessagesFree(
    SceneMessages *sceneMessagesPtr
);

/* A scene encapsulates an ECS world */
typedef struct Scene{
    SceneID id;
    bool _refresh;
    bool updateTransparent;
    bool renderTransparent;

    WindWorld ecsWorld;
    SceneMessages sceneMessages;
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
 * on the scene stack in the specified Scenes pointer
 * (where higher is closer to the top, zero indexed);
 * error if invalid index
 */
Scene *scenesGetScene(Scenes *scenesPtr, int i);

/*
 * Frees the memory associated with the given Scenes
 * object
 */
void scenesFree(Scenes *scenesPtr);

#endif