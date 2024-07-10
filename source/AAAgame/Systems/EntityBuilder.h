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