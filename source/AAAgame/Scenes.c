#include "Scenes.h"

#include "MenuNavigationCommand.h"
#include "GameCommand.h"

/*
 * Constructs and returns a new SceneMessages by
 * value
 */
SceneMessages sceneMessagesMake(){
    SceneMessages toRet = {0};
    toRet.menuNavigationCommands = arrayListMake(
        MenuNavigationCommand,
        10
    );
    toRet.gameCommands = arrayListMake(
        GameCommand,
        10
    );
    toRet.playerHits = arrayListMake(VecsEntity, 10);
    toRet.deaths = arrayListMake(VecsEntity, 100);
    toRet.timer1 = -1;
    toRet.playerCollisionList = arrayListMake(
        Collision,
        10
    );
    toRet.enemyCollisionList = arrayListMake(
        Collision,
        10
    );
    toRet.bulletCollisionList = arrayListMake(
        Collision,
        10
    );
    toRet.pickupCollisionList = arrayListMake(
        Collision,
        10
    );
    return toRet;
}

/* Clears the given SceneMessages */
void sceneMessagesClear(
    SceneMessages *messagesPtr
){
    messagesPtr->initFlag = false;
    arrayListClear(MenuNavigationCommand,
        &(messagesPtr->menuNavigationCommands)
    );
    messagesPtr->backMenuCommand = menu_none;
    messagesPtr->backSceneId = scene_numScenes;
    messagesPtr->currentElement = (VecsEntity){0};
    messagesPtr->elementChanges.newElementSelected
        = false;
    messagesPtr->elementChanges.prevElement
        = (VecsEntity){0};
    messagesPtr->gameBuilderCommand = gb_none;
    messagesPtr->timer1 = -1;
    messagesPtr->readDialogueFlag = false;
    memset(
        &(messagesPtr->dialogueData),
        0,
        sizeof(messagesPtr->dialogueData)
    );
    zmtFree(&(messagesPtr->prng));
    arrayListClear(GameCommand,
        &(messagesPtr->gameCommands)
    );
    messagesPtr->playerStateEntry.playerHandle
        = (VecsEntity){0};
    messagesPtr->playerStateEntry.state = player_none;
    arrayListClear(VecsEntity,
        &(messagesPtr->playerHits)
    );
    arrayListClear(VecsEntity,
        &(messagesPtr->deaths)
    );
    messagesPtr->bossDeathFlag = false;
    messagesPtr->clearFlag = false;
    messagesPtr->pauseFlag = false;
    messagesPtr->winFlag = false;
    messagesPtr->livesToAdd = 0;
    messagesPtr->bombsToAdd = 0;
    memset(
        &(messagesPtr->overlayData),
        0,
        sizeof(messagesPtr->overlayData)
    );
    arrayListClear(Collision,
        &(messagesPtr->playerCollisionList)
    );
    arrayListClear(Collision,
        &(messagesPtr->enemyCollisionList)
    );
    arrayListClear(Collision,
        &(messagesPtr->bulletCollisionList)
    );
    arrayListClear(Collision,
        &(messagesPtr->pickupCollisionList)
    );
    messagesPtr->userFlag1 = 0;
}

/*
 * Frees the memory associated with the specified
 * SceneMessgaes
 */
void sceneMessagesFree(
    SceneMessages *messagesPtr
){
    arrayListFree(MenuNavigationCommand,
        &(messagesPtr->menuNavigationCommands)
    );
    zmtFree(&(messagesPtr->prng));
    arrayListFree(GameCommand,
        &(messagesPtr->gameCommands)
    );
    arrayListFree(VecsEntity,
        &(messagesPtr->playerHits)
    );
    arrayListFree(VecsEntity,
        &(messagesPtr->deaths)
    );
    arrayListFree(Collision,
        &(messagesPtr->playerCollisionList)
    );
    arrayListFree(Collision,
        &(messagesPtr->enemyCollisionList)
    );
    arrayListFree(Collision,
        &(messagesPtr->bulletCollisionList)
    );
    arrayListFree(Collision,
        &(messagesPtr->pickupCollisionList)
    );
    memset(
        messagesPtr,
        0,
        sizeof(*messagesPtr)
    );
}

/* Constructs and returns a new Scene by value */
Scene sceneMake(
    SceneId id,
    size_t entityCapacity,
    VecsComponentList *componentsPtr,
    bool refresh,
    bool updateTransparent,
    bool renderTransparent
){
    Scene toRet = {
        id,
        refresh,
        updateTransparent,
        renderTransparent
    };
    toRet.ecsWorld = vecsWorldMake(
        entityCapacity,
        componentsPtr
    );
    toRet.messages = sceneMessagesMake();
    return toRet;
}

/*
 * Refreshes a scene if it was set to need refreshing,
 * does nothing otherwise
 */
void sceneRefresh(Scene *scenePtr){
    if(scenePtr->_refresh){
        /* clear ecs world */
        vecsWorldClear(&(scenePtr->ecsWorld));

        /* clear messages */
        sceneMessagesClear(&(scenePtr->messages));
    }
}

/* Frees the memory associated with the given Scene */
void sceneFree(Scene *scenePtr){
    vecsWorldFree(&(scenePtr->ecsWorld));
    sceneMessagesFree(&(scenePtr->messages));
    memset(scenePtr, 0, sizeof(*scenePtr));
}

/*
 * helper function to construct a scene for the
 * given Scenes object
 */
void scenesDefineScene(
    Scenes *scenesPtr,
    SceneId id,
    size_t entityCapacity,
    VecsComponentList *componentsPtr,
    bool refresh,
    bool updateTransparent,
    bool renderTransparent
){
    arraySet(Scene,
        &(scenesPtr->_sceneStorage),
        id,
        sceneMake(
            id,
            entityCapacity,
            componentsPtr,
            refresh,
            updateTransparent,
            renderTransparent
        )
    );
}

/*
 * Constructs and returns a new Scenes object by value
 */
Scenes scenesMake(VecsComponentList *componentsPtr){
    Scenes toRet = {0};
    toRet._sceneStorage = arrayMake(Scene,
        scene_numScenes
    );
    toRet._sceneStack = arrayListMake(SceneId,
        scene_numScenes
    );

    #define defineScene( \
        Id, \
        ENTITYCAPACITY, \
        REFRESH, \
        TRANSPARENCY \
    ) \
        arraySet(Scene, \
            &(toRet._sceneStorage), \
            Id, \
            sceneMake( \
                Id, \
                ENTITYCAPACITY, \
                componentsPtr, \
                REFRESH, \
                TRANSPARENCY \
            ) \
        )
    #define notTransparent false, false
    #define renderTransparent false, true
    #define fullyTransparent true, true

    defineScene(scene_main,
        20,     false,  notTransparent
    );
    defineScene(scene_difficulty,
        20,     true,   notTransparent /* refresh */
    );
    defineScene(scene_stage,
        20,     false,  notTransparent
    );
    defineScene(scene_music,
        20,     true,   renderTransparent
    );
    defineScene(scene_options,
        20,     true,   renderTransparent
    );
    defineScene(scene_loading,
        5,      false,  renderTransparent
    );
    defineScene(scene_game,
        config_gameMaxEntities,
        true,
        notTransparent
    );
    defineScene(scene_dialogue,
        5,      true,   fullyTransparent
    );
    defineScene(scene_pause,
        5,      true,   renderTransparent
    );
    defineScene(scene_continue,
        10,     true,   renderTransparent
    );
    defineScene(scene_credits,
        20,     true,   notTransparent
    );

    return toRet;

    #undef defineScene
    #undef noTransparency
    #undef renderTransparent
    #undef fullyTransparent
}

/*
 * Pushes the requested scene onto the top of the scene
 * stack in the given Scenes object
 */
void scenesPush(Scenes *scenesPtr, SceneId sceneId){
    arrayListPushBack(SceneId,
        &(scenesPtr->_sceneStack),
        sceneId
    );
    Scene *scenePtr = arrayGetPtr(Scene,
        &(scenesPtr->_sceneStorage),
        sceneId
    );
    if(scenePtr->_refresh){
        sceneRefresh(scenePtr);
    }
}

/*
 * Pops the top scene off the scene stack in the given
 * Scenes object
 */
void scenesPop(Scenes *scenesPtr){
    arrayListPopBack(SceneId,
        &(scenesPtr->_sceneStack)
    );
}

/*
 * Pops scenes off the scene stack in the given Scenes
 * object until the requested scene is reached; error
 * if stack underflow
 */
void scenesPopTo(
    Scenes *scenesPtr,
    SceneId sceneToFind
){
    SceneId topSceneId = arrayListBack(SceneId,
        &(scenesPtr->_sceneStack)
    );
    while(topSceneId != sceneToFind){
        scenesPop(scenesPtr);
        topSceneId = arrayListBack(SceneId,
            &(scenesPtr->_sceneStack)
        );
    }
}

/*
 * Returns the current number of scenes on the stack in
 * the specified Scenes object
 */
int scenesCurrentCount(Scenes *scenesPtr){
    return scenesPtr->_sceneStack.size;
}

/*
 * Returns a pointer to the scene at the ith position
 * on the scene stack in the specified Scenes pointer
 * (where higher is closer to the top, zero indexed);
 * error if invalid index
 */
Scene *scenesGetScene(Scenes *scenesPtr, int i){
    if(i < 0 || i >= scenesPtr->_sceneStorage.size){
        pgError(
            "requested scene not on stack; "
            SRC_LOCATION
        );
    }
    SceneId sceneId = arrayListGet(SceneId,
        &(scenesPtr->_sceneStack),
        i
    );
    return arrayGetPtr(Scene,
        &(scenesPtr->_sceneStorage),
        sceneId
    );
}

/*
 * Frees the memory associated with the given Scenes
 * object
 */
void scenesFree(Scenes *scenesPtr){
    arrayApply(Scene,
        &(scenesPtr->_sceneStorage),
        sceneFree
    );
    arrayFree(Scene, &(scenesPtr->_sceneStorage));
    arrayListFree(SceneId, &(scenesPtr->_sceneStack));
    memset(scenesPtr, 0, sizeof(*scenesPtr));
}