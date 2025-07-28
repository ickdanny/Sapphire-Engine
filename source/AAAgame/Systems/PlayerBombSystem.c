#include "PlayerBombSystem.h"

static String bombId;
static bool initialized = false;

/* destroys the player bomb system */
static void destroy(){
    if(initialized){
        stringFree(&bombId);
        initialized = false;
    }
}

/* inits the player bomb system */
static void init(){
    if(!initialized){
        bombId = stringMakeC("player_bomb");

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

    VecsEntity playerHandle
        = scenePtr->messages.playerStateEntry
            .playerHandle;

    PlayerData *playerDataPtr = vecsWorldEntityGetPtr(
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
    if(!vecsWorldEntityContainsComponent(Scripts,
        &(scenePtr->ecsWorld),
        playerHandle
    )){
        Scripts scripts = {0};
        vecsWorldEntityAddComponent(Scripts,
            &(scenePtr->ecsWorld),
            playerHandle,
            &scripts
        );
    }
    Scripts *scriptsPtr = vecsWorldEntityGetPtr(
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
    necroVirtualMachineLoad(
        scriptsPtr->vm4,
        resourcesGetScript(
            gamePtr->resourcesPtr,
            &bombId
        )
    );
}