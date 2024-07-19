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
 * Adds the specified sub image to the given component
 * list
 */
#define addSubImage(LISTPTR, SUBIMAGE) \
    addComponent(LISTPTR, SubImage, SUBIMAGE)

/*
 * Adds the specified tiling instruction to the given
 * component list
 */
#define addTilingInstruction( \
    LISTPTR, \
    DRAWRECT, \
    PIXELOFFSET \
) \
    addComponent( \
        LISTPTR, \
        TilingInstruction, \
        ((TilingInstruction){ \
            (DRAWRECT), \
            (PIXELOFFSET) \
        }) \
    )

/*
 * Adds the specified tile scroll to the given
 * component list
 */
#define addTileScroll(LISTPTR, TILESCROLL) \
    addComponent(LISTPTR, TileScroll, TILESCROLL)

/*
 * Adds a rotate sprite forward marker to the given
 * component list
 */
#define addRotateSpriteForward(LISTPTR) \
    addMarker(LISTPTR, RotateSpriteForwardMarkerID)

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
 * Adds the specified player collision source to the
 * given component list
 */
#define addPlayerCollisionSource( \
    LISTPTR, \
    PLAYERCOLLISIONSOURCE \
) \
    addComponent( \
        LISTPTR, \
        PlayerCollisionSource, \
        PLAYERCOLLISIONSOURCE \
    )

/*
 * Adds the specified player collision target to the
 * given component list
 */
#define addPlayerCollisionTarget( \
    LISTPTR, \
    PLAYERCOLLISIONTARGET \
) \
    addComponent( \
        LISTPTR, \
        PlayerCollisionTarget, \
        PLAYERCOLLISIONTARGET \
    )

/*
 * Adds the specified enemy collision source to the
 * given component list
 */
#define addEnemyCollisionSource( \
    LISTPTR, \
    ENEMYCOLLISIONSOURCE \
) \
    addComponent( \
        LISTPTR, \
        EnemyCollisionSource, \
        ENEMYCOLLISIONSOURCE \
    )

/*
 * Adds the specified player collision target to the
 * given component list
 */
#define addEnemyCollisionTarget( \
    LISTPTR, \
    ENEMYCOLLISIONTARGET \
) \
    addComponent( \
        LISTPTR, \
        EnemyCollisionTarget, \
        ENEMYCOLLISIONTARGET \
    )

/*
 * Adds the specified bullet collision source to the
 * given component list
 */
#define addBulletCollisionSource( \
    LISTPTR, \
    BULLETCOLLISIONSOURCE \
) \
    addComponent( \
        LISTPTR, \
        BulletCollisionSource, \
        BULLETCOLLISIONSOURCE \
    )

/*
 * Adds the specified bullet collision target to the
 * given component list
 */
#define addBulletCollisionTarget( \
    LISTPTR, \
    BULLETCOLLISIONTARGET \
) \
    addComponent( \
        LISTPTR, \
        BulletCollisionTarget, \
        BULLETCOLLISIONTARGET \
    )

/*
 * Adds the specified pickup collision source to the
 * given component list
 */
#define addPickupCollisionSource( \
    LISTPTR, \
    PICKUPCOLLISIONSOURCE \
) \
    addComponent( \
        LISTPTR, \
        PickupCollisionSource, \
        PICKUPCOLLISIONSOURCE \
    )

/*
 * Adds the specified pickup collision target to the
 * given component list
 */
#define addPickupCollisionTarget( \
    LISTPTR, \
    PICKUPCOLLISIONSOURCE \
) \
    addComponent( \
        LISTPTR, \
        PickupCollisionTarget, \
        PICKUPCOLLISIONSOURCE \
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