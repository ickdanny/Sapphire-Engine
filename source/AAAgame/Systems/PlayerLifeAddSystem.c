#include "PlayerLifeAddSystem.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(VecsEntityId)
    | vecsComponentSetFromId(PlayerDataId);

/* Adds lives to the player when signaled */
void playerLifeAddSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    if(scenePtr->messages.livesToAdd > 0){
        VecsQueryItr itr = vecsWorldRequestQueryItr(
            &(scenePtr->ecsWorld),
            accept,
            vecsEmptyComponentSet
        );
        while(vecsQueryItrHasEntity(&itr)){
            VecsEntity entity = vecsQueryItrGet(
                VecsEntity,
                &itr
            );

            PlayerData *playerDataPtr
                = vecsWorldEntityGetPtr(PlayerData,
                    &(scenePtr->ecsWorld),
                    entity
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