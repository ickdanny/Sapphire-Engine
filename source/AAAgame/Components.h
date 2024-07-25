#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "Trifecta.h"
#include "WindECS.h"
#include "ZMath.h"

#include "MenuCommand.h"
#include "GameBuilderCommand.h"
#include "Scenes.h"
#include "Animation.h"
#include "PlayerData.h"
#include "CollisionCommand.h"
#include "Scripts.h"
#include "DeathCommand.h"

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

/* Component 4: SubImage */
typedef Rectangle SubImage;

/* Component 5: TilingInstruction */
typedef struct TilingInstruction{
    Rectangle drawRect;
    Point2D pixelOffset;
} TilingInstruction;

/* Component 6: TileScroll */
typedef Vector2D TileScroll;

/* Component 7: TextInstruction */
typedef struct TextInstruction{
    WideString text;
    int rightBound;
} TextInstruction;

void textInstructionDestructor(void *voidPtr);

/* Component 8: Rotate sprite forward marker */

/* Component 9: SpriteSpin */
typedef float SpriteSpin;

/* Component 10: Animations */
typedef AnimationList Animations;

void animationsDestructor(void *voidPtr);

/* Component 11: MenuCommands */
typedef union MenuCommandData{
    /* data pertaining to entering a new scene */
    struct{
        SceneID sceneID;
        GameBuilderCommand gameBuilderCommand;
    } sceneData;
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

/* Component 14: Inbound */
typedef float Inbound;

/* Component 15: Outbound */
typedef float Outbound;

/* Component 16: Player Data */

/* Component 17: Collidable Marker */

/* Component 18: Hitbox */
typedef AABB Hitbox;

/* Component 19: Health */
typedef int Health;

/* Component 20: Damage */
typedef int Damage;

/* Component 21: PlayerCollisionSource */
typedef CollisionCommand PlayerCollisionSource;

/* Component 22: PlayerCollisionTarget */
typedef CollisionCommand PlayerCollisionTarget;

/* Component 23: EnemyCollisionSource */
typedef CollisionCommand EnemyCollisionSource;

/* Component 24: EnemyCollisionTarget */
typedef CollisionCommand EnemyCollisionTarget;

/* Component 25: BulletCollisionSource */
typedef CollisionCommand BulletCollisionSource;

/* Component 26: BulletCollisionTarget */
typedef CollisionCommand BulletCollisionTarget;

/* Component 27: PickupCollisionSource */
typedef CollisionCommand PickupCollisionSource;

/* Component 28: PickupCollisionTarget */
typedef CollisionCommand PickupCollisionTarget;

/* Component 29: PowerGain */
typedef int PowerGain;

/* Component 30: ClearableMarker */

/* Component 31: Scripts */
void scriptsDestructor(void *voidPtr);

/* Component 32: DeathCommand */

/* Component 33: DeathScripts */
typedef struct DeathScripts{
    String scriptID1;
    String scriptID2;
    String scriptID3;
    String scriptID4;
} DeathScripts;

void deathScriptsDestructor(void *voidPtr);

/* each component needs TYPENAME##ID defined */
typedef enum ComponentID{
    PositionID,
    VelocityID,
    VisibleMarkerID,
    SpriteInstructionID,
    SubImageID,
    TilingInstructionID,
    TileScrollID,
    TextInstructionID,
    RotateSpriteForwardMarkerID,
    SpriteSpinID,
    AnimationsID,
    MenuCommandsID,
    NeighborElementsID,
    ButtonDataID,
    InboundID,
    OutboundID,
    PlayerDataID,
    CollidableMarkerID,
    HitboxID,
    HealthID,
    DamageID,
    PlayerCollisionSourceID,
    PlayerCollisionTargetID,
    EnemyCollisionSourceID,
    EnemyCollisionTargetID,
    BulletCollisionSourceID,
    BulletCollisionTargetID,
    PickupCollisionSourceID,
    PickupCollisionTargetID,
    PowerGainID,
    ClearableMarkerID,
    ScriptsID,
    DeathCommandID,
    DeathScriptsID,
    numComponents,
} ComponentID;

#endif