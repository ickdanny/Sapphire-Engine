#ifndef ENTITYBUILDER_H
#define ENTITYBUILDER_H

#include "WindECS.h"

/*
 * the following functions are utility functions for
 * building up component lists; note that WindECS will
 * error for duplicate components
 */

/*
 * Declares a component list with the specified name
 * in the scope of the macro
 */
#define declareList(LISTNAME, INITCAPACITY) \
    ArrayList LISTNAME = arrayListMake( \
        WindComponentDataPair, \
        (INITCAPACITY) \
    )

/*
 * Adds a component to the specified component list
 * by making a shallow copy of the component on the
 * heap
 */
#define addComponent(LISTPTR, TYPENAME, COMPONENT) \
    do{ \
        WindComponentDataPair dataPair = {0}; \
        dataPair.componentID = TYPENAME##ID; \
        TYPENAME *componentCopyPtr \
            = pgAlloc(1, sizeof(TYPENAME)); \
        *componentCopyPtr = (COMPONENT); \
        dataPair.componentPtr = componentCopyPtr; \
        arrayListPushBack(WindComponentDataPair, \
            (LISTPTR), \
            dataPair \
        ); \
    } while(false)

/* Adds a marker to the specified component list */
#define addMarker(LISTPTR, MARKERID) \
    do{ \
        WindComponentDataPair dataPair = {MARKERID}; \
        arrayListPushBack(WindComponentDataPair, \
            (LISTPTR), \
            dataPair \
        ); \
    } while(false)

/*
 * Adds a position holding the specified Point2D to
 * the specified component list
 */
#define addPosition(LISTPTR, INITPOS) \
    addComponent( \
        LISTPTR, \
        Position, \
        ((Position){(INITPOS), (INITPOS)}) \
    )

/*
 * Adds the specified velocity to the given component
 * list
 */
#define addVelocity(LISTPTR, VELOCITY) \
    addComponent(LISTPTR, Velocity, VELOCITY)

/*
 * Adds a visible marker to the specified component
 * list
 */
#define addVisible(LISTPTR) \
    addMarker(LISTPTR, VisibleMarkerID)

/*
 * Adds a sprite instruction to the specified component
 * list for the sprite of the specified name passed as
 * a C string
 */
#define addSpriteInstruction( \
    LISTPTR, \
    GAMEPTR, \
    SPRITENAME, \
    DEPTH, \
    OFFSET, \
    ROTATION, \
    SCALE \
) \
    do{ \
        String nameString = stringMakeC(SPRITENAME); \
        TFSprite *spritePtr = resourcesGetSprite( \
            (GAMEPTR)->resourcesPtr, \
            &nameString \
        ); \
        if(!spritePtr){ \
            pgWarning(nameString._ptr); \
            pgError("failed to find sprite"); \
        }; \
        TFSpriteInstruction spriteInstruction \
            = tfSpriteInstructionMake( \
                spritePtr, \
                (DEPTH), \
                (OFFSET), \
                (ROTATION), \
                (SCALE) \
            ); \
        addComponent( \
            (LISTPTR), \
            SpriteInstruction, \
            spriteInstruction \
        ); \
        stringFree(&nameString); \
    } while(false)

/*
 * Adds a simple sprite instruction to the specified
 * component list for the sprite of the specified name
 * passed as a C string
 */
#define addSpriteInstructionSimple( \
    LISTPTR, \
    GAMEPTR, \
    SPRITENAME, \
    DEPTH, \
    OFFSET \
) \
    addSpriteInstruction( \
        LISTPTR, \
        GAMEPTR, \
        SPRITENAME, \
        DEPTH, \
        OFFSET, \
        0.0f, \
        1.0f \
    )

/*
 * Adds the specified sprite spin to the given
 * component list
 */
#define addSpriteSpin(LISTPTR, SPRITESPIN) \
    addComponent(LISTPTR, SpriteSpin, SPRITESPIN)

/*
 * Makes a copy of the specified animations and adds
 * it to the specified component list (does not free
 * the animations parameter)
 */
#define addAnimations(LISTPTR, ANIMATIONSPTR) \
    addComponent(LISTPTR, Animations, *(ANIMATIONSPTR))

/*
 * Adds the given menu commands object to the
 * specified component list
 */
#define addMenuCommands(LISTPTR, COMMANDS) \
    addComponent(LISTPTR, MenuCommands, COMMANDS)

/*
 * Adds the given neighbor elements object to the
 * specified component list
 */
#define addNeighborElements(LISTPTR, ELEMENTS) \
    addComponent( \
        LISTPTR, \
        NeighborElements, \
        ELEMENTS \
    )

/*
 * Adds the given button data object to the specified
 * component list
 */
#define addButtonData(LISTPTR, BUTTONDATA) \
    addComponent(LISTPTR, ButtonData, BUTTONDATA)

/*
 * Adds the specified inbound to the given component
 * list
 */
#define addInbound(LISTPTR, INBOUND) \
    addComponent(LISTPTR, Inbound, INBOUND)

/*
 * Adds the specified outbound to the given component
 * list
 */
#define addOutbound(LISTPTR, OUTBOUND) \
    addComponent(LISTPTR, Outbound, OUTBOUND)

/*
 * Adds the specified player data to the given
 * component list
 */
#define addPlayerData(LISTPTR, PLAYERDATA) \
    addComponent(LISTPTR, PlayerData, PLAYERDATA)

/*
 * Adds a collidable marker to the specified component
 * list
 */
#define addCollidable(LISTPTR) \
    addMarker(LISTPTR, CollidableMarkerID)

/*
 * Adds the specified hitbox to the given component
 * list
 */
#define addHitbox(LISTPTR, HITBOX) \
    addComponent(LISTPTR, Hitbox, HITBOX)

/*
 * Adds the specified health to the given component
 * list
 */
#define addHealth(LISTPTR, HEALTH) \
    addComponent(LISTPTR, Health, HEALTH)

/*
 * Adds the specified damage to the given component
 * list
 */
#define addDamage(LISTPTR, DAMAGE) \
    addComponent(LISTPTR, Damage, DAMAGE)

/*
 * Adds the specified player collision to the given
 * component list
 */
#define addPlayerCollision(LISTPTR, PLAYERCOLLISION) \
    addComponent( \
        LISTPTR, \
        PlayerCollision, \
        PLAYERCOLLISION \
    )

/*
 * Adds the specified enemy collision to the given
 * component list
 */
#define addEnemyCollision(LISTPTR, ENEMYCOLLISION) \
    addComponent( \
        LISTPTR, \
        EnemyCollision, \
        ENEMYCOLLISION \
    )

/*
 * Adds the specified bullet collision to the given
 * component list
 */
#define addBulletCollision(LISTPTR, BULLETCOLLISION) \
    addComponent( \
        LISTPTR, \
        BulletCollision, \
        BULLETCOLLISION \
    )

/*
 * Adds the specified pickup collision to the given
 * component list
 */
#define addPickupCollision(LISTPTR, PICKUPCOLLISION) \
    addComponent( \
        LISTPTR, \
        PickupCollision, \
        PICKUPCOLLISION \
    )

//todo: add other components

/*
 * Adds an entity to the specified scene and frees
 * the component list, storing the returned WindEntity
 * in the specified pointer (unless NULL is provided)
 */
#define addEntityAndFreeList( \
    LISTPTR, \
    SCENEPTR, \
    ENTITYOUTPTR \
) \
    do{ \
        WindEntity *tempPtr = (ENTITYOUTPTR); \
        if(tempPtr){ \
            *tempPtr = windWorldAddEntity( \
                &((SCENEPTR)->ecsWorld), \
                (LISTPTR) \
            ); \
        } \
        else{ \
            windWorldAddEntity( \
                &((SCENEPTR)->ecsWorld), \
                (LISTPTR) \
            ); \
        } \
        arrayListFree(WindComponentDataPair, \
            (LISTPTR) \
        ); \
    } while(false)

//todo: utility func to queue entity instead of add

#endif