#include "PlayerLifeAddSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the player life add system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the player life add system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, PlayerDataId);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* Adds lives to the player when signaled */
void playerLifeAddSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    init();

    if(scenePtr->messages.livesToAdd > 0){
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
            
            playerDataPtr->lives
                += scenePtr->messages.livesToAdd;
            if(playerDataPtr->lives > config_maxLives){
                playerDataPtr->lives = config_maxLives;
            }
            vecsQueryItrAdvance(&itr);
        }
        scenePtr->messages.livesToAdd = 0;
    }
}