#include "NativeFuncs.h"

/* used for native funcs that work on the entity */
static Game *_gamePtr;
static Scene *_scenePtr;
static WindEntity _handle;

/* the actual native funcs */

#define _angleEpsilon 0.05f
#define _pointEpsilon 0.1f
#define _enemySpawnDist 20.0f
#define _bossY 160.0f
#define _bossInbound 30.0f
#define _bossXLow (_bossInbound + config_gameOffsetX)
#define _bossXHigh (config_gameWidth - _bossInbound \
    + config_gameOffsetX)
#define _bossYLow (_bossInbound + config_gameOffsetY)
#define _bossYHigh (config_gameHeight * 0.72f \
    + config_gameOffsetY)
#define _trapLifetime 36
#define _timeBeforePostDialogue 55

#define assertArity(ARITY, MSG) \
    do{ \
        if(argc != (ARITY)){ \
            pgWarning(MSG); \
            pgError("arity mismatch"); \
        } \
    } while(false); \

#define DECLARE_FLOAT_CONST(FUNCNAME, VALUE) \
    static UNValue FUNCNAME( \
        int argc, \
        UNValue *argv \
    ){ \
        assertArity(0, #FUNCNAME); \
        return unFloatValue(VALUE); \
    }

#define DECLARE_INT_CONST(FUNCNAME, VALUE) \
    static UNValue FUNCNAME( \
        int argc, \
        UNValue *argv \
    ){ \
        assertArity(0, #FUNCNAME); \
        return unIntValue(VALUE); \
    }

#define DECLARE_POLAR_CONST(FUNCNAME, VALUE) \
    static UNValue FUNCNAME( \
        int argc, \
        UNValue *argv \
    ){ \
        assertArity(0, #FUNCNAME); \
        return unVectorValue(VALUE); \
    }

#define DECLARE_POINT_CONST(FUNCNAME, VALUE) \
    static UNValue FUNCNAME( \
        int argc, \
        UNValue *argv \
    ){ \
        assertArity(0, #FUNCNAME); \
        return unPointValue(VALUE); \
    }

/* CONSTANT FUNCTIONS */

/*
 * Returns the difference between angles which is to
 * be ignored (angles within that difference can be
 * considered equal)
 */
DECLARE_FLOAT_CONST(angleEpsilon, _angleEpsilon)

/*
 * Returns the difference between points which is to
 * be ignored (points within that difference can be
 * considered equal)
 */
DECLARE_FLOAT_CONST(pointEpsilon, _pointEpsilon)

/* Returns the game offset as a polar vector */
DECLARE_POLAR_CONST(
    gameOffset,
    polarFromVector(config_gameOffset)
)

/* Returns the game width */
DECLARE_FLOAT_CONST(gameWidth, config_gameWidth)

/* Returns the game height */
DECLARE_FLOAT_CONST(gameHeight, config_gameHeight)

/*
 * Returns the spawning distance for enemies outside
 * the bounds of the game
 */
DECLARE_FLOAT_CONST(enemySpawnDist, _enemySpawnDist)

/* Returns the boss midpoint as a point */
DECLARE_POINT_CONST(
    bossMidpoint,
    ((Point2D){
        (config_gameWidth / 2.0f) + config_gameOffsetX,
        _bossY
    })
)

/* Returns the boss x low */
DECLARE_FLOAT_CONST(bossXLow, _bossXLow)

/* Returns the boss x high */
DECLARE_FLOAT_CONST(bossXHigh, _bossXHigh)

/* Returns the boss y low */
DECLARE_FLOAT_CONST(bossYLow, _bossYLow)

/* Returns the boss y high */
DECLARE_FLOAT_CONST(bossYHigh, _bossYHigh)

/*
 * Returns the time before post dialogue after
 * defeating a boss
 */
DECLARE_INT_CONST(
    timeBeforePostDialogue,
    _timeBeforePostDialogue
)

/* Returns the trap lifetime */
DECLARE_INT_CONST(trapLifetime, _trapLifetime)

/* Returns the mathematical constant pi */
DECLARE_FLOAT_CONST(pi, z_pi)

/* Returns the mathematical constant phi */
DECLARE_FLOAT_CONST(phi, z_phi)

/* UTILITY FUNCTIONS */

/*
 * errors the whole program with the specified
 * error message
 */
static UNValue error(int argc, UNValue *argv){
    assertArity(1, "error expects 1 string arg");
    pgWarning("SCRIPT ERROR");
    pgError(unObjectAsCString(*argv));
    /* should never be reached */
    return unBoolValue(false);
}

/* warns the specified message */
static UNValue warn(int argc, UNValue *argv){
    assertArity(1, "warn expects 1 string arg");
    pgWarning("SCRIPT WARNING");
    pgWarning(unObjectAsCString(*argv));
    return unBoolValue(false);
}

/* GENERAL QUERIES */

/*
 * Returns true if the boss death flag was set and
 * unsets it, false otherwise
 */
static UNValue isBossDead(int argc, UNValue *argv){
    assertArity(0, "isBossDead expects no args");
    if(_scenePtr->messages.bossDeathFlag){
        _scenePtr->messages.bossDeathFlag = false;
        return unBoolValue(true);
    }
    return unBoolValue(false);
}

/*
 * Returns true if the end dialogue flag was set and
 * unsets it, false otherwise
 */
static UNValue isDialogueOver(
    int argc,
    UNValue *argv
){
    assertArity(0, "isDialogueOver expects no args");
    if(_gamePtr->messages.endDialogueFlag){
        _gamePtr->messages.endDialogueFlag = false;
        return unBoolValue(true);
    }
    return unBoolValue(false);
}

/*
 * Returns true if the win flag was set and unsets it,
 * false otherwise
 */
static UNValue isWin(int argc, UNValue *argv){
    assertArity(0, "isWin expects no args");
    if(_scenePtr->messages.winFlag){
        _scenePtr->messages.winFlag = false;
        return unBoolValue(true);
    }
    return unBoolValue(false);
}

/*
 * Returns the difficulty of the game as an int where
 * 1 is normal, 2 is hard, and 3 is lunatic
 */
static UNValue getDifficulty(int argc, UNValue *argv){
    assertArity(0, "getDifficulty expects no args");
    return unIntValue(
        _gamePtr->messages.gameState.difficulty
    );
}

/*
 * Returns the current point position of the player,
 * or the player spawn if such an operation is not
 * possible
 */
static UNValue getPlayerPos(int argc, UNValue *argv){
    assertArity(0, "getPlayerPos expects no args");
    //todo: need to query for player and position
}

#undef assertArity
#undef DECLARE_FLOAT_CONST
#undef DECLARE_INT_CONST
#undef DECLARE_POLAR_CONST
#undef DECLARE_POINT_CONST
/* END actual native funcs */

static UNNativeFuncSet *nativeFuncSetPtr = NULL;
static bool initialized = false;

/*
 * piggyback off the system destructor system to
 * clean up the native func set
 */
static void destroy(){
    if(initialized){
        unNativeFuncSetFree(nativeFuncSetPtr);
        pgFree(nativeFuncSetPtr);
        initialized = false;
    }
}

/*
 * piggyback off the system destructor system to init
 * the native func set
 */
static void init(){
    if(!initialized){
        nativeFuncSetPtr
            = pgAlloc(1, sizeof(*nativeFuncSetPtr));
        *nativeFuncSetPtr = unNativeFuncSetMake();

        #define addNativeFunc(FUNCNAME) \
            unNativeFuncSetAdd( \
                nativeFuncSetPtr, \
                #FUNCNAME, \
                FUNCNAME \
            );
        /* constants */
        addNativeFunc(angleEpsilon);
        addNativeFunc(pointEpsilon);
        addNativeFunc(gameOffset);
        addNativeFunc(gameWidth);
        addNativeFunc(gameHeight);
        addNativeFunc(enemySpawnDist);
        addNativeFunc(bossMidpoint);
        addNativeFunc(bossXLow);
        addNativeFunc(bossXHigh);
        addNativeFunc(bossYLow);
        addNativeFunc(bossYHigh);
        addNativeFunc(timeBeforePostDialogue);
        addNativeFunc(trapLifetime);
        addNativeFunc(pi);
        addNativeFunc(phi);

        /* utility */
        addNativeFunc(error);
        addNativeFunc(warn);

        /* general queries */
        addNativeFunc(isBossDead);
        addNativeFunc(isDialogueOver);
        addNativeFunc(isWin);
        addNativeFunc(getDifficulty);
        
        #undef addNativeFunc
        
        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/*
 * Returns a pointer to the native func set used by
 * the game
 */
UNNativeFuncSet *getNativeFuncSet(){
    init();
    return nativeFuncSetPtr;
}

/* Sets the game pointer for native funcs */
void setGameForNativeFuncs(Game *gamePtr){
    _gamePtr = gamePtr;
}

/* Sets the scene pointer for native funcs */
void setSceneForNativeFuncs(Scene *scenePtr){
    _scenePtr = scenePtr;
}

/* Sets the entity handle for native funcs */
void setEntityForNativeFuncs(WindEntity handle){
    _handle = handle;
}