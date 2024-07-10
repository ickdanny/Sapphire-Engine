#include "SceneUpdater.h"

#include "InitSystem.h"

/* Updates the specified scene */
void updateScene(Game *gamePtr, Scene *scenePtr){
    pgWarning("updating scene\n");
    initSystem(gamePtr, scenePtr);
    //todo update scene
}