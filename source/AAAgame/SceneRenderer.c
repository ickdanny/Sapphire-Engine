#include "SceneRenderer.h"

#include "SpriteRenderSystem.h"

/* Renders the specified scene to screen */
void renderScene(Game *gamePtr, Scene *scenePtr){
    //todo render system
    spriteRenderSystem(gamePtr, scenePtr);
    
    //todo text render system
}