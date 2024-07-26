#include "SceneRenderer.h"

#include "SpriteRenderSystem.h"
#include "TextRenderSystem.h"

/* Renders the specified scene to screen */
void renderScene(Game *gamePtr, Scene *scenePtr){
    spriteRenderSystem(gamePtr, scenePtr);
    textRenderSystem(gamePtr, scenePtr);
}