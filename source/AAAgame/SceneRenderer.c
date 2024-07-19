#include "SceneRenderer.h"

#include "SpriteRenderSystem.h"

/* Renders the specified scene to screen */
void renderScene(Game *gamePtr, Scene *scenePtr){
    spriteRenderSystem(gamePtr, scenePtr);
    
    //todo text render system
}