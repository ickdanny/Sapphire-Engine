#include "NativeFuncs.h"

static UNNativeFuncSet *nativeFuncSetPtr = NULL;
static Bitset playerPosSet;
static bool initialized = false;

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

/*
 * error if the arity given to the native function is
 * not as specified
 */
#define assertArity(ARITY, MSG) \
    do{ \
        if(argc != (ARITY)){ \
            pgWarning(MSG); \
            pgError("arity mismatch"); \
        } \
    } while(false); \

/*
 * Fills the component pointer of the specified name
 * and type with a pointer to the actual component
 * of the entity; error if the entity lacks the
 * requested component
 */
#define fillComponentPtr(TYPENAME, PTRNAME, ERRMSG) \
    do{ \
        if(!windWorldHandleContainsComponent( \
            TYPENAME, \
            &(_scenePtr->ecsWorld), \
            _handle \
        )){ \
            pgError(ERRMSG); \
        } \
        PTRNAME = windWorldHandleGetPtr( \
            TYPENAME, \
            &(_scenePtr->ecsWorld), \
            _handle \
        ); \
    } while(false)

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

    /* get players with position */
    WindQueryItr itr = windWorldRequestQueryItr(
        &(_scenePtr->ecsWorld),
        &playerPosSet,
        NULL
    );
    /* if player exists, grab first one */
    if(windQueryItrHasEntity(&itr)){
        Position *positionPtr = windQueryItrGetPtr(
            Position,
            &itr
        );
        return unPointValue(positionPtr->currentPos);
    }
    /* otherwise just return player spawn */
    else{
        return unPointValue(config_playerSpawn);
    }
}

/* ENTITY GRAPHICS */

/* Marks the entity as visible */
static UNValue setVisible(int argc, UNValue *argv){
    assertArity(0, "setVisible expects no args");
    windWorldHandleQueueSetComponent(VisibleMarker,
        &(_scenePtr->ecsWorld),
        _handle,
        NULL
    );
    return unBoolValue(false);
}

/* Unmarks the entity as visible */
static UNValue removeVisible(int argc, UNValue *argv){
    assertArity(0, "removeVisible expects no args");
    windWorldHandleQueueRemoveComponent(VisibleMarker,
        &(_scenePtr->ecsWorld),
        _handle
    );
    return unBoolValue(false);
}

/*
 * Set the sprite of the entity to the requested image
 */
static UNValue setSprite(int argc, UNValue *argv){
    assertArity(1, "setSprite expects 1 string arg");
    String *stringPtr
        = &(unObjectAsString(*argv)->string);
    TFSprite *spritePtr = resourcesGetSprite(
        _gamePtr->resourcesPtr,
        stringPtr
    );
    if(!spritePtr){
        pgWarning(stringPtr->_ptr);
        pgError("failed to get sprite;" SRC_LOCATION);
    }

    SpriteInstruction *spriteInstrPtr = NULL;
    fillComponentPtr(SpriteInstruction,
        spriteInstrPtr,
        "calling setSprite on entity without "
        "sprite instruction; " SRC_LOCATION
    );

    spriteInstrPtr->spritePtr = spritePtr;

    return unBoolValue(false);
}

/*
 * Sets the sprite instruction of the entity based on
 * the parameters (string id, int depth, vector offset,
 * float rotation, float scale)
 */
static UNValue setSpriteInstruction(
    int argc,
    UNValue *argv
){
    assertArity(
        5,
        "usage: setSpriteInstruction("
        "string id, int depth, vector offset, float "
        "rotation, float scale)"
    );

    /* build the sprite instruction */
    SpriteInstruction spriteInstr = {0};
    String *stringPtr
        = &(unObjectAsString(argv[0])->string);
    spriteInstr.spritePtr = resourcesGetSprite(
        _gamePtr->resourcesPtr,
        stringPtr
    );
    if(!spriteInstr.spritePtr){
        pgWarning(stringPtr->_ptr);
        pgError("failed to get sprite;" SRC_LOCATION);
    }
    spriteInstr.depth = unAsInt(argv[1]);
    spriteInstr.offset = polarToVector(
        unAsVector(argv[2])
    );
    spriteInstr.rotation = unAsFloat(argv[3]);
    spriteInstr.scale = unAsFloat(argv[4]);

    /* queue a set command */
    windWorldHandleQueueSetComponent(SpriteInstruction,
        &(_scenePtr->ecsWorld),
        _handle,
        &spriteInstr
    );

    return unBoolValue(false);
}

/* Sets the depth of the entity sprite */
static UNValue setDepth(int argc, UNValue *argv){
    assertArity(1, "setDepth expects 1 int arg");
    int depth = unAsInt(*argv);

    SpriteInstruction *spriteInstrPtr = NULL;
    fillComponentPtr(SpriteInstruction,
        spriteInstrPtr,
        "calling setDepth on entity without "
        "sprite instruction; " SRC_LOCATION
    );
    spriteInstrPtr->depth = depth;

    return unBoolValue(false);
}

/* Sets the rotation of the entity sprite */
static UNValue setRotation(int argc, UNValue *argv){
    assertArity(1, "setRotation expects 1 float arg");
    float rotation = unAsFloat(*argv);

    SpriteInstruction *spriteInstrPtr = NULL;
    fillComponentPtr(SpriteInstruction,
        spriteInstrPtr,
        "calling setRotation on entity without "
        "sprite instruction; " SRC_LOCATION
    );
    spriteInstrPtr->rotation = rotation;

    return unBoolValue(false);
}

/* Sets the scale of the entity sprite */
static UNValue setScale(int argc, UNValue *argv){
    assertArity(1, "setScale expects 1 float arg");
    float scale = unAsFloat(*argv);

    SpriteInstruction *spriteInstrPtr = NULL;
    fillComponentPtr(SpriteInstruction,
        spriteInstrPtr,
        "calling setScale on entity without "
        "sprite instruction; " SRC_LOCATION
    );

    spriteInstrPtr->scale = scale;

    return unBoolValue(false);
}

/* ENTITY QUERIES */

/* Returns the position of the entity */
static UNValue getPosition(
    int argc,
    UNValue *argv
){
    assertArity(0, "getPosition expects no args");

    Position *positionPtr = NULL;
    fillComponentPtr(Position,
        positionPtr,
        "cannot get entity position when entity lacks "
        "position component; " SRC_LOCATION
    );

    return unPointValue(positionPtr->currentPos);
}

/* Returns the x coordinate of the entity */
static UNValue getX(int argc, UNValue *argv){
    assertArity(0, "getX expects no args");

    Point2D position
        = unAsPoint(getPosition(0, NULL));
    return unFloatValue(position.x);
}

/* Returns the y coordinate of the entity */
static UNValue getY(int argc, UNValue *argv){
    assertArity(0, "getY expects no args");

    Point2D position
        = unAsPoint(getPosition(0, NULL));
    return unFloatValue(position.y);
}

/* Returns the angle from the entity to the player */
static UNValue getAngleToPlayer(
    int argc,
    UNValue *argv
){
    assertArity(0, "getAngleToPlayer expects no args");
    
    Point2D playerPos
        = unAsPoint(getPlayerPos(0, NULL));

    Point2D entityPos
        = unAsPoint(getPosition(0, NULL));
    
    //todo: do i seriously have no angle func?
    float angle = vector2DAngle(vector2DFromAToB(
        entityPos,
        playerPos
    ));

    return unFloatValue(angle);
}

/*
 * Returns the velocity of the entity (as a polar
 * vector)
 */
static UNValue getVelocity(int argc, UNValue *argv){
    assertArity(0, "getVelocity expects no args");

    Velocity *velocityPtr = NULL;
    fillComponentPtr(Velocity,
        velocityPtr,
        "cannot get entity velocity when entity lacks "
        "velocity component; " SRC_LOCATION
    );

    return unVectorValue(*velocityPtr);
}

/*
 * Returns the speed of the entity (r component of
 * its velocity
 */
static UNValue getSpeed(int argc, UNValue *argv){
    assertArity(0, "getSpeed expects no args");

    Polar velocity
        = unAsVector(getVelocity(0, NULL));
    return unFloatValue(velocity.magnitude);
}

/*
 * Returns the angle of the entity (a component of
 * its velocity
 */
static UNValue getAngle(int argc, UNValue *argv){
    assertArity(0, "getAngle expects no args");

    Polar velocity
        = unAsVector(getVelocity(0, NULL));
    return unFloatValue(velocity.angle);
}

/* Returns the sprite spin of the entity */
static UNValue getSpin(int argc, UNValue *argv){
    assertArity(0, "getSpin expects no args");

    SpriteSpin *spinPtr = NULL;
    fillComponentPtr(SpriteSpin,
        spinPtr,
        "cannot get entity sprite spin when entity "
        "lacks sprite spin component; " SRC_LOCATION
    );

    return unFloatValue(*spinPtr);
}

/*
 * Returns true if the entity is spawning, false
 * otherwise; spawning scripts go in virtual machine
 * slots 3 and 4
 */
static UNValue isSpawning(int argc, UNValue *argv){
    assertArity(0, "isSpawning expects no args");

    Scripts *scriptsPtr = NULL;
    fillComponentPtr(Scripts,
        scriptsPtr,
        "unexpectedly unable to get Scripts ptr for "
        "entity that is currently running a script; "
        SRC_LOCATION
    );

    /* if either vm3 or vm4 is active, return true */
    return unBoolValue(
        scriptsPtr->vm3 || scriptsPtr->vm4
    );
}

/* Returns the current power of the player */
static UNValue getPlayerPower(int argc, UNValue *argv){
    assertArity(0, "getPlayerPower expects no args");

    //todo: get the player 
}

#undef assertArity
#undef DECLARE_FLOAT_CONST
#undef DECLARE_INT_CONST
#undef DECLARE_POLAR_CONST
#undef DECLARE_POINT_CONST

/* END actual native funcs */

/*
 * piggyback off the system destructor system to
 * clean up the native func set
 */
static void destroy(){
    if(initialized){
        bitsetFree(&playerPosSet);
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

        playerPosSet = bitsetMake(numComponents);
        bitsetSet(&playerPosSet, PlayerDataID);
        bitsetSet(&playerPosSet, PositionID);

        #define addNativeFunc(FUNCNAME) \
            unNativeFuncSetAdd( \
                nativeFuncSetPtr, \
                #FUNCNAME, \
                FUNCNAME \
            );
        #define addNativeFuncWithName(FUNC, NAME) \
            unNativeFuncSetAdd( \
                nativeFuncSetPtr, \
                #NAME, \
                FUNC \
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

        /* entity graphics */
        addNativeFunc(setVisible);
        addNativeFunc(removeVisible);
        addNativeFunc(setSprite);
        addNativeFunc(setSpriteInstruction);
        addNativeFunc(setDepth);
        addNativeFunc(setRotation);
        addNativeFunc(setScale);

        /* entity queries */
        addNativeFunc(getPosition);
        addNativeFunc(getX);
        addNativeFunc(getY);
        addNativeFunc(getAngleToPlayer);
        addNativeFunc(getVelocity);
        addNativeFunc(getSpeed);
        addNativeFunc(getAngle);
        addNativeFunc(getSpin);
        addNativeFunc(isSpawning);
        
        #undef addNativeFunc
        #undef addNativeFuncWithName
        
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