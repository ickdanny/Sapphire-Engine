#include "SceneUpdater.h"

#include "InitSystem.h"
#include "MessageCleanupSystem.h"
#include "InputSystem.h"
#include "MenuNavigationSystem.h"

/* Updates the specified scene */
void updateScene(Game *gamePtr, Scene *scenePtr){
    //pgWarning("updating scene\n");
    initSystem(gamePtr, scenePtr);
    messageCleanupSystem(gamePtr, scenePtr);
    inputSystem(gamePtr, scenePtr);
    menuNavigationSystem(gamePtr, scenePtr);
    //todo update scene
}