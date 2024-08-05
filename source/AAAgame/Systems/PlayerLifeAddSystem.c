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
        bitsetSet(&accept, PlayerDataID);

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

            PlayerData *playerDataPtr
                = windWorldHandleGetPtr(PlayerData,
                    &(scenePtr->ecsWorld),
                    handle
                );
            
            playerDataPtr->lives
                += scenePtr->messages.livesToAdd;
            if(playerDataPtr->lives > config_maxLives){
                playerDataPtr->lives = config_maxLives;
            }
            windQueryItrAdvance(&itr);
        }
        scenePtr->messages.livesToAdd = 0;
    }
}