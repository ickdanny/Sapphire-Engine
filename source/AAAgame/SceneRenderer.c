#include "SceneRenderer.h"

#include "SpriteRenderSystem.h"

/* Renders the specified scene to screen */
void renderScene(Game *gamePtr, Scene *scenePtr){
    pgWarning("rendering scene");
    //todo render scene
    //todo render system
    spriteRenderSystem(gamePtr, scenePtr);
    
    //todo text render system
}