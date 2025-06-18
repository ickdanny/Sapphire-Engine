#include "NativeFuncs.h"

#include "GameCommand.h"
#include "Prototypes.h"

static UNNativeFuncSet *nativeFuncSetPtr = NULL;
static Bitset playerPosSet;
static Bitset playerSet;
static bool initialized = false;

/* used for native funcs that work on the entity */
static Game *_gamePtr;
static Scene *_scenePtr;
static VecsEntity _handle;

/* the actual native funcs */

#define _angleEpsilon 0.05f
#define _pointEpsilon 0.05f
#define _enemySpawnDist 20.0f
#define _bossY 180.0f
#define _bossInbound 30.0f
#define _bossXLow (_bossInbound + config_gameOffsetX)
#define _bossXHigh (config_gameWidth - _bossInbound \
    + config_gameOffsetX)
#define _bossYLow (config_gameHeight * 0.72f \
    + config_gameOffsetY)
#define _bossYHigh (config_gameHeight - _bossInbound \
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
        if(!vecsWorldEntityContainsComponent( \
            TYPENAME, \
            &(_scenePtr->ecsWorld), \
            _handle \
        )){ \
            pgError(ERRMSG); \
        } \
        PTRNAME = vecsWorldEntityGetPtr( \
            TYPENAME, \
            &(_scenePtr->ecsWorld), \
            _handle \
        ); \
    } while(false)

/*
 * Sets the component of the specified type to the
 * specified value, either by directly changing it if
 * the component is already present or by queueing
 * an add component order if not
 */
#define setComponent(TYPENAME, COMPONENTPTR) \
    do{ \
        if(vecsWorldEntityContainsComponent( \
            TYPENAME, \
            &(_scenePtr->ecsWorld), \
            _handle \
        )){ \
            TYPENAME *componentPtr \
                = vecsWorldEntityGetPtr( \
                    TYPENAME, \
                    &(_scenePtr->ecsWorld), \
                    _handle \
                ); \
            *componentPtr = *(COMPONENTPTR); \
        } \
        else{ \
            windWorldHandleQueueSetComponent( \
                TYPENAME, \
                &(_scenePtr->ecsWorld), \
                _handle, \
                (COMPONENTPTR) \
            ); \
        } \
    } while(false)

/*
 * Removes the component of the specified type from
 * the entity
 */
#define removeComponent(TYPENAME) \
    vecsWorldEntityQueueRemoveComponent(TYPENAME, \
        &(_scenePtr->ecsWorld), \
        _handle \
    )

/*
 * Retrieves the value of the specified UNValue as
 * a number; error if the value was not an int or
 * a float
 */
#define getValueAsNumber(VALUE, ERRMSG) \
    (unIsInt(VALUE) \
        ? unAsInt(VALUE) \
        : (unIsFloat(VALUE) \
            ? unAsFloat(VALUE) \
            : ( \
                pgError(ERRMSG), \
                0 \
            ) \
        ) \
    )

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

/*
 * Returns the number of updates per second the game is
 * running at
 */
DECLARE_INT_CONST(
    updatesPerSecond,
    config_updatesPerSecond
)

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
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(_scenePtr->ecsWorld),
        &playerPosSet,
        NULL
    );
    /* if player exists, grab first one */
    if(vecsQueryItrHasEntity(&itr)){
        Position *positionPtr = vecsQueryItrGetPtr(
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
    removeComponent(VisibleMarker);
    return unBoolValue(false);
}

/* Marks the entity as rotate forward */
static UNValue setRotateSpriteForward(
    int argc,
    UNValue *argv
){
    assertArity(
        0,
        "setRotateSpriteForward expects no args"
    );
    windWorldHandleQueueSetComponent(
        RotateSpriteForwardMarker,
        &(_scenePtr->ecsWorld),
        _handle,
        NULL
    );
    return unBoolValue(false);
}

/* Unmarks the entity as rotate forward */
static UNValue removeRotateSpriteForward(
    int argc,
    UNValue *argv
){
    assertArity(
        0,
        "removeRotateSpriteForward expects no args"
    );
    removeComponent(RotateSpriteForwardMarker);
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
        "string id, int depth, vector offset, number "
        "rotation, number scale)"
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
    spriteInstr.rotation = getValueAsNumber(
        argv[3],
        "rotation should be a number; " SRC_LOCATION
    );
    spriteInstr.scale = getValueAsNumber(
        argv[4],
        "scale should be a number; " SRC_LOCATION
    );

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
    float rotation = getValueAsNumber(
        *argv,
        "set rotation expects a number; " SRC_LOCATION
    );

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
    float scale = getValueAsNumber(
        *argv,
        "set scale expects a number; " SRC_LOCATION
    );

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

    float angle = point2DAngle(entityPos, playerPos);
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

    /* get players */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(_scenePtr->ecsWorld),
        &playerSet,
        NULL
    );
    /* if player exists, grab first one */
    if(vecsQueryItrHasEntity(&itr)){
        PlayerData *playerDataPtr = vecsQueryItrGetPtr(
            PlayerData,
            &itr
        );
        return unIntValue(playerDataPtr->power);
    }
    /* error if no player exists */
    else{
        pgError(
            "failed to find player for "
            "getPlayerPower; "
            SRC_LOCATION
        );
        return unBoolValue(false);
    }
}

/*
 * Returns true if the player is currently focused,
 * false otherwise
 */
static UNValue isPlayerFocused(
    int argc,
    UNValue *argv
){
    assertArity(0, "isPlayerFocused expects no args");

    /*
     * search game commands for a focus command, since
     * the player would have to press down the focus
     * button on every frame including the current one
     * to be focused
     */
    ArrayList *gameCommandsPtr
        = &(_scenePtr->messages.gameCommands);
    for(size_t i = 0; i < gameCommandsPtr->size; ++i){
        GameCommand command = arrayListGet(GameCommand,
            gameCommandsPtr,
            i
        );
        if(command == game_focus){
            return unBoolValue(true);
        }
    }
    return unBoolValue(false);
}

/* ENTITY MUTATORS */

/* Marks the entity as collidable */
static UNValue setCollidable(int argc, UNValue *argv){
    assertArity(0, "setCollidable expects no args");
    windWorldHandleQueueSetComponent(CollidableMarker,
        &(_scenePtr->ecsWorld),
        _handle,
        NULL
    );
    return unBoolValue(false);
}

/* Unmarks the entity as collidable */
static UNValue removeCollidable(
    int argc,
    UNValue *argv
){
    assertArity(0, "removeCollidable expects no args");
    removeComponent(CollidableMarker);
    return unBoolValue(false);
}

/* Sets the entity health to the specified value */
static UNValue setHealth(int argc, UNValue *argv){
    assertArity(1, "setHealth expects int arg");
    int health = unAsInt(*argv);
    setComponent(Health, &health);
    return unBoolValue(false);
}

/* Removes the entity's health component entirely */
static UNValue removeHealth(int argc, UNValue *argv){
    assertArity(0, "removeHealth expects 0 args");
    removeComponent(Health);
    return unBoolValue(false);
}

/* Sets the entity damage to the specified value */
static UNValue setDamage(int argc, UNValue *argv){
    assertArity(1, "setDamage expects int arg");
    int damage = unAsInt(*argv);
    setComponent(Damage, &damage);
    return unBoolValue(false);
}

/* Removes the entity's damage component entirely */
static UNValue removeDamage(int argc, UNValue *argv){
    assertArity(0, "removeDamage expects 0 args");
    removeComponent(Damage);
    return unBoolValue(false);
}

/* Marks the entity as clearable */
static UNValue setClearable(int argc, UNValue *argv){
    assertArity(0, "setClearable expects no args");
    windWorldHandleQueueSetComponent(ClearableMarker,
        &(_scenePtr->ecsWorld),
        _handle,
        NULL
    );
    return unBoolValue(false);
}

/* Unmarks the entity as clearable */
static UNValue removeClearable(
    int argc,
    UNValue *argv
){
    assertArity(0, "removeClearable expects no args");
    removeComponent(ClearableMarker);
    return unBoolValue(false);
}

/* Sets the entity inbound to the specified value */
static UNValue setInbound(int argc, UNValue *argv){
    assertArity(1, "setInbound expects float arg");
    float inbound = getValueAsNumber(
        *argv,
        "set inbound expects a number; " SRC_LOCATION
    );
    setComponent(Inbound, &inbound);
    return unBoolValue(false);
}

/* Removes the entity's inbound component entirely */
static UNValue removeInbound(int argc, UNValue *argv){
    assertArity(0, "removeInbound expects 0 args");
    removeComponent(Inbound);
    return unBoolValue(false);
}

/* Sets the entity outbound to the specified value */
static UNValue setOutbound(int argc, UNValue *argv){
    assertArity(1, "setOutbound expects float arg");
    float outbound = getValueAsNumber(
        *argv,
        "set outbound expects a number; " SRC_LOCATION
    );
    setComponent(Outbound, &outbound);
    return unBoolValue(false);
}

/* Removes the entity's outbound component entirely */
static UNValue removeOutbound(int argc, UNValue *argv){
    assertArity(0, "removeOutbound expects 0 args");
    removeComponent(Outbound);
    return unBoolValue(false);
}

/* Sets the entity position to the specified value */
static UNValue setPosition(int argc, UNValue *argv){
    assertArity(1, "setPosition expects point arg");
    Point2D point = unAsPoint(*argv);
    /*
     * by using the point for both frames, essentially
     * the same as teleportation (otherwise the
     * collision system would try to calculate
     * subframe collisions)
     */
    Position position = {point, point};
    setComponent(Position, &position);
    return unBoolValue(false);
}

/* Removes the entity's position component entirely */
static UNValue removePosition(int argc, UNValue *argv){
    assertArity(0, "removePosition expects 0 args");
    removeComponent(Position);
    return unBoolValue(false);
}

/* Sets the entity velocity to the specified value */
static UNValue setVelocity(int argc, UNValue *argv){
    assertArity(1, "setVelocity expects vector arg");
    Velocity velocity = unAsVector(*argv);
    setComponent(Velocity, &velocity);
    return unBoolValue(false);
}

/* Removes the entity's velocity component entirely */
static UNValue removeVelocity(int argc, UNValue *argv){
    assertArity(0, "removeVelocity expects 0 args");
    removeComponent(Velocity);
    return unBoolValue(false);
}

/* Sets the entity speed to the specified value */
static UNValue setSpeed(int argc, UNValue *argv){
    assertArity(1, "setSpeed expects float arg");
    float speed = getValueAsNumber(
        *argv,
        "set speed expects a number; " SRC_LOCATION
    );

    Velocity *velocityPtr = NULL;
    fillComponentPtr(Velocity,
        velocityPtr,
        "cannot get entity velocity when entity lacks "
        "velocity component; " SRC_LOCATION
    );

    velocityPtr->magnitude = speed;

    return unBoolValue(false);
}

/* Sets the entity angle to the specified value */
static UNValue setAngle(int argc, UNValue *argv){
    assertArity(1, "setAngle expects float arg");
    float angle = getValueAsNumber(
        *argv,
        "set angle expects a number; " SRC_LOCATION
    );

    Velocity *velocityPtr = NULL;
    fillComponentPtr(Velocity,
        velocityPtr,
        "cannot get entity velocity when entity lacks "
        "velocity component; " SRC_LOCATION
    );

    velocityPtr->angle = angle;

    return unBoolValue(false);
}

/*
 * Sets the entity sprite spin to the specified value
 */
static UNValue setSpin(int argc, UNValue *argv){
    assertArity(1, "setSpin expects float arg");
    SpriteSpin spin = getValueAsNumber(
        *argv,
        "set spin expects a number; " SRC_LOCATION
    );
    setComponent(SpriteSpin, &spin);
    return unBoolValue(false);
}

/*
 * Removes the entity's sprite spin component entirely
 */
static UNValue removeSpin(int argc, UNValue *argv){
    assertArity(0, "removeSpin expects 0 args");
    removeComponent(SpriteSpin);
    return unBoolValue(false);
}

/* Flags the entity as dead */
static UNValue die(int argc, UNValue *argv){
    assertArity(0, "die expects 0 args");
    arrayListPushBack(VecsEntity,
        &(_scenePtr->messages.deaths),
        _handle
    );
    return unBoolValue(false);
}

/*
 * Removes the entity entirely without going through
 * the death system
 */
static UNValue removeEntity(int argc, UNValue *argv){
    assertArity(0, "removeEntity expects 0 args");
    windWorldHandleQueueRemoveEntity(
        &(_scenePtr->ecsWorld),
        _handle
    );
    return unBoolValue(false);
}

/* SCRIPTING */

/*
 * Returns true if the entity has a script in the
 * specified VM slot, false otherwise
 */
static UNValue hasScript(int argc, UNValue *argv){
    assertArity(1, "usage: hasScript(slot)");

    /* get the slot */
    int slot = unAsInt(*argv);
    if(slot < 1 || slot > 4){
        pgError("invalid VM slot; " SRC_LOCATION);
    }
    
    /* get the script component */
    Scripts *scriptsPtr = NULL;
    fillComponentPtr(Scripts,
        scriptsPtr,
        "unexpectedly unable to get Scripts ptr for "
        "entity that is currently running a script; "
        SRC_LOCATION
    );

    switch(slot){
        case 1:
            return unBoolValue(scriptsPtr->vm1);
        case 2:
            return unBoolValue(scriptsPtr->vm2);
        case 3:
            return unBoolValue(scriptsPtr->vm3);
        case 4:
            return unBoolValue(scriptsPtr->vm4);
        default:
            pgError(
                "unexpected default vm slot; "
                SRC_LOCATION
            );
            break;
    }

    /* should never be reached */
    return unBoolValue(false);
}

/*
 * Adds a script to the specified VM slot
 * (String scriptId, int slot), returns
 * true if successful, false otherwise; Spawns go
 * in slots 3 and 4
 */
static UNValue addScript(int argc, UNValue *argv){
    assertArity(2, "usage: addScript(scriptId, slot)");

    /* get the script */
    String *stringPtr
        = &(unObjectAsString(argv[0])->string);
    UNObjectFunc *scriptPtr = resourcesGetScript(
        _gamePtr->resourcesPtr,
        stringPtr
    );
    if(!scriptPtr){
        pgWarning(stringPtr->_ptr);
        pgError("failed to get script; " SRC_LOCATION);
    }

    /* get the slot */
    int slot = unAsInt(argv[1]);
    if(slot < 1 || slot > 4){
        pgError("invalid VM slot; " SRC_LOCATION);
    }
    
    /* get the script component */
    Scripts *scriptsPtr = NULL;
    fillComponentPtr(Scripts,
        scriptsPtr,
        "unexpectedly unable to get Scripts ptr for "
        "entity that is currently running a script; "
        SRC_LOCATION
    );

    /*
     * try to load script; return true if the
     * requested slot was availible, false otherwise
     */
    #define loadScript(SLOT) \
        do{ \
            if(!scriptsPtr->vm##SLOT){ \
                scriptsPtr->vm##SLOT \
                    = vmPoolRequest(); \
                unVirtualMachineLoad( \
                    scriptsPtr->vm##SLOT, \
                    scriptPtr \
                ); \
                return unBoolValue(true); \
            } \
            else{ \
                return unBoolValue(false); \
            } \
        } while(false)

    switch(slot){
        case 1:
            loadScript(1);
            break;
        case 2:
            loadScript(2);
            break;
        case 3:
            loadScript(3);
            break;
        case 4:
            loadScript(4);
            break;
        default:
            pgError(
                "unexpected default vm slot; "
                SRC_LOCATION
            );
            break;
    }

    /* should never be reached */
    return unBoolValue(false);

    #undef loadScript
}

/*
 * Removes the script in the specified VM slot, returns
 * true if successful, false if no such script existed;
 * UB IF ATTEMPTING TO REMOVE SELF
 */
static UNValue removeScript(int argc, UNValue *argv){
    assertArity(1, "usage: removeScript(slot)");

    /* get the slot */
    int slot = unAsInt(*argv);
    if(slot < 1 || slot > 4){
        pgError("invalid VM slot; " SRC_LOCATION);
    }
    
    /* get the script component */
    Scripts *scriptsPtr = NULL;
    fillComponentPtr(Scripts,
        scriptsPtr,
        "unexpectedly unable to get Scripts ptr for "
        "entity that is currently running a script; "
        SRC_LOCATION
    );

    /*
     * try to remove script; return true if the
     * requested slot was occupied, false otherwise
     */
    #define _removeScriptAndReturn(SLOT) \
        do{ \
            if(scriptsPtr->vm##SLOT){ \
                vmPoolReclaim(scriptsPtr->vm##SLOT); \
                scriptsPtr->vm##SLOT = NULL; \
                return unBoolValue(true); \
            } \
            else{ \
                return unBoolValue(false); \
            } \
        } while(false)

    switch(slot){
        case 1:
            _removeScriptAndReturn(1);
            break;
        case 2:
            _removeScriptAndReturn(2);
            break;
        case 3:
            _removeScriptAndReturn(3);
            break;
        case 4:
            _removeScriptAndReturn(4);
            break;
        default:
            pgError(
                "unexpected default vm slot; "
                SRC_LOCATION
            );
            break;
    }

    /* should never be reached */
    return unBoolValue(false);

    #undef _removeScriptAndReturn
}

/*
 * Removes spawns from the entity, i.e. scripts running
 * in VM slots 3 and 4; UB IF ATTEMPTING TO REMOVE
 * SELF
 */
static UNValue removeSpawns(int argc, UNValue *argv){
    assertArity(0, "removeSpawns expects 0 args");

    /* get the script component */
    Scripts *scriptsPtr = NULL;
    fillComponentPtr(Scripts,
        scriptsPtr,
        "unexpectedly unable to get Scripts ptr for "
        "entity that is currently running a script; "
        SRC_LOCATION
    );

    /* try to remove script */
    #define _removeScript(SLOT) \
        do{ \
            if(scriptsPtr->vm##SLOT){ \
                vmPoolReclaim(scriptsPtr->vm##SLOT); \
                scriptsPtr->vm##SLOT = NULL; \
            } \
        } while(false)

    _removeScript(3);
    _removeScript(4);

    return unBoolValue(false);

    /*
     * undefine macro _removeScript which was defined
     * in the body of removeScript
     */
    #undef _removeScript
}

/*
 * Adds the specified death script to the entity 
 * in the specified slot (String scriptId, int slot);
 * returns true if successful, false otherwise - note
 * that just like with normal scripts, slots 3 and 4
 * are typically used for spawns
 */
static UNValue addDeathScript(int argc, UNValue *argv){
    assertArity(
        2,
        "usage: addDeathScript(scriptId, slot)"
    );

    /* get the script Id */
    String *stringPtr
        = &(unObjectAsString(argv[0])->string);

    /* get the slot */
    int slot = unAsInt(argv[1]);
    if(slot < 1 || slot > 4){
        pgError(
            "invalid death script slot; "
            SRC_LOCATION
        );
    }
    
    /* get the death script component if present */
    if(vecsWorldEntityContainsComponent(DeathScripts,
        &(_scenePtr->ecsWorld),
        _handle
    )){
        DeathScripts *deathScriptsPtr = NULL;
        fillComponentPtr(DeathScripts,
            deathScriptsPtr,
            "error: failed to get death scripts; "
            SRC_LOCATION
        );

        /*
         * try to load death script; return true if the
         * requested slot was availible, false
         * otherwise
         */
        #define loadDeathScript(SLOT) \
            do{ \
                if(!deathScriptsPtr->scriptId##SLOT \
                    ._ptr \
                ){ \
                    deathScriptsPtr->scriptId##SLOT \
                        = stringCopy(stringPtr); \
                    return unBoolValue(true); \
                } \
                else{ \
                    return unBoolValue(false); \
                } \
            } while(false)

        switch(slot){
            case 1:
                loadDeathScript(1);
                break;
            case 2:
                loadDeathScript(2);
                break;
            case 3:
                loadDeathScript(3);
                break;
            case 4:
                loadDeathScript(4);
                break;
            default:
                pgError(
                    "unexpected default death script "
                    "slot; "
                    SRC_LOCATION
                );
                break;
        }

        #undef loadDeathScript
    }
    /*
     * otherwise prepare a new component to add and
     * queue the operation
     */
    else{
        DeathScripts deathScripts = {0};
        switch(slot){
            case 1:
                deathScripts.scriptId1
                    = stringCopy(stringPtr);
                break;
            case 2:
                deathScripts.scriptId2
                    = stringCopy(stringPtr);
                break;
            case 3:
                deathScripts.scriptId3
                    = stringCopy(stringPtr);
                break;
            case 4:
                deathScripts.scriptId4
                    = stringCopy(stringPtr);
                break;
            default:
                pgError(
                    "unexpected default death script "
                    "slot; "
                    SRC_LOCATION
                );
                break;
        }
        vecsWorldEntityQueueAddComponent(DeathScripts,
            &(_scenePtr->ecsWorld),
            _handle,
            &deathScripts
        );
        return unBoolValue(true);
    }

    /* should never be reached */
    return unBoolValue(false);
}

/*
 * Removes the specified death script from the entity
 * of the specified slot (int slot); returns true if
 * successful, false otherwise
 */
static UNValue removeDeathScript(
    int argc,
    UNValue *argv
){
    assertArity(1, "usage: removeDeathScript(slot)");

    int slot = unAsInt(*argv);
    if(slot < 1 || slot > 4){
        pgError(
            "invalid death script slot; "
            SRC_LOCATION
        );
    }

    /*
     * if death script component not present, return
     * false
     */
    if(!vecsWorldEntityContainsComponent(DeathScripts,
        &(_scenePtr->ecsWorld),
        _handle
    )){
        return unBoolValue(false);
    }

    DeathScripts *deathScriptsPtr = NULL;
    fillComponentPtr(DeathScripts,
        deathScriptsPtr,
        "error: failed to get death scripts; "
        SRC_LOCATION
    );

    /*
     * try to remove death script; return true if the
     * requested slot was present, false otherwise
     */
    #define removeDeathScript(SLOT) \
        do{ \
            if(deathScriptsPtr->scriptId##SLOT \
                ._ptr \
            ){ \
                stringFree( \
                    &(deathScriptsPtr \
                        ->scriptId##SLOT) \
                ); \
                return unBoolValue(true); \
            } \
            else{ \
                return unBoolValue(false); \
            } \
        } while(false)

    switch(slot){
        case 1:
            removeDeathScript(1);
            break;
        case 2:
            removeDeathScript(2);
            break;
        case 3:
            removeDeathScript(3);
            break;
        case 4:
            removeDeathScript(4);
            break;
        default:
            pgError(
                "unexpected default death script "
                "slot; "
                SRC_LOCATION
            );
            break;
    }

    return unBoolValue(false);

    #undef removeDeathScript
}

/* MATH */

/*
 * Flips a float angle or a polar vector about the x
 * axis
 */
static UNValue flipX(int argc, UNValue *argv){
    assertArity(
        1,
        "flipX expects float or vector arg"
    );

    UNValue value = *argv;
    if(unIsFloat(value)){
        float floatValue = unAsFloat(value);
        return unFloatValue(angleFlipX(floatValue));
    }
    else if(unIsVector(value)){
        Polar vectorValue = unAsVector(value);
        vectorValue.angle
            = angleFlipX(vectorValue.angle);
        return unVectorValue(vectorValue);
    }
    else{
        pgError(
            "flipX expects float or vector; "
            SRC_LOCATION
        );
        return unBoolValue(false);
    }
}

/*
 * Flips a float angle or a polar vector about the x
 * axis
 */
static UNValue flipY(int argc, UNValue *argv){
    assertArity(
        1,
        "flipY expects float or vector arg"
    );

    UNValue value = *argv;
    if(unIsFloat(value)){
        float floatValue = unAsFloat(value);
        return unFloatValue(angleFlipY(floatValue));
    }
    else if(unIsVector(value)){
        Polar vectorValue = unAsVector(value);
        vectorValue.angle
            = angleFlipY(vectorValue.angle);
        return unVectorValue(vectorValue);
    }
    else{
        pgError(
            "flipY expects float or vector; "
            SRC_LOCATION
        );
        return unBoolValue(false);
    }
}

/*
 * Computes an exponent (number base, number exp);
 * if both arguments are ints, the result will be an
 * int, otherwise float (if the exponent is negative,
 * the result will also be a float)
 */
static UNValue _pow(int argc, UNValue *argv){
    assertArity(
        2,
        "usage: pow(base, exponent)"
    );

    UNValue baseValue = argv[0];
    UNValue expValue = argv[1];

    /* if both ints, we may return an int */
    if(unIsInt(baseValue) && unIsInt(expValue)){
        int base = unAsInt(baseValue);
        int exp = unAsInt(expValue);
        if(exp == 0){
            return unIntValue(1);
        }
        if(exp < 0){
            return unFloatValue(powf(base, exp));
        }
        /* calculate integer power */
        int result = 1;
        while(true){
            if(exp & 1){
                result *= base;
            }
            exp >>= 1;
            if(!exp){
                break;
            }
            base *= base;
        }
        return unIntValue(result);
    }

    /* if not both ints, return a float */
    float base = getValueAsNumber(
        baseValue,
        "base of pow() function should be number; "
        SRC_LOCATION
    );
    float exp = getValueAsNumber(
        expValue,
        "exp of pow() function should be number; "
        SRC_LOCATION
    );
    return unFloatValue(powf(base, exp));
}

/*
 * Computes the sin function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue _sin(int argc, UNValue *argv){
    assertArity(1, "sin expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of sin() function should be number; "
        SRC_LOCATION
    );
    return unFloatValue(sinf(arg));
}

/*
 * Computes the cos function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue _cos(int argc, UNValue *argv){
    assertArity(1, "cos expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of cos() function should be number; "
        SRC_LOCATION
    );
    return unFloatValue(cosf(arg));
}

/*
 * Computes the tan function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue _tan(int argc, UNValue *argv){
    assertArity(1, "tan expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of tan() function should be number; "
        SRC_LOCATION
    );
    return unFloatValue(tanf(arg));
}

/*
 * Computes the sec function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue _sec(int argc, UNValue *argv){
    assertArity(1, "sec expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of sec() function should be number; "
        SRC_LOCATION
    );
    float cos = cosf(arg);
    if(cos == 0.0f){
        pgError("cannot compute sec; divide by 0");
    }
    return unFloatValue(1.0f/cos);
}

/*
 * Computes the csc function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue _csc(int argc, UNValue *argv){
    assertArity(1, "csc expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of csc() function should be number; "
        SRC_LOCATION
    );
    float sin = sinf(arg);
    if(sin == 0.0f){
        pgError("cannot compute csc; divide by 0");
    }
    return unFloatValue(1.0f/sin);
}

/*
 * Computes the cot function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue _cot(int argc, UNValue *argv){
    assertArity(1, "cot expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of cot() function should be number; "
        SRC_LOCATION
    );
    float tan = tanf(arg);
    if(tan == 0.0f){
        pgError("cannot compute cot; divide by 0");
    }
    return unFloatValue(1.0f/tan);
}

/*
 * Computes the arcsin function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue arcsin(int argc, UNValue *argv){
    assertArity(1, "arcsin expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of arcsin() function should be number; "
        SRC_LOCATION
    );
    return unFloatValue(asinf(arg));
}

/*
 * Computes the arccos function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue arccos(int argc, UNValue *argv){
    assertArity(1, "arccos expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of arccos() function should be number; "
        SRC_LOCATION
    );
    return unFloatValue(acosf(arg));
}

/*
 * Computes the arctan function; takes either int or
 * float and returns a float, works in radians
 */
static UNValue arctan(int argc, UNValue *argv){
    assertArity(1, "arctan expects 1 number arg");

    float arg = getValueAsNumber(
        *argv,
        "arg of arctan() function should be number; "
        SRC_LOCATION
    );
    return unFloatValue(atanf(arg));
}

/*
 * Returns the maximum of any number of numbers passed
 * in; error if 0 numbers passed in
 */
static UNValue _max(int argc, UNValue *argv){
    static const char* notNumberErrMsg
        = "args of max() should all be numbers";
    if(argc <= 0){
        pgError("max should have at least 1 arg");
    }
    float maxValue = getValueAsNumber(
        *argv,
        notNumberErrMsg
    );
    int maxIndex = 0;
    for(int i = 1; i < argc; ++i){
        float testValue = getValueAsNumber(
            argv[i],
            notNumberErrMsg
        );
        if(testValue > maxValue){
            maxValue = testValue;
            maxIndex = i;
        }
    }
    return argv[maxIndex];
}

/*
 * Returns the minimum of any number of numbers passed
 * in; error if 0 numbers passed in
 */
static UNValue _min(int argc, UNValue *argv){
    static const char* notNumberErrMsg
        = "args of min() should all be numbers";
    if(argc <= 0){
        pgError("min should have at least 1 arg");
    }
    float minValue = getValueAsNumber(
        *argv,
        notNumberErrMsg
    );
    int minIndex = 0;
    for(int i = 1; i < argc; ++i){
        float testValue = getValueAsNumber(
            argv[i],
            notNumberErrMsg
        );
        if(testValue < minValue){
            minValue = testValue;
            minIndex = i;
        }
    }
    return argv[minIndex];
}

/*
 * Returns the smaller angle difference from the first
 * number arg to the second as a float
 */
static UNValue smallerAngleDiff(
    int argc,
    UNValue *argv
){
    static const char* notNumberErrMsg
        = "args of smallerAngleDiff() should all be "
            "numbers";
    assertArity(
        2,
        "usage: smallerAngleDiff(from, to)"
    );

    float from = getValueAsNumber(
        argv[0],
        notNumberErrMsg
    );
    float to = getValueAsNumber(
        argv[1],
        notNumberErrMsg
    );
    return unFloatValue(
        angleSmallerDifference(from, to)
    );
}

/*
 * Returns the larger angle difference from the first
 * number arg to the second as a float
 */
static UNValue largerAngleDiff(
    int argc,
    UNValue *argv
){
    static const char* notNumberErrMsg
        = "args of largerAngleDiff() should all be "
            "numbers";
    assertArity(
        2,
        "usage: largerAngleDiff(from, to)"
    );

    float from = getValueAsNumber(
        argv[0],
        notNumberErrMsg
    );
    float to = getValueAsNumber(
        argv[1],
        notNumberErrMsg
    );
    return unFloatValue(
        angleLargerDifference(from, to)
    );
}

/*
 * Returns the absolute value of the argument as an
 * integer or a float depending on its type
 */
static UNValue _abs(int argc, UNValue *argv){
    assertArity(1, "abs expects 1 number arg");

    UNValue value = *argv;
    if(unIsInt(value)){
        return unIntValue(abs(unAsInt(value)));
    }
    else if(unIsFloat(value)){
        return unFloatValue(fabsf(unAsFloat(value)));
    }
    else{
        pgError(
            "arg of abs() should be number; "
            SRC_LOCATION
        );
        return unBoolValue(false);
    }
}

/* Returns the distance between two points */
static UNValue pointDist(int argc, UNValue *argv){
    assertArity(2, "pointDist expects 2 point args");

    Point2D pointA = unAsPoint(argv[0]);
    Point2D pointB = unAsPoint(argv[1]);

    return unFloatValue(
        point2DDistance(pointA, pointB)
    );
}

/*
 * Returns the angle from point a to point b 
 * in degrees
 */
static UNValue pointAngle(int argc, UNValue *argv){
    assertArity(2, "pointAngle expects 2 point args");

    Point2D pointA = unAsPoint(argv[0]);
    Point2D pointB = unAsPoint(argv[1]);

    return unFloatValue(point2DAngle(pointA, pointB));
}

/*
 * Converts the given number from degrees to radians
 * and returns the result as a float
 */
static UNValue _toRadians(int argc, UNValue *argv){
    assertArity(1, "toRadians expects 1 number arg");
    float degrees = getValueAsNumber(
        *argv,
        "toRadians expects a number; "
        SRC_LOCATION
    );
    return unFloatValue(toRadians(degrees));
}

/*
 * Converts the given number from radians to degrees
 * and returns the result as a float
 */
static UNValue _toDegrees(int argc, UNValue *argv){
    assertArity(1, "toDegrees expects 1 number arg");
    float radians = getValueAsNumber(
        *argv,
        "toDegrees expects a number; "
        SRC_LOCATION
    );
    return unFloatValue(toDegrees(radians));
}

/*
 * Returns a random number between
 * (number min, number max) inclusive;
 * calculates a random int if both numbers are ints,
 * otherwise calculates a random float
 */
static UNValue _random(int argc, UNValue *argv){
    assertArity(
        2,
        "usage: random(number min, number max)"
    );

    ZMT *prngPtr = &(_scenePtr->messages.prng);
    
    UNValue minValue = argv[0];
    UNValue maxValue = argv[1];

    /* if both ints, calculate random int */
    if(unIsInt(minValue) && unIsInt(maxValue)){
        int min = unAsInt(minValue);
        int max = unAsInt(maxValue);
        if(max < min){
            pgError("max < min; " SRC_LOCATION);
        }
        int result = zmtIntDie(prngPtr, min, max);
        return unIntValue(result);
    }

    /* if not both ints, return a float */
    float min = getValueAsNumber(
        minValue,
        "min of random() function should be number; "
        SRC_LOCATION
    );
    float max = getValueAsNumber(
        maxValue,
        "max of random() function should be number; "
        SRC_LOCATION
    );
    if(max < min){
        pgError("max < min; " SRC_LOCATION);
    }
    float result = zmtFloatDie(prngPtr, min, max);
    return unFloatValue(result);
}

/*
 * Generates a random bool based on the provided
 * chance [0.0, 1.0]; error if the chance is out of
 * bounds
 */
static UNValue chance(int argc, UNValue *argv){
    assertArity(1, "chance() expects 1 float arg");

    ZMT *prngPtr = &(_scenePtr->messages.prng);

    assertTrue(
        unIsFloat(*argv),
        "chance expects a float; " SRC_LOCATION
    );
    float chance = unAsFloat(*argv);

    if(chance < 0.0f || chance > 1.0f){
        pgError("chance out of range; " SRC_LOCATION);
    }

    if(chance == 0.0f){
        return unBoolValue(false);
    }
    if(chance == 1.0f){
        return unBoolValue(true);
    }

    /* random float in range [0.0, 1.0] */
    float rand = zmtRandFloat(prngPtr);
    
    return unBoolValue(rand < chance);
}

/*
 * Returns true if the specified point is outside the
 * given bounds, false otherwise; (point, bound)
 */
static UNValue isPointOutOfBounds(
    int argc,
    UNValue *argv
){
    assertArity(
        2,
        "usage: isPointOutOfBounds(point, bound)"
    );
    Point2D point = unAsPoint(argv[0]);
    float bound = getValueAsNumber(
        argv[1],
        "bound should be a number; " SRC_LOCATION
    );
    return unBoolValue(isOutOfBounds(point, bound));
}

/* SCENE SIGNALING */

/*
 * Adds a life to the player; calling multiple times
 * on the same tick will have an extra effect
 */
static UNValue addLife(int argc, UNValue *argv){
    assertArity(0, "addLife expects 0 args");
    ++(_scenePtr->messages.livesToAdd);
    return unBoolValue(false);
}

/*
 * Adds a bomb to the player; calling multiple times
 * on the same tick will have an extra effect
 */
static UNValue addBomb(int argc, UNValue *argv){
    assertArity(0, "addBomb expects 0 args");
    ++(_scenePtr->messages.bombsToAdd);
    return unBoolValue(false);
}

/*
 * Flags a boss death; flagging multiple times on the
 * same tick has no extra effect
 */
static UNValue flagBossDeath(int argc, UNValue *argv){
    assertArity(0, "flagBossDeath expects 0 args");
    _scenePtr->messages.bossDeathFlag = true;
    return unBoolValue(false);
}

/*
 * Flags a bullet clear; flagging multiple times on the
 * same tick has no extra effect
 */
static UNValue flagBulletClear(
    int argc,
    UNValue *argv
){
    assertArity(0, "flagBulletClear expects 0 args");
    _scenePtr->messages.clearFlag = true;
    return unBoolValue(false);
}

/*
 * Flags a stage win; flagging multiple times on the
 * same tick has no extra effect
 */
static UNValue flagWin(int argc, UNValue *argv){
    assertArity(0, "flagWin expects 0 args");
    _scenePtr->messages.winFlag = true;
    return unBoolValue(false);
}

/*
 * Ends the current stage and either advances to the
 * next stage, goes back to the menu, or enters the
 * credits scene depending on the game state
 */
static UNValue endStage(int argc, UNValue *argv){
    assertArity(0, "endStage expects 0 args");
    
    GameState *gameStatePtr
        = &(_gamePtr->messages.gameState);

    SceneId backTo = -1;
    switch(gameStatePtr->gameMode){
        case game_story:
            backTo = scene_main;
            break;
        case game_practice:
            backTo = scene_stage;
            break;
        default:
            pgError(
                "unexpected default game mode; "
                SRC_LOCATION
            );
            break;
    }

    _gamePtr->messages.sceneExitToId = backTo;

    /* in story mode, advance stage or go to credits */
    if(gameStatePtr->gameMode == game_story){
        /* if stage not final, advance */
        if(gameStatePtr->stage < 4){
            ++(gameStatePtr->stage);

            /* push new game and loading screen */
            arrayListPushBack(SceneId,
                &(_gamePtr->messages.sceneEntryList),
                scene_game
            );
            arrayListPushBack(SceneId,
                &(_gamePtr->messages.sceneEntryList),
                scene_loading
            );

            /* save player data */
            VecsQueryItr itr
                = vecsWorldRequestQueryItr(
                    &(_scenePtr->ecsWorld),
                    &playerSet,
                    NULL
                );
            /* if player exists, grab first one */
            if(vecsQueryItrHasEntity(&itr)){
                PlayerData *playerDataPtr
                    = vecsQueryItrGetPtr(
                        PlayerData,
                        &itr
                    );
                _gamePtr->messages.playerData.data
                    = *playerDataPtr;
                _gamePtr->messages.playerData.isPresent
                    = true;
            }
            /* error if no player exists */
            else{
                pgError(
                    "failed to find player for "
                    "endStage; "
                    SRC_LOCATION
                );
            }
        }
        /* otherwise, if final stage, go to credits */
        else{
            arrayListPushBack(SceneId,
                &(_gamePtr->messages.sceneEntryList),
                scene_credits
            );
            
            /* start playback of track 10 */
            String *trackIdPtr = &(_gamePtr->messages
                .startMusicString);
            stringClear(trackIdPtr);
            stringAppendC(trackIdPtr, "10");
        }
    }
    /*
     * if practice mode, start menu track since we
     * are going back to the menu
     */
    else if(gameStatePtr->gameMode == game_practice){
        String *trackIdPtr = &(_gamePtr->messages
            .startMusicString);
        stringClear(trackIdPtr);
        stringAppendC(trackIdPtr, "01");
    }
    else{
        pgError(
            "Unexpected gamemode; not story or "
            "practice; " SRC_LOCATION
        );
    }

    return unBoolValue(false);
}

/*
 * Displays the dialogue with the specified string Id
 */
static UNValue startDialogue(int argc, UNValue *argv){
    assertArity(1, "startDialogue expects string arg");
    String *stringPtr
        = &(unObjectAsString(*argv)->string);
    
    arrayListPushBack(SceneId,
        &(_gamePtr->messages.sceneEntryList),
        scene_dialogue
    );
    stringCopyInto(
        &(_gamePtr->messages.startDialogueString),
        stringPtr
    );

    return unBoolValue(false);
}

/*
 * if need more flags, can refactor to take int
 * arg and store an array of flags
 */

/*
 * Sets the user flag 1 such that other entities will
 * see it next tick
 */
static UNValue flagUser1(int argc, UNValue *argv){
    assertArity(0, "flagUser1 expects no args");

    _scenePtr->messages.userFlag1 |= 2;
    return unBoolValue(false);
}

/*
 * Returns true if the user flag 1 was set last tick,
 * false otherwise
 */
static UNValue isFlagged1(int argc, UNValue *argv){
    assertArity(0, "isFlagged1 expects no args");

    return unBoolValue(
        _scenePtr->messages.userFlag1 & 1
    );
}

/* SPAWNING */

/*
 * Queues a new entity to be spawned:
 * spawn(
 *      String prototypeId,
 *      Point pos,
 *      Vector vel,
 *      int depthOffset,
 *      OPTIONAL string scriptId1, 2, 3, 4
 * )
 */
static UNValue spawn(int argc, UNValue *argv){
    static const char *usageMsg
        = "spawn(String prototypeId, Point position, "
            "Vector velocity, int depthOffset, "
            "OPTIONAL String scriptId1, 2, 3, 4)";
    if(argc < 4 || argc > 8){
        pgError(usageMsg);
    }

    String *prototypeIdPtr
        = &(unObjectAsString(argv[0])->string);
    Point2D pos = unAsPoint(argv[1]);
    Polar vel = unAsVector(argv[2]);
    int depthOffset = unAsInt(argv[3]);

    String *scriptId1Ptr = NULL;
    String *scriptId2Ptr = NULL;
    String *scriptId3Ptr = NULL;
    String *scriptId4Ptr = NULL;
    switch(argc){
        case 8:
            scriptId4Ptr
                = &(unObjectAsString(argv[7])->string);
            /* fallthrough */
        case 7:
            scriptId3Ptr
                = &(unObjectAsString(argv[6])->string);
            /* fallthrough */
        case 6:
            scriptId2Ptr
                = &(unObjectAsString(argv[5])->string);
            /* fallthrough */
        case 5:
            scriptId1Ptr
                = &(unObjectAsString(argv[4])->string);
            /* fallthrough */
        default:
            break;
            /* do nothing */
    }

    declareList(componentList, 10);

    /*
     * load the prototype, passing depth offset to the
     * sprite instruction
     */
    applyPrototype(
        _gamePtr,
        _scenePtr,
        prototypeIdPtr,
        &componentList,
        depthOffset
    );

    addPosition(&componentList, pos);
    addVelocity(&componentList, vel);

    /* add scripts if needed */
    if(argc > 4){
        Scripts scripts = {0};

        #define loadScriptIfStringIdValid(SLOT) \
            do{ \
                if((scriptId##SLOT##Ptr) \
                    && !stringIsEmpty( \
                        scriptId##SLOT##Ptr \
                    ) \
                ){ \
                    scripts.vm##SLOT \
                        = vmPoolRequest(); \
                    UNObjectFunc *scriptPtr \
                        = resourcesGetScript( \
                            _gamePtr->resourcesPtr, \
                            scriptId##SLOT##Ptr \
                        ); \
                    unVirtualMachineLoad( \
                        scripts.vm##SLOT, \
                        scriptPtr \
                    ); \
                } \
            } while(false)

        loadScriptIfStringIdValid(1);
        loadScriptIfStringIdValid(2);
        loadScriptIfStringIdValid(3);
        loadScriptIfStringIdValid(4);

        #undef loadScriptIfStringIdValid

        /*
         * only add scripts if at least one VM is
         * not null
         */
        if(scripts.vm1 || scripts.vm2 
            || scripts.vm3 || scripts.vm4
        ){
            addScripts(&componentList, scripts);
        }
    }

    queueAddEntityAndFreeList(
        &componentList,
        _scenePtr
    );

    return unBoolValue(false);
}

#undef assertArity
#undef fillComponentPtr
#undef setComponent
#undef removeComponent
#undef getValueAsNumber

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
        bitsetFree(&playerSet);
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
        bitsetSet(&playerPosSet, PlayerDataId);
        bitsetSet(&playerPosSet, PositionId);

        playerSet = bitsetMake(numComponents);
        bitsetSet(&playerSet, PlayerDataId);

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
        addNativeFunc(updatesPerSecond);
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
        addNativeFunc(getPlayerPos);

        /* entity graphics */
        addNativeFunc(setVisible);
        addNativeFunc(removeVisible);
        addNativeFunc(setRotateSpriteForward);
        addNativeFunc(removeRotateSpriteForward);
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
        addNativeFunc(getPlayerPower)
        addNativeFunc(isPlayerFocused);

        /* entity mutators */
        addNativeFunc(setCollidable);
        addNativeFunc(removeCollidable);
        addNativeFunc(setHealth);
        addNativeFunc(removeHealth);
        addNativeFunc(setDamage);
        addNativeFunc(removeDamage);
        addNativeFunc(setClearable);
        addNativeFunc(removeClearable);
        addNativeFunc(setInbound);
        addNativeFunc(removeInbound);
        addNativeFunc(setOutbound);
        addNativeFunc(removeOutbound);
        addNativeFunc(setPosition);
        addNativeFunc(removePosition);
        addNativeFunc(setVelocity);
        addNativeFunc(removeVelocity);
        addNativeFunc(setSpeed);
        addNativeFunc(setAngle);
        addNativeFunc(setSpin);
        addNativeFunc(removeSpin);
        addNativeFunc(die);
        addNativeFunc(removeEntity);

        /* scripting */
        addNativeFunc(hasScript);
        addNativeFunc(addScript);
        addNativeFunc(removeScript);
        addNativeFunc(removeSpawns);
        addNativeFunc(addDeathScript);
        addNativeFunc(removeDeathScript);

        /* math */
        addNativeFunc(flipX);
        addNativeFunc(flipY);
        addNativeFuncWithName(_pow, pow);
        addNativeFuncWithName(_sin, sin);
        addNativeFuncWithName(_cos, cos);
        addNativeFuncWithName(_tan, tan);
        addNativeFuncWithName(_sec, sec);
        addNativeFuncWithName(_csc, csc);
        addNativeFuncWithName(_cot, cot);
        addNativeFunc(arcsin);
        addNativeFunc(arccos);
        addNativeFunc(arctan);
        addNativeFuncWithName(_max, max);
        addNativeFuncWithName(_min, min);
        addNativeFunc(smallerAngleDiff);
        addNativeFunc(largerAngleDiff);
        addNativeFuncWithName(_abs, abs);
        addNativeFunc(pointDist);
        addNativeFunc(pointAngle);
        addNativeFuncWithName(_toRadians, toRadians);
        addNativeFuncWithName(_toDegrees, toDegrees);
        addNativeFuncWithName(_random, random);
        addNativeFunc(chance);
        addNativeFunc(isPointOutOfBounds);

        /* scene signaling */
        addNativeFunc(addLife);
        addNativeFunc(addBomb);
        addNativeFunc(flagBossDeath);
        addNativeFunc(flagBulletClear);
        addNativeFunc(flagWin);
        addNativeFunc(endStage);
        addNativeFunc(startDialogue);
        addNativeFunc(flagUser1);
        addNativeFunc(isFlagged1);

        /* spawning */
        addNativeFunc(spawn);
        
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
void setEntityForNativeFuncs(VecsEntity handle){
    _handle = handle;
}