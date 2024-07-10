#ifndef ENTITYBUILDER_H
#define ENTITYBUILDER_H

#include "WindECS.h"

/*
 * the following functions are utility functions for
 * building up component lists; note that WindECS will
 * error for duplicate components
 */

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
 * Adds a simple sprite instruction to the specified
 * component list for the sprite of the specified name
 * passed as a C string
 */
#define addSpriteInstructionSimple( \
    LISTPTR, \
    GAMEPTR, \
    SPRITENAME \
) \
    do{

//TODO: 
    } while(false)

#endif