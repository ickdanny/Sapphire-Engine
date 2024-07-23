#include "OverlaySystem.h"

static Bitset accept;
static bool initialized = false;

/* destroys the overlay system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the overlay system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PlayerDataID);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* retrieves the player handle */
static WindEntity getPlayerHandle(Scene *scenePtr){
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    /* get first player */
    if(windQueryItrHasEntity(&itr)){
        return windWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentID(&itr)
        );
    }
    /* error if cannot find player */
    else{
        pgError(
            "failed to find player; "
            SRC_LOCATION
        );
        return (WindEntity){0};
    }
}

/*
 * Makes the specified life ui element invisible and
 * spawns an explode at its location
 */
static void removeLife(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity handle
){
    /* make the entity invisible */
    windWorldHandleRemoveComponent(VisibleMarker,
        &(scenePtr->ecsWorld),
        handle
    );
    //todo: spawn an explode
}

/*
 * Makes the specified life ui element visible and
 * plays the spawn animation
 */
static void addLife(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity handle
){
    /* make the entity visible */
    windWorldHandleAddComponent(VisibleMarker,
        &(scenePtr->ecsWorld),
        handle,
        NULL
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
        else if (currentLifeIndex < playerLifeIndex) {
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
    //todo
}

/* Updates the power display */
static void updatePower(
    Game *gamePtr,
    Scene *scenePtr,
    PlayerData *playerDataPtr
){
    //todo
}

/* Updates the overlay UI */
void overlaySystem(Game *gamePtr, Scene *scenePtr){
    init();

    /* bail if not the game */
    if(scenePtr->id != scene_game){
        return;
    }
    
    WindEntity playerHandle
        = getPlayerHandle(scenePtr);
    PlayerData *playerDataPtr
        = windWorldHandleGetPtr(PlayerData,
            &(scenePtr->ecsWorld),
            playerHandle
        );
    
    updateLives(gamePtr, scenePtr, playerDataPtr);
    updateBombs(gamePtr, scenePtr, playerDataPtr);
    updatePower(gamePtr, scenePtr, playerDataPtr);
}