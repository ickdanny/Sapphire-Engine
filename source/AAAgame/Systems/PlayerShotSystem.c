#include "PlayerShotSystem.h"

#include "GameCommand.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static String shotID;
static bool initialized = false;

/* destroys the player shot system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        stringFree(&shotID);
        initialized = false;
    }
}

/* inits the player shot system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PlayerDataID);

        shotID = stringMakeC("player_shot");

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/*
 * Adds the shot script to the player in script
 * slot 3
 */
static void addPlayerShot(
    Game *gamePtr,
    Scene *scenePtr
){
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    while(windQueryItrHasEntity(&itr)){
        WindEntity handle = windWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentID(&itr)
        );

        /* bail if the player is in the wrong state */
        PlayerData *playerDataPtr
            = windWorldHandleGetPtr(PlayerData,
                &(scenePtr->ecsWorld),
                handle
            );
        switch(playerDataPtr->stateMachine.state){
            case player_normal:
            case player_bombing:
            case player_respawnIFrames:
                /* continue by adding shot */
                break;
            case player_none:
            case player_dead:
            case player_respawning:
            case player_gameOver:
                /*
                 * bail out and go to next loop
                 * iteration
                 */
                goto loopInc;
            default:
                pgError(
                    "unexpected player state; "
                    SRC_LOCATION
                );
                goto loopInc;
        }

        /* if player has scripts, add in slot 4 */
        if(windWorldHandleContainsComponent(Scripts,
            &(scenePtr->ecsWorld),
            handle
        )){
            Scripts *scriptsPtr
                = windWorldHandleGetPtr(Scripts,
                    &(scenePtr->ecsWorld),
                    handle
                );
            /* if slot 3 is empty */
            if(!scriptsPtr->vm3){
                scriptsPtr->vm3 = vmPoolRequest();
                unVirtualMachineLoad(
                    scriptsPtr->vm3,
                    resourcesGetScript(
                        gamePtr->resourcesPtr,
                        &shotID
                    )
                );
            }
        }
        /* otherwise, add a new script component */
        else{
            Scripts scripts = {0};
            scripts.vm3 = vmPoolRequest();
            unVirtualMachineLoad(
                scripts.vm3,
                resourcesGetScript(
                    gamePtr->resourcesPtr,
                    &shotID
                )
            );
            windWorldHandleQueueAddComponent(Scripts,
                &(scenePtr->ecsWorld),
                handle,
                &scripts
            );
        }
loopInc:
        windQueryItrAdvance(&itr);
    }

    windWorldHandleOrders(&(scenePtr->ecsWorld));
}

/*
 * Detects when the player inputs a shot and adds
 * the shot script to the player
 */
void playerShotSystem(Game *gamePtr, Scene *scenePtr){
    init();

    ArrayList *gameCommandsPtr
        = &(scenePtr->messages.gameCommands);

    for(int i = 0; i < gameCommandsPtr->size; ++i){
        if(arrayListGet(GameCommand,
                gameCommandsPtr,
                i
            ) == game_shoot
        ){
            addPlayerShot(gamePtr, scenePtr);
            break;
        }
    }
}