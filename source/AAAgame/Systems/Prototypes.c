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

#define bombHitboxRadius 25.0f
#define bombDamagePerTick 1 /* small; bomb explodes */
#define bombOutbound -100.0f /* just in case */

#define bombExplodeHitboxRadius 35.0f
#define bombExplodeDamage 2000

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

        /* miscellaneous prototypes */
        addPrototypeFunction(explode_projectile);

        //todo add other prototype functions

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