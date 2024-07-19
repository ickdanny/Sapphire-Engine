#include "CollisionHandlerSystem.h"

/*
 * Handles a death collision command by publishing a
 * death message for the entity
 */
void handleDeathCommand(
    Scene *scenePtr,
    WindEntity handle
){
    arrayListPushBack(WindEntity,
        &(scenePtr->messages.deaths),
        handle
    );
}

/*
 * Handles a damage command by dealing damage to the
 * affected entity based on the colliding entity
 */
void handleDamageCommand(
    Scene *scenePtr,
    WindEntity handle,
    WindEntity collidedHandle
){
    /* bail if entity has no health */
    if(!windWorldHandleContainsComponent(Health,
        &(scenePtr->ecsWorld),
        handle
    )){
        return;
    }

    /* bail if collided entity has no damage */
    if(!windWorldHandleContainsComponent(Damage,
        &(scenePtr->ecsWorld),
        collidedHandle
    )){
        return;
    }

    Health *healthPtr = windWorldHandleGetPtr(Health,
        &(scenePtr->ecsWorld),
        handle
    );
    if(*healthPtr > 0){
        Damage damage = windWorldHandleGet(Damage,
            &(scenePtr->ecsWorld),
            collidedHandle
        );
        *healthPtr -= damage;
        /* if health <= 0, treat as entity death */
        if(*healthPtr <= 0){
            handleDeathCommand(scenePtr, handle);
        }
    }
}

/* Handles a player collision */
void handlePlayerCommand(
    Scene *scenePtr,
    WindEntity playerHandle
){
    arrayListPushBack(WindEntity,
        &(scenePtr->messages.playerHits),
        playerHandle
    );
}

/*
 * Handles a pickup collision by giving the player
 * power
 */
void handlePickupCommand(
    Scene *scenePtr,
    WindEntity pickupHandle,
    WindEntity collidedHandle
){
    /*
     * bail if the collided handle has no player data
     * component
     */
    if(!windWorldHandleContainsComponent(PlayerData,
        &(scenePtr->ecsWorld),
        collidedHandle
    )){
        goto killPickup;
    }

    /* error if pickup lacks power gain component */
    if(!windWorldHandleContainsComponent(PowerGain,
        &(scenePtr->ecsWorld),
        pickupHandle
    )){
        pgError(
            "Expect all pickups to have power gain; "
            SRC_LOCATION
        );
    }

    PlayerData *playerDataPtr = windWorldHandleGetPtr(
        PlayerData,
        &(scenePtr->ecsWorld),
        collidedHandle
    );
    if(playerDataPtr->power < config_maxPower){
        PowerGain powerGain = windWorldHandleGet(
            PowerGain,
            &(scenePtr->ecsWorld),
            pickupHandle
        );
        playerDataPtr->power += powerGain;
        /* if player gains max power, clear bullets */
        if(playerDataPtr->power > config_maxPower){
            playerDataPtr->power = config_maxPower;
            scenePtr->messages.clearFlag = true;
        }
    }

killPickup:
    /* kill the pickup */
    handleDeathCommand(scenePtr, pickupHandle);
}

#define COLLISION_HANDLER_DECLARE(PREFIX, SUFFIX) \
/* Removes the collision type from the hit entity */ \
void handleRemoveTypeCommand##SUFFIX( \
    Scene *scenePtr, \
    WindEntity handle \
){ \
    /* \
     * unknown whether source or target, assume we \
     * can remove both \
     */ \
    windWorldHandleRemoveComponent( \
        SUFFIX##CollisionSource, \
        &(scenePtr->ecsWorld), \
        handle \
    ); \
    windWorldHandleRemoveComponent( \
        SUFFIX##CollisionTarget, \
        &(scenePtr->ecsWorld), \
        handle \
    ); \
} \
\
/* \
 * Determines what to do with the collided entity \
 * based on its collision command \
 */ \
void handleCollisionCommand##SUFFIX( \
    Scene *scenePtr, \
    WindEntity handle, \
    CollisionCommand command, \
    WindEntity collidedHandle \
){ \
    switch(command){ \
        case collision_none: \
            /* do nothing */ \
            break; \
        case collision_death: \
            handleDeathCommand(scenePtr, handle); \
            break; \
        case collision_damage: \
            handleDamageCommand( \
                scenePtr, \
                handle, \
                collidedHandle \
            ); \
            break; \
        case collision_removeType: \
            handleRemoveTypeCommand##SUFFIX( \
                scenePtr, \
                handle \
            ); \
            break; \
        case collision_player: \
            handlePlayerCommand(scenePtr, handle); \
            break; \
        case collision_pickup: \
            handlePickupCommand( \
                scenePtr, \
                handle, \
                collidedHandle \
            ); \
            break; \
        default: \
            pgError( \
                "unexpected collision command; " \
                SRC_LOCATION \
            ); \
            break; \
    } \
} \
\
/* Handles all collisions of a specific type */ \
void handleCollisions##SUFFIX(Scene *scenePtr){ \
    ArrayList *collisionChannelPtr \
        = &(scenePtr->messages \
            .PREFIX##CollisionList); \
    for(size_t i = 0; \
        i < collisionChannelPtr->size; \
        ++i \
    ){ \
        Collision collision = arrayListGet(Collision, \
            collisionChannelPtr, \
            i \
        ); \
        if(windWorldHandleContainsComponent( \
            SUFFIX##CollisionSource, \
            &(scenePtr->ecsWorld), \
            collision.sourceHandle \
        )){ \
            SUFFIX##CollisionSource sourceCommand \
                = windWorldHandleGet( \
                    SUFFIX##CollisionSource, \
                    &(scenePtr->ecsWorld), \
                    collision.sourceHandle \
                ); \
            handleCollisionCommand##SUFFIX( \
                scenePtr, \
                collision.sourceHandle, \
                sourceCommand, \
                collision.targetHandle \
            ); \
        } \
        if(windWorldHandleContainsComponent( \
            SUFFIX##CollisionTarget, \
            &(scenePtr->ecsWorld), \
            collision.targetHandle \
        )){ \
            SUFFIX##CollisionTarget targetCommand \
                = windWorldHandleGet( \
                    SUFFIX##CollisionTarget, \
                    &(scenePtr->ecsWorld), \
                    collision.targetHandle \
                ); \
            handleCollisionCommand##SUFFIX( \
                scenePtr, \
                collision.targetHandle, \
                targetCommand, \
                collision.sourceHandle \
            ); \
        } \
    } \
}

COLLISION_HANDLER_DECLARE(player, Player)
COLLISION_HANDLER_DECLARE(enemy, Enemy)
COLLISION_HANDLER_DECLARE(bullet, Bullet)
COLLISION_HANDLER_DECLARE(pickup, Pickup)

#undef COLLISION_HANDLER_DECLARE

/* Handles collisions between entities */
void collisionHandlerSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    /* clear player hits messages */
    arrayListClear(WindEntity,
        &(scenePtr->messages.playerHits)
    );

    handleCollisionsPlayer(scenePtr);
    handleCollisionsEnemy(scenePtr);
    handleCollisionsBullet(scenePtr);
    handleCollisionsPickup(scenePtr);
}