#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "Trifecta.h"
#include "WindECS.h"
#include "ZMath.h"

#include "Scenes.h"

/*
 * Allocates and returns a new WindComponents object
 * containing the RTTI details of every component
 * used in the game engine
 */
WindComponents *componentsMake();

/* Component 0: Position */
typedef struct Position{
    Point2D currentPos;
    Point2D pastPos;
} Position;

/* Component 1: Velocity */
typedef Polar Velocity;

/* Component 2: Visible Marker */

/* Component 3: SpriteInstruction */
typedef TFSpriteInstruction SpriteInstruction;

/* TODO: Component 4: subimage rectangle */

/* TODO: Component 5: TilingInstruction with rect and point2 */

/* TODO: Component 6: TileScroll vec2 */

/* TODO: Component 7: TextInstruction with string and int rightBound */

/* TODO: Component 8: Rotate sprite forward marker */

/* TODO: Component 9: SpriteSpin with float spin */

/* TODO: Component 10: AnimationList */ 

/* Component 11: MenuCommands */
typedef enum GameBuilderCommand{
    gb_none,
    gb_start,
    gb_practice,

    gb_normal,
    gb_hard,
    gb_lunatic,

    gb_stage1,
    gb_stage2,
    gb_stage3,
    gb_stage4,

    gb_reset,

    gb_invalid,
} GameBuilderCommand;
typedef enum MenuCommand{
    menu_none,

    menu_navUp,
    menu_navDown,
    menu_navLeft,
    menu_navRight,

    menu_navFarUp,
    menu_navFarDown,
    menu_navFarLeft,
    menu_navFarRight,

    menu_enter,
    menu_enterStopMusic,

    menu_backTo,
    menu_backWriteSettings,
    menu_backSetMenuTrack,

    menu_startTrack,

    menu_toggleSound,
    menu_toggleFullscreen,

    menu_restartGame,

    menu_gameOver,

    menu_exit,

    menu_invalid,
} MenuCommand;
typedef union MenuCommandData{
    /* data pertaining to entering a new scene */
    struct{
        SceneID sceneID;
        GameBuilderCommand gameBuilderCommand;
    } sceneEntry;
    /*
     * the name of the track to start playback, not
     * owned by the object (should be a string literal)
     */
    const char *trackName;
} MenuCommandData;
typedef struct MenuCommands{
    MenuCommand upCommand;
    MenuCommandData upData;

    MenuCommand downCommand;
    MenuCommandData downData;

    MenuCommand leftCommand;
    MenuCommandData leftData;

    MenuCommand rightCommand;
    MenuCommandData rightData;

    MenuCommand selectCommand;
    MenuCommandData selectData;
} MenuCommands;

/* Component 12: NeighborElements */
typedef struct NeighborElements{
    WindEntity up;
    WindEntity down;
    WindEntity left;
    WindEntity right;
} NeighborElements;

/* Component 13: ButtonData */
typedef struct ButtonData{
    /*
     * name of the unselected sprite as a C string; not
     * owned by the object (should be string literal)
     */
    const char *unselSpriteName;
    /*
     * name of the selected sprite as a C string; not
     * owned by the object (should be string literal)
     */
    const char *selSpriteName;
    
    Point2D unselPos;
    Point2D selPos;

    /*
     * OYCN will not have any locked buttons, but the
     * implementation of such a feature would be
     * here
     */
} ButtonData;

/* each component needs TYPENAME##ID defined */
typedef enum ComponentID{
    PositionID,
    VelocityID,
    VisibleMarkerID,
    SpriteInstructionID,
    MenuCommandsID,
    NeighborElementsID,
    ButtonDataID,
    numComponents,
} ComponentID;

#endif