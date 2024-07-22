#include "Prototypes.h"

/* PLAYER PROTOTYPE CONFIG */
#define shardHitboxRadius 9.0f
#define shardDamage 100
#define shardOutbound -30.0f

#define caltropHitboxRadius 8.0f
#define caltropDamage 400
#define caltropOutbound -30.0f /* just in case */

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
    addDeathScripts(componentListPtr, (DeathScripts){
        .scriptID1 = stringMakeC(
            "spawn_explode_projectile"
        )
    });
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

        addPrototypeFunction(shard);
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