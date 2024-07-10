#include "InitSystem.h"

/* Adds the requested background to the given scene */
static void addBackground(
    Game *gamePtr,
    Scene *scenePtr,
    const char *name,
    int relativeDepth,
    Point2D position
){
    ArrayList componentList = arrayListMake(
        WindComponentDataPair,
        10
    );
    addVisible(&componentList);
    addPosition(&componentList, position);
    addComponent(
        &componentList,
        SpriteInstruction,
        tfSpriteInstructionMakeSimple(

        )
    )
    windWorldAddEntity(
        &(scenePtr->ecsWorld),
        &componentList
    );
    
    //todo add background


    arrayListFree(WindComponentDataPair,
        &componentList
    );
}

/* initializes the entities for the main menu */
static void initMainMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    //todo init main menu

    //todo: add background
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