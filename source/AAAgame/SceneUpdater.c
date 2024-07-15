#include "SceneUpdater.h"

#include "InitSystem.h"
#include "MessageCleanupSystem.h"
#include "InputSystem.h"
#include "MenuNavigationSystem.h"
#include "ButtonSpriteSystem.h"
#include "GameBuilderSystem.h"
#include "LoadingScreenSystem.h"

#include "VelocitySystem.h"
#include "InboundSystem.h"

/* Updates the specified scene */
void updateScene(Game *gamePtr, Scene *scenePtr){
    //pgWarning("updating scene\n");
    initSystem(gamePtr, scenePtr);
    messageCleanupSystem(gamePtr, scenePtr);
    inputSystem(gamePtr, scenePtr);
    menuNavigationSystem(gamePtr, scenePtr);
    buttonSpriteSystem(gamePtr, scenePtr);
    gameBuilderSystem(gamePtr, scenePtr);
    loadingScreenSystem(gamePtr, scenePtr);
    //todo call dialogue system
    //todo call script system
    //todo call player movement system
    velocitySystem(gamePtr, scenePtr);
    inboundSystem(gamePtr, scenePtr);
    //todo call collision detector system
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
    //todo call pause system
    //todo call animation system
    //todo call rotate sprite forward system
    //todo call sprite spin system
    //todo call sub image scroll system
    //todo call outbound system
    //todo call game over system
    //todo call credits system

    //todo update scene
}