#include "DeathHandlerSystem.h"

#define powerLossRatio 4
#define powerMinLoss 4
#define powerMaxLoss 10

/*
 * Calculates the power loss a player should receive
 * as penalty for death
 */
static int powerLoss(int initPower){
    if(initPower < powerMinLoss){
        return initPower;
    }
    
    int loss = initPower / powerLossRatio;
    if(loss < powerMinLoss){
        loss = powerMinLoss;
    }
    else if(loss > powerMaxLoss){
        loss = powerMaxLoss;
    }
    return loss;
}

/*
 * handles death scripts by creating a ghost at the
 * location of the dead entity
 */
static void handleDeathScript(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity handle,
    bool removeEntityAfter
){
    if(windWorldHandleContainsComponent(DeathScripts,
        &(scenePtr->ecsWorld),
        handle
    )){
        DeathScripts *deathScriptsPtr
            = windWorldHandleGetPtr(DeathScripts,
                &(scenePtr->ecsWorld),
                handle
            );

        declareList(componentList, 3);

        /*
         * Make a new scripts object based on the
         * death script ids
         */
        Scripts scripts = {0};
        #define addScriptIfStringAllocated(SLOT) \
            do{ \
                if(deathScriptsPtr->scriptID##SLOT \
                        ._ptr \
                    && !stringIsEmpty( \
                        &(deathScriptsPtr \
                            ->scriptID##SLOT) \
                    ) \
                ){ \
                    UNObjectFunc *scriptPtr \
                        = resourcesGetScript( \
                            gamePtr->resourcesPtr, \
                            &(deathScriptsPtr \
                                ->scriptID##SLOT) \
                        ); \
                    scripts.vm##SLOT \
                        = vmPoolRequest(); \
                    unVirtualMachineLoad( \
                        scripts.vm##SLOT, \
                        scriptPtr \
                    ); \
                } \
            } while(false)

        addScriptIfStringAllocated(1);
        addScriptIfStringAllocated(2);
        addScriptIfStringAllocated(3);
        addScriptIfStringAllocated(4);

        #undef addScriptIfStringAllocated

        addScripts(&componentList, scripts);

        /*
         * clear the death scripts object to avoid
         * having the destructor reclaim the VMs
         */
        memset(
            deathScriptsPtr,
            0,
            sizeof(*deathScriptsPtr)
        );

        /*
         * if the entity had a position, add the
         * position to the ghost also
         */
        if(windWorldHandleContainsComponent(Position,
            &(scenePtr->ecsWorld),
            handle
        )){
            Position *positionPtr
                = windWorldHandleGetPtr(
                    Position,
                    &(scenePtr->ecsWorld),
                    handle
                );
            addPosition(
                &componentList,
                positionPtr->currentPos
            );
        }

        /*
         * if the entity had a velocity, add the angle
         * to the ghost but not the magnitude
         */
        if(windWorldHandleContainsComponent(Velocity,
            &(scenePtr->ecsWorld),
            handle
        )){
            Velocity velocity
                = windWorldHandleGet(
                    Velocity,
                    &(scenePtr->ecsWorld),
                    handle
                );
            velocity.magnitude = 0;
            addVelocity(&componentList, velocity);
        }

        /* spawn the ghost */
        addEntityAndFreeList(
            &componentList,
            scenePtr,
            NULL
        );
    }

    /* optionally remove the dead entity */
    if(removeEntityAfter){
        windWorldHandleRemoveEntity(
            &(scenePtr->ecsWorld),
            handle
        );
    }
}

/*
 * handles player death; error if the entity handle
 * is not in fact a player
 */
static void handlePlayerDeath(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity playerHandle
){
    /* remove script component and pickup collision */
    windWorldHandleRemoveComponent(Scripts,
        &(scenePtr->ecsWorld),
        playerHandle
    );
    windWorldHandleRemoveComponent(
        PickupCollisionTarget,
        &(scenePtr->ecsWorld),
        playerHandle
    );

    /* create ghost for death spawn */
    handleDeathScript(
        gamePtr,
        scenePtr,
        playerHandle,
        false
    );

    /* subtract power */
    PlayerData *playerDataPtr = windWorldHandleGetPtr(
        PlayerData,
        &(scenePtr->ecsWorld),
        playerHandle
    );
    playerDataPtr->power 
        -= powerLoss(playerDataPtr->power);
}

/* Handles an entity's death command */
static void handleDeathCommand(
    Game *gamePtr,
    Scene *scenePtr,
    WindEntity handle,
    DeathCommand command
){
    switch(command){
        case death_remove:
            windWorldHandleRemoveEntity(
                &(scenePtr->ecsWorld),
                handle
            );
            break;
        case death_player:
            handlePlayerDeath(
                gamePtr,
                scenePtr,
                handle
            );
            break;
        case death_boss:
            scenePtr->messages.bossDeathFlag = true;
            handleDeathScript(
                gamePtr,
                scenePtr,
                handle,
                false
            );
            break;
        case death_script:
            handleDeathScript(
                gamePtr,
                scenePtr,
                handle,
                true
            );
            break;
        default:
            pgError(
                "unexpected default death command; "
                SRC_LOCATION
            );
    }
}

/* Handles entities flagged as dead */
void deathHandlerSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    ArrayList *deathsPtr
        = &(scenePtr->messages.deaths);
    for(size_t i = 0; i < deathsPtr->size; ++i){
        WindEntity handle = arrayListGet(WindEntity,
            deathsPtr,
            i
        );
        if(!windWorldIsHandleAlive(
            &(scenePtr->ecsWorld),
            handle
        )){
            continue;
        }
        /* if entity had death command, use it */
        if(windWorldHandleContainsComponent(
            DeathCommand,
            &(scenePtr->ecsWorld),
            handle
        )){
            DeathCommand command = windWorldHandleGet(
                DeathCommand,
                &(scenePtr->ecsWorld),
                handle
            );
            handleDeathCommand(
                gamePtr,
                scenePtr,
                handle,
                command
            );
        }
        /* otherwise, default to removal */
        else{
            windWorldHandleRemoveEntity(
                &(scenePtr->ecsWorld),
                handle
            );
        }
    }

    /* this system does not clear deaths */
}