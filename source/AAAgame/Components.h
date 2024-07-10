#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "Trifecta.h"
#include "WindECS.h"
#include "ZMath.h"

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

/* each component needs TYPENAME##ID defined */
typedef enum ComponentID{
    PositionID,
    VelocityID,
    VisibleMarkerID,
    SpriteInstructionID,
    numComponents,
} ComponentID;

#endif