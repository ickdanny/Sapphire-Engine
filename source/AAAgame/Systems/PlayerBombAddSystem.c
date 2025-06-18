#include "PlayerBombAddSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(VecsEntityId)
    | vecsComponentSetFromId(PlayerDataId);

/* Adds bombs to the player when signaled */
void playerBombAddSystem(
    Game *gamePtr,
    Scene *scenePtr
){    
    if(scenePtr->messages.bombsToAdd <= 0){
        return;
    }
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&itr)){
        VecsEntity entity = vecsQueryItrGet(VecsEntity,
            &itr
        );
        PlayerData *playerDataPtr
            = vecsWorldEntityGetPtr(PlayerData,
                &(scenePtr->ecsWorld),
                entity
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