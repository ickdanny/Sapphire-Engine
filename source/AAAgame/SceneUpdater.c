#include "SceneUpdater.h"

#include "InitSystem.h"
#include "MessageCleanupSystem.h"
#include "InputSystem.h"
#include "MenuNavigationSystem.h"
#include "ButtonSpriteSystem.h"
#include "GameBuilderSystem.h"
#include "LoadingScreenSystem.h"

#include "PlayerMovementSystem.h"

#include "VelocitySystem.h"
#include "InboundSystem.h"
#include "CollisionDetectionSystem.h"

#include "PauseSystem.h"
#include "AnimationSystem.h"
#include "RotateSpriteForwardSystem.h"
#include "SpriteSpinSystem.h"

#include "OutboundSystem.h"

/* Updates the specified scene */
void updateScene(Game *gamePtr, Scene *scenePtr){
    initSystem(gamePtr, scenePtr);
    messageCleanupSystem(gamePtr, scenePtr);
    inputSystem(gamePtr, scenePtr);
    menuNavigationSystem(gamePtr, scenePtr);
    buttonSpriteSystem(gamePtr, scenePtr);
    gameBuilderSystem(gamePtr, scenePtr);
    loadingScreenSystem(gamePtr, scenePtr);
    //todo call dialogue system
    //todo call script system
    playerMovementSystem(gamePtr, scenePtr);
    velocitySystem(gamePtr, scenePtr);
    inboundSystem(gamePtr, scenePtr);
    collisionDetectionSystem(gamePtr, scenePtr);
    //todo call collision handler system
    //todo call clear system
    //todo call player shot system (?)
    //todo call player state system (?)
    //todo call player bomb system (?)
    //todo call player death detector system (?)
    //todo call continue system
    //todo call player respawn system (?)
    //todo call player reactivate system (?)
    //todo call death handler system
    //todo call overlay system
    pauseSystem(gamePtr, scenePtr);
    animationSystem(gamePtr, scenePtr);
    rotateSpriteForwardSystem(gamePtr, scenePtr);
    spriteSpinSystem(gamePtr, scenePtr);
    //todo call sub image scroll system
    outboundSystem(gamePtr, scenePtr);
    //todo call game over system
    //todo call credits system
}