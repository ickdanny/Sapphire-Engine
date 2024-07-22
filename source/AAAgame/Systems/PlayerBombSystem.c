#include "PlayerBombSystem.h"

static String bombID;
static bool initialized = false;

/* destroys the player bomb system */
static void destroy(){
    if(initialized){
        stringFree(&bombID);
        initialized = false;
    }
}

/* inits the player bomb system */
static void init(){
    if(!initialized){
        bombID = stringMakeC("player_bomb");

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/*
 * Handles making the player bomb when the player state
 * enters bombing
 */
void playerBombSystem(Game *gamePtr, Scene *scenePtr){
    init();

    /*
     * do nothing if player did not just enter the
     * bombing state
     */
    if(scenePtr->messages.playerStateEntry.state
        != player_bombing
    ){
        return;
    }

    WindEntity playerHandle
        = scenePtr->messages.playerStateEntry
            .playerHandle;

    PlayerData *playerDataPtr = windWorldHandleGetPtr(
        PlayerData,
        &(scenePtr->ecsWorld),
        playerHandle
    );
    /*
     * decrement the bomb count; assume it is positive 
     * for the state to have entered bombing to begin
     * with
     */
    --(playerDataPtr->bombs);

    /* if player lacks script component, add one */
    if(!windWorldHandleContainsComponent(Scripts,
        &(scenePtr->ecsWorld),
        playerHandle
    )){
        Scripts scripts = {0};
        windWorldHandleAddComponent(Scripts,
            &(scenePtr->ecsWorld),
            playerHandle,
            &scripts
        );
    }
    Scripts *scriptsPtr = windWorldHandleGetPtr(
        Scripts,
        &(scenePtr->ecsWorld),
        playerHandle
    );
    /* add bomb in slot 4 */
    assertNull(scriptsPtr->vm4,
        "error: try to add bomb script but slot4 "
        "is occupied; " SRC_LOCATION
    );
    scriptsPtr->vm4 = vmPoolRequest();
    unVirtualMachineLoad(
        scriptsPtr->vm4,
        resourcesGetScript(
            gamePtr->resourcesPtr,
            &bombID
        )
    );
}