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
    VecsEntity entity,
    bool removeEntityAfter
){
    if(vecsWorldEntityContainsComponent(DeathScripts,
        &(scenePtr->ecsWorld),
        entity
    )){
        DeathScripts *deathScriptsPtr
            = vecsWorldEntityGetPtr(DeathScripts,
                &(scenePtr->ecsWorld),
                entity
            );

        declareList(componentList, 3);

        /*
         * Make a new scripts object based on the
         * death script ids
         */
        Scripts scripts = {0};
        #define addScriptIfStringAllocated(SLOT) \
            do{ \
                if(deathScriptsPtr->scriptId##SLOT \
                        ._ptr \
                    && !stringIsEmpty( \
                        &(deathScriptsPtr \
                            ->scriptId##SLOT) \
                    ) \
                ){ \
                    UNObjectFunc *scriptPtr \
                        = resourcesGetScript( \
                            gamePtr->resourcesPtr, \
                            &(deathScriptsPtr \
                                ->scriptId##SLOT) \
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

        /* do not clear the death scripts object */

        /*
         * if the entity had a position, add the
         * position to the ghost also
         */
        if(vecsWorldEntityContainsComponent(Position,
            &(scenePtr->ecsWorld),
            entity
        )){
            Position *positionPtr
                = vecsWorldEntityGetPtr(
                    Position,
                    &(scenePtr->ecsWorld),
                    entity
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
        if(vecsWorldEntityContainsComponent(Velocity,
            &(scenePtr->ecsWorld),
            entity
        )){
            Velocity velocity
                = vecsWorldEntityGet(Velocity,
                    &(scenePtr->ecsWorld),
                    entity
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
        vecsWorldEntityRemoveEntity(
            &(scenePtr->ecsWorld),
            entity
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
    VecsEntity playerHandle
){
    /* remove script component and pickup collision */
    vecsWorldEntityRemoveComponent(Scripts,
        &(scenePtr->ecsWorld),
        playerHandle
    );
    vecsWorldEntityRemoveComponent(
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
    PlayerData *playerDataPtr = vecsWorldEntityGetPtr(
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
    VecsEntity handle,
    DeathCommand command
){
    switch(command){
        case death_remove:
            vecsWorldEntityRemoveEntity(
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
        VecsEntity entity = arrayListGet(VecsEntity,
            deathsPtr,
            i
        );
        /*
         * skip over entities that are somehow already
         * dead
         */
        if(!vecsWorldIsEntityLive(
            &(scenePtr->ecsWorld),
            entity
        )){
            continue;
        }
        /* if entity had death command, use it */
        if(vecsWorldEntityContainsComponent(
            DeathCommand,
            &(scenePtr->ecsWorld),
            entity
        )){
            DeathCommand command = vecsWorldEntityGet(
                DeathCommand,
                &(scenePtr->ecsWorld),
                entity
            );
            handleDeathCommand(
                gamePtr,
                scenePtr,
                entity,
                command
            );
        }
        /* otherwise, default to removal */
        else{
            vecsWorldEntityRemoveEntity(
                &(scenePtr->ecsWorld),
                entity
            );
        }
    }

    /* this system does not clear deaths */
}