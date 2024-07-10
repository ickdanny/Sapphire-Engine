#include "InitSystem.h"

/* point at the center of the screen */
static const Point2D screenCenter = {
    config_windowWidth / 2.0f,
    config_windowHeight / 2.0f
};

/* Adds the requested background to the given scene */
static void addBackground(
    Game *gamePtr,
    Scene *scenePtr,
    const char *name,
    int relativeDepth,
    Point2D position
){
    declareList(componentList, 10);
    addVisible(&componentList);
    addPosition(&componentList, position);
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        name,
        config_backgroundDepth + relativeDepth,
        ((Vector2D){0})
    );
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        NULL
    );
}

/* initializes the entities for the main menu */
static void initMainMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    //todo init main menu

    //todo: add background
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_main",
        0,
        screenCenter
    );
    //todo: add buttons
    //todo: set back menu to nav down
    //todo: start playback of track 01
}

/* initializes each scene */
void initSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    //todo init system
    if(scenePtr->sceneMessages.initFlag){
        return;
    }
    scenePtr->sceneMessages.initFlag = true;

    switch(scenePtr->id){
        case scene_main:
            initMainMenu(gamePtr, scenePtr);
            break;
        default:
            pgError(
                "unrecognized scene in init system; "
                SRC_LOCATION
            );
            return;
    }
}