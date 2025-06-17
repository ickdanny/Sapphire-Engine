#include "PlayerBombAddSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the player bomb add system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the player bomb add system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PlayerDataId);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* Adds bombs to the player when signaled */
void playerBombAddSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    init();
    
    if(scenePtr->messages.bombsToAdd > 0){
        VecsQueryItr itr = vecsWorldRequestQueryItr(
            &(scenePtr->ecsWorld),
            &accept,
            NULL
        );
        while(windQueryItrHasEntity(&itr)){
            VecsEntity handle = vecsWorldMakeHandle(
                &(scenePtr->ecsWorld),
                windQueryItrCurrentId(&itr)
            );

            PlayerData *playerDataPtr
                = vecsWorldEntityGetPtr(PlayerData,
                    &(scenePtr->ecsWorld),
                    handle
                );
            
            playerDataPtr->bombs
                += scenePtr->messages.bombsToAdd;
            if(playerDataPtr->bombs > config_maxBombs){
                playerDataPtr->bombs = config_maxBombs;
            }
            vecsQueryItrAdvance(&itr);
        }
        scenePtr->messages.bombsToAdd = 0;
    }
}