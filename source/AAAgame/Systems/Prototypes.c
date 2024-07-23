#include "Prototypes.h"

/* PLAYER PROTOTYPE CONFIG */
#define shardHitboxRadius 9.0f
#define shardDamage 100
#define shardOutbound -30.0f

#define caltropHitboxRadius 8.0f
#define caltropDamage 400
#define caltropOutbound -30.0f /* just in case */

#define spikeHitboxRadius 5.0f
#define spikeDamage 100
#define spikeOutbound -30.0f

#define bombHitboxRadius 20.0f
#define bombDamagePerTick 1 /* small; bomb explodes */
#define bombOutbound -100.0f /* just in case */

#define bombExplodeHitboxRadius 27.0f
#define bombExplodeDamage 2000

/* PICKUP PROTOTYPE CONFIG */
#define smallPickupHitboxRadius 8.0f
#define largePickupHitboxRadius 12.0f
#define pickupOutbound -100.0f
#define smallPowerGain 1
#define largePowerGain 5

typedef void (*PrototypeFunction)(
    Game*,
    Scene*,
    ArrayList*,
    int
);

#define DECLARE_PROTOTYPE(NAME) \
    static void NAME( \
        Game *gamePtr, \
        Scene *scenePtr, \
        ArrayList *componentListPtr, \
        int depthOffset \
    )

/* PLAYER PROTOTYPES */

DECLARE_PROTOTYPE(shard){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbFromRadius(shardHitboxRadius)
    );
    addEnemyCollisionSource(
        componentListPtr,
        collision_death
    );
    addDamage(componentListPtr, shardDamage);
    addOutbound(componentListPtr, shardOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "shard",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    addRotateSpriteForward(componentListPtr);
    addDeathCommand(componentListPtr, death_script);
    addDeathScripts(componentListPtr, ((DeathScripts){
        .scriptID1 = stringMakeC("remove_ghost"),
        .scriptID3 = stringMakeC(
            "spawn_explode_projectile"
        )
    }));
}

DECLARE_PROTOTYPE(caltrop){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbFromRadius(caltropHitboxRadius)
    );
    addEnemyCollisionSource(
        componentListPtr,
        collision_death
    );
    addDamage(componentListPtr, caltropDamage);
    addOutbound(componentListPtr, caltropOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "caltrop",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    addDeathCommand(componentListPtr, death_script);
    addDeathScripts(componentListPtr, ((DeathScripts){
        .scriptID1 = stringMakeC("remove_ghost"),
        .scriptID3 = stringMakeC(
            "spawn_explode_projectile"
        ),
        .scriptID4 = stringMakeC("death_caltrop")
    }));
}

#define DECLARE_SPIKE_PROTOTYPE(NAME) \
    DECLARE_PROTOTYPE(NAME){ \
        addVisible(componentListPtr); \
        addCollidable(componentListPtr); \
        addHitbox( \
            componentListPtr, \
            aabbFromRadius(spikeHitboxRadius) \
        ); \
        addEnemyCollisionSource( \
            componentListPtr, \
            collision_death \
        ); \
        addDamage(componentListPtr, spikeDamage); \
        addOutbound(componentListPtr, spikeOutbound); \
        addSpriteInstructionSimple( \
            componentListPtr, \
            gamePtr, \
            #NAME, \
            config_playerBulletDepth + depthOffset, \
            (Vector2D){0} \
        ); \
        addDeathCommand( \
            componentListPtr, \
            death_script \
        ); \
        addDeathScripts( \
            componentListPtr, \
            ((DeathScripts){ \
                .scriptID1 = stringMakeC( \
                    "remove_ghost" \
                ), \
                .scriptID3 = stringMakeC( \
                    "spawn_explode_projectile" \
                ) \
            }) \
        ); \
    }
DECLARE_SPIKE_PROTOTYPE(spike_bottomLeft)
DECLARE_SPIKE_PROTOTYPE(spike_bottomRight)
DECLARE_SPIKE_PROTOTYPE(spike_topLeft)
DECLARE_SPIKE_PROTOTYPE(spike_topRight)
#undef DECLARE_SPIKE_PROTOTYPE

DECLARE_PROTOTYPE(bomb){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbFromRadius(bombHitboxRadius)
    );
    /* bomb explodes on enemies */
    addEnemyCollisionSource(
        componentListPtr,
        collision_death
    );
    /*
     * bullets collide with bomb but nothing happens
     * to bomb
     */
    addBulletCollisionSource(
        componentListPtr,
        collision_none
    );
    addDamage(componentListPtr, bombDamagePerTick);
    addOutbound(componentListPtr, bombOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "bomb1",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(true);
    animationAddFrame(&animation, "bomb1");
    animationAddFrame(&animation, "bomb2");
    animationAddFrame(&animation, "bomb3");
    animationAddFrame(&animation, "bomb4");
    animationAddFrame(&animation, "bomb5");
    animationAddFrame(&animation, "bomb6");
    animationAddFrame(&animation, "bomb7");
    animationAddFrame(&animation, "bomb8");
    animationAddFrame(&animation, "bomb9");
    animationAddFrame(&animation, "bomb10");
    animationAddFrame(&animation, "bomb11");
    animationAddFrame(&animation, "bomb12");
    animationAddFrame(&animation, "bomb13");
    animationAddFrame(&animation, "bomb14");
    animationAddFrame(&animation, "bomb15");
    animationAddFrame(&animation, "bomb16");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 3;
    addAnimations(componentListPtr, &animations);

    addRotateSpriteForward(componentListPtr);
    addDeathCommand(componentListPtr, death_script);
    addDeathScripts(componentListPtr, ((DeathScripts){
        .scriptID1 = stringMakeC("remove_ghost"),
        .scriptID3 = stringMakeC(
            "spawn_explode_bomb"
        )
    }));
}

DECLARE_PROTOTYPE(explode_bomb){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbFromRadius(bombExplodeHitboxRadius)
    );
    addBulletCollisionSource(
        componentListPtr,
        collision_none
    );
    addDamage(componentListPtr, bombExplodeDamage);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "explode_bomb1",
        config_playerBulletDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(&animation, "explode_bomb1");
    animationAddFrame(&animation, "explode_bomb2");
    animationAddFrame(&animation, "explode_bomb3");
    animationAddFrame(&animation, "explode_bomb4");
    animationAddFrame(&animation, "explode_bomb5");
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 2;
    addAnimations(componentListPtr, &animations);
}

/* PICKUP PROTOTYPES */
DECLARE_PROTOTYPE(power_small){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbFromRadius(smallPickupHitboxRadius)
    );
    addPickupCollisionSource(
        componentListPtr,
        collision_pickup
    );
    addPowerGain(componentListPtr, smallPowerGain);
    addOutbound(componentListPtr, pickupOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "pickup_powerSmall",
        config_pickupDepth + depthOffset,
        (Vector2D){0}
    );
}

DECLARE_PROTOTYPE(power_large){
    addVisible(componentListPtr);
    addCollidable(componentListPtr);
    addHitbox(
        componentListPtr,
        aabbFromRadius(largePickupHitboxRadius)
    );
    addPickupCollisionSource(
        componentListPtr,
        collision_pickup
    );
    addPowerGain(componentListPtr, largePowerGain);
    addOutbound(componentListPtr, pickupOutbound);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "pickup_powerLarge",
        config_pickupDepth + depthOffset,
        (Vector2D){0}
    );
}

/* MISCELLANEOUS PROTOTYPES */

DECLARE_PROTOTYPE(explode_projectile){
    pgWarning("spawning explode projectile");
    addVisible(componentListPtr);
    addSpriteInstructionSimple(
        componentListPtr,
        gamePtr,
        "explode_projectile1",
        config_effectDepth + depthOffset,
        (Vector2D){0}
    );
    Animations animations = animationListMake();
    Animation animation = animationMake(false);
    animationAddFrame(
        &animation,
        "explode_projectile1"
    );
    animationAddFrame(
        &animation,
        "explode_projectile2"
    );
    animationAddFrame(
        &animation,
        "explode_projectile3"
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        animation
    );
    animations.currentIndex = 0;
    animations.idleIndex = 0;
    animations._maxTick = 2;
    addAnimations(componentListPtr, &animations);
}

/* map from char* (unowned) to PrototypeFunction */
static HashMap prototypeFunctionMap;
static bool initialized = false;

/*
 * piggyback off the system destructor system to clean
 * up the prototype function map
 */
static void destroy(){
    if(initialized){
        hashMapFree(char*, PrototypeFunction,
            &prototypeFunctionMap
        );
        initialized = false;
    }
}

/*
 * piggyback off the system destructor system to init
 * the prototype function map
 */
static void init(){
    if(!initialized){
        prototypeFunctionMap = hashMapMake(
            char*,
            PrototypeFunction,
            50,
            cStringHash,
            cStringEquals
        );

        #define addPrototypeFunction(FUNCNAME) \
            hashMapPut(char*, PrototypeFunction, \
                &(prototypeFunctionMap), \
                #FUNCNAME, \
                FUNCNAME \
            )

        /* player prototypes */
        addPrototypeFunction(shard);
        addPrototypeFunction(caltrop);
        addPrototypeFunction(spike_bottomLeft);
        addPrototypeFunction(spike_bottomRight);
        addPrototypeFunction(spike_topLeft);
        addPrototypeFunction(spike_topRight);
        addPrototypeFunction(bomb);
        addPrototypeFunction(explode_bomb);

        /* pickup prototypes */
        addPrototypeFunction(power_small);
        addPrototypeFunction(power_large);

        /* miscellaneous prototypes */
        addPrototypeFunction(explode_projectile);

        #undef addPrototypeFunction

        initialized = true;
    }
}

/*
 * Applies the specified prototype to the given
 * component list array; also applies the depth
 * offset when creating the sprite instruction
 */
void applyPrototype(
    Game *gamePtr,
    Scene *scenePtr,
    String *prototypeIDPtr,
    ArrayList *componentListPtr,
    int depthOffset
){
    init();

    assertNotNull(
        gamePtr,
        "null game passed;" SRC_LOCATION
    );
    assertNotNull(
        scenePtr,
        "null scene passed;" SRC_LOCATION
    );
    assertNotNull(
        prototypeIDPtr,
        "null prototype ID passed;" SRC_LOCATION
    );
    assertNotNull(
        prototypeIDPtr->_ptr,
        "prototype string holds null;" SRC_LOCATION
    );
    
    if(!hashMapHasKey(char*, PrototypeFunction,
        &(prototypeFunctionMap),
        prototypeIDPtr->_ptr
    )){
        pgWarning(prototypeIDPtr->_ptr);
        pgError(
            "failed to find prototype; " SRC_LOCATION
        );
    }
    PrototypeFunction prototypeFunction = hashMapGet(
        char*,
        PrototypeFunction,
        &(prototypeFunctionMap),
        prototypeIDPtr->_ptr
    );

    prototypeFunction(
        gamePtr,
        scenePtr,
        componentListPtr,
        depthOffset
    );
}