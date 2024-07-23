#include "MenuNavigationSystem.h"

#include "MenuNavigationCommand.h"

/* Sets the current element in the specified scene */
void setElement(
    Scene *scenePtr,
    WindEntity prevElement,
    WindEntity newElement
){
    /* bail if prev and new are same */
    if(windEntityEquals(prevElement, newElement)){
        return;
    }
    scenePtr->messages.currentElement = newElement;
    scenePtr->messages.elementChanges
        .newElementSelected = true;
    scenePtr->messages.elementChanges.prevElement
        = prevElement;
}

/* Handles a navigate (direction) command */
void handleNavigateCommand(
    Scene *scenePtr,
    MenuCommand menuCommand,
    WindEntity currentElement
){
    NeighborElements *neighborElementsPtr
        = windWorldHandleGetPtr(NeighborElements,
            &(scenePtr->ecsWorld),
            currentElement
        );
    WindEntity nextElement = {0};
    /*
     * assume the neighbor element is valid, since
     * otherwise the element would not have a nav
     * command associated
     */
    switch(menuCommand){
        case menu_navUp:
            nextElement
                = neighborElementsPtr->up;
            break;
        case menu_navDown:
            nextElement
                = neighborElementsPtr->down;
            break;
        case menu_navLeft:
            nextElement
                = neighborElementsPtr->left;
            break;
        case menu_navRight:
            nextElement
                = neighborElementsPtr->right;
            break;
        default:
            pgError(
                "not a nav command; "
                SRC_LOCATION
            );
    }
    setElement(
        scenePtr,
        currentElement,
        nextElement
    );
}

/*
 * Handles a navigate far command, which basically
 * navigates in a certain direction until it can
 * no longer go any further
 */
void handleNavigateFarCommand(
    Scene *scenePtr,
    MenuCommand menuCommand,
    WindEntity currentElement
){
    NeighborElements *neighborElementsPtr
        = windWorldHandleGetPtr(NeighborElements,
            &(scenePtr->ecsWorld),
            currentElement
        );
    WindEntity nextElement = currentElement;
    /*
     * assume the neighbor element is valid, since
     * otherwise the element would not have a nav
     * command associated
     */
    do{
        switch(menuCommand){
            case menu_navFarUp:
                /* bail if invalid entity (gen0) */
                if(neighborElementsPtr->up._generation
                    == 0
                ){
                    goto endLoop;
                }
                nextElement
                    = neighborElementsPtr->up;
                break;
            case menu_navFarDown:
                /* bail if invalid entity (gen0) */
                if(neighborElementsPtr->down
                    ._generation == 0
                ){
                    goto endLoop;
                }
                nextElement
                    = neighborElementsPtr->down;
                break;
            case menu_navFarLeft:
                /* bail if invalid entity (gen0) */
                if(neighborElementsPtr->left
                    ._generation == 0
                ){
                    goto endLoop;
                }
                nextElement
                    = neighborElementsPtr->left;
                break;
            case menu_navFarRight:
                /* bail if invalid entity (gen0) */
                if(neighborElementsPtr->right
                    ._generation == 0
                ){
                    goto endLoop;
                }
                nextElement
                    = neighborElementsPtr->right;
                break;
            default:
                pgError(
                    "unexpected default nav cmd; "
                    SRC_LOCATION
                );
                break;
        }
        neighborElementsPtr
            = windWorldHandleGetPtr(NeighborElements,
                &(scenePtr->ecsWorld),
                nextElement
            );
    /* avoid infinite loops */
    } while(!windEntityEquals(
        currentElement,
        nextElement)
    );
endLoop:
    setElement(
        scenePtr,
        currentElement,
        nextElement
    );
}

/*
 * Handles an enter command by requesting scene pushes
 */
void handleEnterCommand(
    Game *gamePtr,
    Scene *scenePtr,
    MenuCommandData commandData
){
    SceneID sceneID = commandData.sceneData.sceneID;
    GameBuilderCommand gameBuilderCommand
        = commandData.sceneData.gameBuilderCommand;
    arrayListPushBack(SceneID,
        &(gamePtr->messages.sceneEntryList),
        sceneID
    );

    /* if going to game, push a load */
    if(sceneID == scene_game){
        arrayListPushBack(SceneID,
            &(gamePtr->messages.sceneEntryList),
            scene_loading
        );
    }

    /* set game builder command; gb_none is possible */
    scenePtr->messages.gameBuilderCommand
        = gameBuilderCommand;
}

/* Pops the scene stack out of the game */
void popOutOfGame(Game *gamePtr){
    GameState *gameStatePtr
        = &(gamePtr->messages.gameState);
    SceneID backTo = 0;
    switch(gameStatePtr->gameMode){
        case game_story:
            backTo = scene_main;
            break;
        case game_practice:
            backTo = scene_stage;
            break;
        default:
            pgError(
                "unexpected game mode default case; "
                SRC_LOCATION
            );
            break;
    }
    gamePtr->messages.sceneExitToID = backTo;
}

/*
 * Handles restarting the game by popping and pushing
 * scenes
 */
void handleRestartGameCommand(
    Game *gamePtr,
    Scene *scenePtr
){
    gamePtr->messages.stopMusicFlag = true;

    popOutOfGame(gamePtr);

    /* have game builder system reset the game state */
    scenePtr->messages.gameBuilderCommand = gb_reset;

    /* pop new game and loading screen */
    arrayListPushBack(SceneID,
        &(gamePtr->messages.sceneEntryList),
        scene_game
    );
    arrayListPushBack(SceneID,
        &(gamePtr->messages.sceneEntryList),
        scene_loading
    );
}

/* Handles game over by popping scenes */
void handleGameOverCommand(Game *gamePtr){
    gamePtr->messages.stopMusicFlag = true;
    //todo: start track 1

    popOutOfGame(gamePtr);
}

/*
 * Handles the specified menu command; returns true if
 * the command is critical, false otherwise
 */
bool parseMenuCommand(
    Game *gamePtr,
    Scene *scenePtr,
    MenuCommand menuCommand,
    MenuCommandData commandData,
    WindEntity currentElement
){
    switch(menuCommand){
        case menu_navUp:
        case menu_navDown:
        case menu_navLeft:
        case menu_navRight:
            handleNavigateCommand(
                scenePtr,
                menuCommand,
                currentElement
            );
            return false;
        case menu_navFarUp:
        case menu_navFarDown:
        case menu_navFarLeft:
        case menu_navFarRight:
            handleNavigateFarCommand(
                scenePtr,
                menuCommand,
                currentElement
            );
            return false;

        case menu_enterStopMusic:
            gamePtr->messages.stopMusicFlag = true;
            /* fall through */
        case menu_enter:
            handleEnterCommand(
                gamePtr,
                scenePtr,
                commandData
            );
            return true; /* entering is critical */

        case menu_backSetMenuTrack:
            //todo start track 1
            /* fall through */
        case menu_backTo:
            gamePtr->messages.sceneExitToID
                = commandData.sceneData.sceneID;
            return true; /* exiting is critical */
        case menu_backWriteSettings:
            gamePtr->messages.writeSettingsFlag = true;
            gamePtr->messages.sceneExitToID
                = commandData.sceneData.sceneID;
            return true; /* exiting is critical */

        case menu_startTrack:
            //todo start music
            return false;
        case menu_toggleSound:
            gamePtr->messages.toggleSoundFlag = true;
            return false;
        case menu_toggleFullscreen:
            gamePtr->messages.toggleFullscreenFlag
                = true;
            return false;
        
        case menu_restartGame:
            handleRestartGameCommand(
                gamePtr,
                scenePtr
            );
            return true; /* restarting is critical */
        case menu_gameOver:
            handleGameOverCommand(gamePtr);
            return true; /* game over is critical */
        case menu_exit:
            gamePtr->messages.exitFlag = true;
            return true; /* exiting is critical */

        case menu_none:
            return false;

        default:
            pgError(
                "unexpected default menu command; "
                SRC_LOCATION
            );
            return true;
    }
}

/*
 * Retrieves the associated menu command associated
 * with the specified navigation command and handles
 * it; returns true if the command is critical, false
 * otherwise
 */
bool parseNavigationCommand(
    Game *gamePtr,
    Scene *scenePtr,
    MenuNavigationCommand navigationCommand,
    WindEntity currentElement
){
    MenuCommand menuCommand = menu_none;
    MenuCommandData commandData = {0};
    MenuCommands *elementCommandsPtr
        = windWorldHandleGetPtr(MenuCommands,
            &(scenePtr->ecsWorld),
            currentElement
        );
    switch(navigationCommand){
        case menuNav_back:
            menuCommand
                = scenePtr->messages.backMenuCommand;
            commandData.sceneData.sceneID
                = scenePtr->messages.backSceneID;
            break;
        case menuNav_select:
            menuCommand
                = elementCommandsPtr->selectCommand;
            commandData
                = elementCommandsPtr->selectData;
            break;
        case menuNav_up:
            menuCommand
                = elementCommandsPtr->upCommand;
            commandData
                = elementCommandsPtr->upData;
            break;
        case menuNav_down:
            menuCommand
                = elementCommandsPtr->downCommand;
            commandData
                = elementCommandsPtr->downData;
            break;
        case menuNav_left:
            menuCommand
                = elementCommandsPtr->leftCommand;
            commandData
                = elementCommandsPtr->leftData;
            break;
        case menuNav_right:
            menuCommand
                = elementCommandsPtr->rightCommand;
            commandData
                = elementCommandsPtr->rightData;
            break;
        default:
            pgError(
                "unexpected default menu nav command; "
                SRC_LOCATION
            );
    }
    return parseMenuCommand(
        gamePtr,
        scenePtr,
        menuCommand,
        commandData,
        currentElement
    );
}


/* handles navigating between menu elements */
void menuNavigationSystem(
    Game *gamePtr,
    Scene *scenePtr
){
    /*
     * this system is responsible for clearing the
     * element changes messages
     */
    scenePtr->messages.elementChanges
        .newElementSelected = false;
    scenePtr->messages.elementChanges.prevElement
        = (WindEntity){0};
    
    ArrayList *menuNavigationCommandsPtr
        = &(scenePtr->messages.menuNavigationCommands);

    /* bail if no navigation commands */
    if(arrayListIsEmpty(menuNavigationCommandsPtr)){
        return;
    }

    WindEntity currentElement
        = scenePtr->messages.currentElement;
    bool critical = false;
    for(size_t i = 0;
        i < menuNavigationCommandsPtr->size;
        ++i
    ){
        MenuNavigationCommand navigationCommand
            = arrayListGet(MenuNavigationCommand,
                menuNavigationCommandsPtr,
                i
            );
        critical = parseNavigationCommand(
            gamePtr,
            scenePtr,
            navigationCommand,
            currentElement
        );
        if(critical){
            break;
        }
    }
}