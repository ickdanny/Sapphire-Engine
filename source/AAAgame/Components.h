#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "Trifecta.h"
#include "Vecs.h"
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
 * Allocates and returns a new VecsComponentList object
 * containing the RTTI details of every component
 * used in the game engine
 */
VecsComponentList *componentListMake();

/* Component 1: Position */
typedef struct Position{
    Point2D currentPos;
    Point2D pastPos;
} Position;

/* Component 2: Velocity */
typedef Polar Velocity;

/* Component 3: Visible Marker */

/* Component 4: SpriteInstruction */
typedef TFSpriteInstruction SpriteInstruction;

/* Component 5: SubImage */
typedef Rectangle SubImage;

/* Component 6: TilingInstruction */
typedef struct TilingInstruction{
    Rectangle drawRect;
    Point2D pixelOffset;
} TilingInstruction;

/* Component 7: TileScroll */
typedef Vector2D TileScroll;

/* Component 8: TextInstruction */
typedef struct TextInstruction{
    WideString text;
    int rightBound;
} TextInstruction;

void textInstructionDestructor(void *voidPtr);

/* Component 9: Rotate sprite forward marker */

/* Component 10: SpriteSpin */
typedef float SpriteSpin;

/* Component 11: Animations */
typedef AnimationList Animations;

void animationsDestructor(void *voidPtr);

/* Component 12: MenuCommands */
typedef union MenuCommandData{
    /* data pertaining to entering a new scene */
    struct{
        SceneId sceneId;
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

/* Component 13: NeighborElements */
typedef struct NeighborElements{
    VecsEntity up;
    VecsEntity down;
    VecsEntity left;
    VecsEntity right;
} NeighborElements;

/* Component 14: ButtonData */
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

/* Component 15: Inbound */
typedef float Inbound;

/* Component 16: Outbound */
typedef float Outbound;

/* Component 17: Player Data */

/* Component 18: Collidable Marker */

/* Component 19: Hitbox */
typedef AABB Hitbox;

/* Component 20: Health */
typedef int Health;

/* Component 21: Damage */
typedef int Damage;

/* Component 22: PlayerCollisionSource */
typedef CollisionCommand PlayerCollisionSource;

/* Component 23: PlayerCollisionTarget */
typedef CollisionCommand PlayerCollisionTarget;

/* Component 24: EnemyCollisionSource */
typedef CollisionCommand EnemyCollisionSource;

/* Component 25: EnemyCollisionTarget */
typedef CollisionCommand EnemyCollisionTarget;

/* Component 26: BulletCollisionSource */
typedef CollisionCommand BulletCollisionSource;

/* Component 27: BulletCollisionTarget */
typedef CollisionCommand BulletCollisionTarget;

/* Component 28: PickupCollisionSource */
typedef CollisionCommand PickupCollisionSource;

/* Component 29: PickupCollisionTarget */
typedef CollisionCommand PickupCollisionTarget;

/* Component 30: PowerGain */
typedef int PowerGain;

/* Component 31: ClearableMarker */

/* Component 32: Scripts */
void scriptsDestructor(void *voidPtr);

/* Component 33: DeathCommand */

/* Component 34: DeathScripts */
typedef struct DeathScripts{
    String scriptId1;
    String scriptId2;
    String scriptId3;
    String scriptId4;
} DeathScripts;

void deathScriptsDestructor(void *voidPtr);

/* each component needs TYPENAME##Id defined */
typedef enum ComponentId{
    PositionId = 1,
    VelocityId,
    VisibleMarkerId,
    SpriteInstructionId,
    SubImageId,
    TilingInstructionId,
    TileScrollId,
    TextInstructionId,
    RotateSpriteForwardMarkerId,
    SpriteSpinId,
    AnimationsId,
    MenuCommandsId,
    NeighborElementsId,
    ButtonDataId,
    InboundId,
    OutboundId,
    PlayerDataId,
    CollidableMarkerId,
    HitboxId,
    HealthId,
    DamageId,
    PlayerCollisionSourceId,
    PlayerCollisionTargetId,
    EnemyCollisionSourceId,
    EnemyCollisionTargetId,
    BulletCollisionSourceId,
    BulletCollisionTargetId,
    PickupCollisionSourceId,
    PickupCollisionTargetId,
    PowerGainId,
    ClearableMarkerId,
    ScriptsId,
    DeathCommandId,
    DeathScriptsId,
    numComponents,
} ComponentId;

#endif