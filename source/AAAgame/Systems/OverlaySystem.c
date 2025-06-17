#include "OverlaySystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static String removeUIScriptId;
static String lifeSpawnSpriteId;
static String bombSpawnSpriteId;
static String powerId;
static String powerMaxId;
static bool initialized = false;

/* destroys the overlay system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        stringFree(&removeUIScriptId);
        stringFree(&lifeSpawnSpriteId);
        stringFree(&bombSpawnSpriteId);
        stringFree(&powerId);
        stringFree(&powerMaxId);
        initialized = false;
    }
}

/* inits the overlay system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PlayerDataId);

        removeUIScriptId = stringMakeC(
            "remove_explode_ui"
        );
        lifeSpawnSpriteId = stringMakeC(
            "overlay_spawn_life1"
        );
        bombSpawnSpriteId = stringMakeC(
            "overlay_spawn_bomb1"
        );
        powerId = stringMakeC("overlay_power");
        powerMaxId = stringMakeC("overlay_power_max");

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* retrieves the player handle */
static VecsEntity getPlayerHandle(Scene *scenePtr){
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    /* get first player */
    if(windQueryItrHasEntity(&itr)){
        return vecsWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentId(&itr)
        );
    }
    /* error if cannot find player */
    else{
        pgError(
            "failed to find player; "
            SRC_LOCATION
        );
        return (VecsEntity){0};
    }
}

/*
 * Makes the specified life ui element invisible and
 * spawns an explode at its location
 */
static void removeLife(
    Game *gamePtr,
    Scene *scenePtr,
    VecsEntity handle
){
    /* make the entity invisible */
    windWorldHandleRemoveComponent(VisibleMarker,
        &(scenePtr->ecsWorld),
        handle
    );

    /* spawn an explode at entity location */
    Position *positionPtr = vecsWorldEntityGetPtr(
        Position,
        &(scenePtr->ecsWorld),
        handle
    );
    Point2D pos = positionPtr->currentPos;

    declareList(componentList, 10);
    addVisible(&componentList);
    addPosition(&componentList, pos);
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        "overlay_explode_life1",
        config_foregroundDepth + 10,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "overlay_explode_life1"
    );
    animationAddFrame(
        &animation,
        "overlay_explode_life2"
    );
    animationAddFrame(
        &animation,
        "overlay_explode_life3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    addAnimations(&componentList, &animations);
    Scripts scripts = {0};
    scripts.vm1 = vmPoolRequest();
    unVirtualMachineLoad(
        scripts.vm1,
        resourcesGetScript(
            gamePtr->resourcesPtr,
            &removeUIScriptId
        )
    );
    addScripts(&componentList, scripts);
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        NULL
    );
}

/*
 * Makes the specified bomb ui element invisible and
 * spawns an explode at its location
 */
static void removeBomb(
    Game *gamePtr,
    Scene *scenePtr,
    VecsEntity handle
){
    /* make the entity invisible */
    windWorldHandleRemoveComponent(VisibleMarker,
        &(scenePtr->ecsWorld),
        handle
    );

    /* spawn an explode at entity location */
    Position *positionPtr = vecsWorldEntityGetPtr(
        Position,
        &(scenePtr->ecsWorld),
        handle
    );
    Point2D pos = positionPtr->currentPos;

    declareList(componentList, 10);
    addVisible(&componentList);
    addPosition(&componentList, pos);
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        "overlay_explode_bomb1",
        config_foregroundDepth + 10,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "overlay_explode_bomb1"
    );
    animationAddFrame(
        &animation,
        "overlay_explode_bomb2"
    );
    animationAddFrame(
        &animation,
        "overlay_explode_bomb3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    addAnimations(&componentList, &animations);
    Scripts scripts = {0};
    scripts.vm1 = vmPoolRequest();
    unVirtualMachineLoad(
        scripts.vm1,
        resourcesGetScript(
            gamePtr->resourcesPtr,
            &removeUIScriptId
        )
    );
    addScripts(&componentList, scripts);
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        NULL
    );
}

/*
 * Makes the specified life ui element visible and
 * plays the spawn animation
 */
static void addLife(
    Game *gamePtr,
    Scene *scenePtr,
    VecsEntity handle
){
    /* make the entity visible */
    windWorldHandleAddComponent(VisibleMarker,
        &(scenePtr->ecsWorld),
        handle,
        NULL
    );
    /* change the current sprite to spawn 1 */
    SpriteInstruction *spriteInstrPtr
        = vecsWorldEntityGetPtr(SpriteInstruction,
            &(scenePtr->ecsWorld),
            handle
        );
    spriteInstrPtr->spritePtr = resourcesGetSprite(
        gamePtr->resourcesPtr,
        &lifeSpawnSpriteId
    );
    /* remove the old animation component if needed */
    windWorldHandleRemoveComponent(Animations,
        &(scenePtr->ecsWorld),
        handle
    );
    /* add new animation component */
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "overlay_spawn_life1"
    );
    animationAddFrame(
        &animation,
        "overlay_spawn_life2"
    );
    animationAddFrame(
        &animation,
        "overlay_spawn_life3"
    );
    animationAddFrame(&animation, "overlay_life");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    windWorldHandleAddComponent(Animations,
        &(scenePtr->ecsWorld),
        handle,
        &animations
    );
}

/*
 * Makes the specified bomb ui element visible and
 * plays the spawn animation
 */
static void addBomb(
    Game *gamePtr,
    Scene *scenePtr,
    VecsEntity handle
){
    /* make the entity visible */
    windWorldHandleAddComponent(VisibleMarker,
        &(scenePtr->ecsWorld),
        handle,
        NULL
    );
    /* change the current sprite to spawn 1 */
    SpriteInstruction *spriteInstrPtr
        = vecsWorldEntityGetPtr(SpriteInstruction,
            &(scenePtr->ecsWorld),
            handle
        );
    spriteInstrPtr->spritePtr = resourcesGetSprite(
        gamePtr->resourcesPtr,
        &bombSpawnSpriteId
    );
    /* remove the old animation component if needed */
    windWorldHandleRemoveComponent(Animations,
        &(scenePtr->ecsWorld),
        handle
    );
    /* add new animation component */
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "overlay_spawn_bomb1"
    );
    animationAddFrame(
        &animation,
        "overlay_spawn_bomb2"
    );
    animationAddFrame(
        &animation,
        "overlay_spawn_bomb3"
    );
    animationAddFrame(&animation, "overlay_bomb");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    windWorldHandleAddComponent(Animations,
        &(scenePtr->ecsWorld),
        handle,
        &animations
    );
}

/* Updates the life display */
static void updateLives(
    Game *gamePtr,
    Scene *scenePtr,
    PlayerData *playerDataPtr
){
    #define currentLifeIndex \
        scenePtr->messages.overlayData.lifeIndex
    int playerLifeIndex = playerDataPtr->lives - 1;
    if(playerLifeIndex >= -1){
        /* if too many life icons active right now */
        if(currentLifeIndex > playerLifeIndex){
            int highestLifeIndex = minInt(
                currentLifeIndex,
                config_maxLives - 1
            );
            for(int i = highestLifeIndex; 
                i > playerLifeIndex;
                --i
            ){
                removeLife(
                    gamePtr,
                    scenePtr,
                    scenePtr->messages.overlayData
                        .lifeHandles[i]
                );
            }
        }
        /* otherwise if too few life icons active */
        else if(currentLifeIndex < playerLifeIndex){
            int lowestLifeIndex = maxInt(
                currentLifeIndex + 1,
                0
            );
            for(int i = lowestLifeIndex; 
                i <= playerLifeIndex; 
                ++i
            ){
                addLife(
                    gamePtr,
                    scenePtr,
                    scenePtr->messages.overlayData
                        .lifeHandles[i]
                );
            }
        }
        currentLifeIndex = playerLifeIndex;
    }
    #undef currentLifeIndex
}

/* Updates the bomb display */
static void updateBombs(
    Game *gamePtr,
    Scene *scenePtr,
    PlayerData *playerDataPtr
){
    #define currentBombIndex \
        scenePtr->messages.overlayData.bombIndex
    int playerBombIndex = playerDataPtr->bombs - 1;
    if(playerBombIndex >= -1){
        /* if too many bomb icons active right now */
        if(currentBombIndex > playerBombIndex){
            int highestBombIndex = minInt(
                currentBombIndex,
                config_maxLives - 1
            );
            for(int i = highestBombIndex; 
                i > playerBombIndex;
                --i
            ){
                removeBomb(
                    gamePtr,
                    scenePtr,
                    scenePtr->messages.overlayData
                        .bombHandles[i]
                );
            }
        }
        /* otherwise if too few bomb icons active */
        else if(currentBombIndex < playerBombIndex){
            int lowestBombIndex = maxInt(
                currentBombIndex + 1,
                0
            );
            for(int i = lowestBombIndex; 
                i <= playerBombIndex; 
                ++i
            ){
                addBomb(
                    gamePtr,
                    scenePtr,
                    scenePtr->messages.overlayData
                        .bombHandles[i]
                );
            }
        }
        currentBombIndex = playerBombIndex;
    }
    #undef currentBombIndex
}

/* Updates the power display */
static void updatePower(
    Game *gamePtr,
    Scene *scenePtr,
    PlayerData *playerDataPtr
){
    VecsEntity handle
        = scenePtr->messages.overlayData.powerHandle;
    
    SpriteInstruction *spriteInstrPtr
        = vecsWorldEntityGetPtr(SpriteInstruction,
            &(scenePtr->ecsWorld),
            handle
        );
    int power = playerDataPtr->power;
    /* if power 0, make power ui invisible */
    if(power == 0){
        windWorldHandleRemoveComponent(VisibleMarker,
            &(scenePtr->ecsWorld),
            handle
        );
    }
    /* otherwise, make sure power ui visible */
    else{
        vecsWorldEntitySetComponent(VisibleMarker,
            &(scenePtr->ecsWorld),
            handle,
            NULL
        );
    }
    /* if power not max, set sprite to non-max */
    if(power != config_maxPower){
        spriteInstrPtr->spritePtr = resourcesGetSprite(
            gamePtr->resourcesPtr,
            &powerId
        );
    }
    /* otherwise, set to max */
    else{
        spriteInstrPtr->spritePtr = resourcesGetSprite(
            gamePtr->resourcesPtr,
            &powerMaxId
        );
    }
    /* update sub image */
    SubImage *subImagePtr = vecsWorldEntityGetPtr(
        SubImage,
        &(scenePtr->ecsWorld),
        handle
    );
    subImagePtr->width = (float)(power);
}

/* Updates the overlay UI */
void overlaySystem(Game *gamePtr, Scene *scenePtr){
    init();

    /* bail if not the game */
    if(scenePtr->id != scene_game){
        return;
    }
    
    VecsEntity playerHandle
        = getPlayerHandle(scenePtr);
    PlayerData *playerDataPtr
        = vecsWorldEntityGetPtr(PlayerData,
            &(scenePtr->ecsWorld),
            playerHandle
        );
    
    updateLives(gamePtr, scenePtr, playerDataPtr);
    updateBombs(gamePtr, scenePtr, playerDataPtr);
    updatePower(gamePtr, scenePtr, playerDataPtr);
}