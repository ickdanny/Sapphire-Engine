#include "InitSystem.h"

/* point at the center of the screen */
static const Point2D screenCenter = {
    config_graphicsWidth / 2.0f,
    config_graphicsHeight / 2.0f
};

/* Adds the requested background to the given scene */
static void addBackground(
    Game *gamePtr,
    Scene *scenePtr,
    const char *spriteName,
    int relativeDepth,
    Point2D pos
){
    declareList(componentList, 10);
    addVisible(&componentList);
    addPosition(&componentList, pos);
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        spriteName,
        config_backgroundDepth + relativeDepth,
        ((Vector2D){0})
    );
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        NULL
    );
}

/*
 * Adds the requested button to the given scene and
 * returns its handle; the button is assumed to be part
 * of a line of buttons each offset by the specified
 * vector
 */
static WindEntity addButtonInLine(
    Game *gamePtr,
    Scene *scenePtr,
    const char *unselSpriteName,
    const char *selSpriteName,
    MenuCommand selectCommand,
    MenuCommandData selectData,
    int relativeDepth,
    Point2D initPos,
    Vector2D lineOffset,
    int rowIndex,
    Vector2D selOffset,
    bool initiallySelected
){
    Point2D unselPos = point2DAddVector2D(
        initPos,
        vector2DMultiply(lineOffset, rowIndex)
    );
    Point2D selPos = point2DAddVector2D(
        unselPos,
        selOffset
    );
    ButtonData buttonData = {
        unselSpriteName,
        selSpriteName,
        unselPos,
        selPos
    };
    MenuCommands menuCommands = {
        .selectCommand = selectCommand,
        .selectData = selectData
    };
    
    declareList(componentList, 10);
    addVisible(&componentList);
    addPosition(
        &componentList,
        initiallySelected ? selPos : unselPos
    );
    addButtonData(&componentList, buttonData);
    addMenuCommands(&componentList, menuCommands);
    /* insert empty neighbor elements component */
    addNeighborElements(
        &componentList,
        ((NeighborElements){0})
    );
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        initiallySelected
            ? selSpriteName : unselSpriteName,
        config_foregroundDepth + relativeDepth,
        ((Vector2D){0})
    );
    WindEntity toRet = {0};
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        &toRet
    );

    return toRet;
}

/*
 * used for specifying the direction menu elements
 * should be linked
 */
typedef enum ElementLinkDirection{
    topDown,
    bottomUp,
    leftRight,
    rightLeft
} ElementLinkDirection;

/*
 * Attaches the elements provided in the given
 * arraylist of WindEntity in the specified direction
 * using the NeighborElements component (error if any
 * of the provided entities lacks such a component)
 */
static void linkElements(
    Scene *scenePtr,
    ArrayList *elementHandleListPtr,
    ElementLinkDirection linkDirection
){
    /* bail if not enough elements */
    if(elementHandleListPtr->size < 2){
        return;
    }
    WindEntity *predPtr = arrayListFrontPtr(
        WindEntity,
        elementHandleListPtr
    );
    WindEntity *succPtr = predPtr + 1;

    MenuCommands *predCommandsPtr
        = windWorldHandleGetPtr(
            MenuCommands,
            &(scenePtr->ecsWorld),
            *predPtr
        );
    MenuCommands *succCommandsPtr
        = windWorldHandleGetPtr(
            MenuCommands,
            &(scenePtr->ecsWorld),
            *succPtr
        );
    NeighborElements *predElementsPtr
        = windWorldHandleGetPtr(
            NeighborElements,
            &(scenePtr->ecsWorld),
            *predPtr
        );
    NeighborElements *succElementsPtr
        = windWorldHandleGetPtr(
            NeighborElements,
            &(scenePtr->ecsWorld),
            *succPtr
        );
    /* run once for every link between elements */
    for(size_t i = 0;
        i < elementHandleListPtr->size - 1;
        ++i
    ){
        /* link elements based on direction */
        switch(linkDirection){
            case topDown:
                predElementsPtr->down = *succPtr;
                succElementsPtr->up = *predPtr;
                predCommandsPtr->downCommand
                    = menu_navDown;
                succCommandsPtr->upCommand
                    = menu_navUp;
                break;
            case bottomUp:
                predElementsPtr->up = *succPtr;
                succElementsPtr->down = *predPtr;
                predCommandsPtr->downCommand
                    = menu_navDown;
                succCommandsPtr->upCommand
                    = menu_navUp;
                break;
            case leftRight:
                predElementsPtr->right = *succPtr;
                succElementsPtr->left = *predPtr;
                predCommandsPtr->rightCommand
                    = menu_navRight;
                succCommandsPtr->leftCommand
                    = menu_navLeft;
                break;
            case rightLeft:
                predElementsPtr->left = *succPtr;
                succElementsPtr->right = *predPtr;
                predCommandsPtr->rightCommand
                    = menu_navRight;
                succCommandsPtr->leftCommand
                    = menu_navLeft;
                break;
        }
        
        /* advance loop variables */
        predCommandsPtr = succCommandsPtr;
        predElementsPtr = succElementsPtr;
        ++predPtr;
        ++succPtr;
        succCommandsPtr = windWorldHandleGetPtr(
            MenuCommands,
            &(scenePtr->ecsWorld),
            *succPtr
        );
        succElementsPtr = windWorldHandleGetPtr(
            NeighborElements,
            &(scenePtr->ecsWorld),
            *succPtr
        );
    }
}

/*
 * Sets the initial selected element for the given
 * scene to the specified entity
 */
static void setInitSelectedElement(
    Scene *scenePtr,
    WindEntity handle
){
    scenePtr->messages.currentElement = handle;
}

/* initializes the entities for the main menu */
static void initMainMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    /* add the background for the main menu */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_main",
        0,
        screenCenter
    );

    /* add the buttons for the main menu */
    Point2D initPos = {
        280.0f,
        109.0f
    };
    Vector2D lineOffset = {-12.0f, -22.0f};
    Vector2D selOffset = {1.0f, 0.0f};

    ArrayList buttonHandles = arrayListMake(WindEntity,
        10
    );
    /* the start button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_startUnsel",
            "button_startSel",
            menu_enter,
            (MenuCommandData){.sceneData = {
                scene_difficulty,
                gb_start
            }},
            0,
            initPos,
            lineOffset,
            0,
            selOffset,
            true
        )
    );
    /* the practice button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_practiceUnsel",
            "button_practiceSel",
            menu_enter,
            (MenuCommandData){.sceneData = {
                scene_difficulty,
                gb_practice
            }},
            0,
            initPos,
            lineOffset,
            1,
            selOffset,
            false
        )
    );
    /* the music button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_musicUnsel",
            "button_musicSel",
            menu_enterStopMusic,
            (MenuCommandData){.sceneData = {
                scene_music,
                gb_none
            }},
            0,
            initPos,
            lineOffset,
            2,
            selOffset,
            false
        )
    );
    /* the options button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_optionsUnsel",
            "button_optionsSel",
            menu_enter,
            (MenuCommandData){.sceneData = {
                scene_options,
                gb_none
            }},
            0,
            initPos,
            lineOffset,
            3,
            selOffset,
            false
        )
    );
    /* the quit button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_quitUnsel",
            "button_quitSel",
            menu_exit,
            (MenuCommandData){0},
            0,
            initPos,
            lineOffset,
            4,
            selOffset,
            false
        )
    );
    linkElements(scenePtr, &buttonHandles, topDown);
    setInitSelectedElement(
        scenePtr,
        arrayListFront(WindEntity, &buttonHandles)
    );

    arrayListFree(WindEntity, &buttonHandles);

    scenePtr->messages.backMenuCommand
        = menu_navFarDown;

    //todo: start playback of track 01
}

/* initializes the entities for the difficulty menu */
static void initDiffMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    /* add the background for the difficulty menu */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_diff",
        0,
        screenCenter
    );

    /* add the buttons for the main menu */
    Point2D initPos = {
        config_graphicsWidth / 2,
        155.0f
    };
    Vector2D lineOffset = {0.0f, -50.0f};
    Vector2D selOffset = {0.0f, 1.0f};

    ArrayList buttonHandles = arrayListMake(WindEntity,
        10
    );
    /* the normal button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_normalUnsel",
            "button_normalSel",
            menu_enter,
            (MenuCommandData){.sceneData = {
                scene_game,
                gb_normal
            }},
            0,
            initPos,
            lineOffset,
            0,
            selOffset,
            true
        )
    );
    /* the hard button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_hardUnsel",
            "button_hardSel",
            menu_enter,
            (MenuCommandData){.sceneData = {
                scene_game,
                gb_hard
            }},
            0,
            initPos,
            lineOffset,
            1,
            selOffset,
            false
        )
    );
    /* the lunatic button button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_lunaticUnsel",
            "button_lunaticSel",
            menu_enterStopMusic,
            (MenuCommandData){.sceneData = {
                scene_game,
                gb_lunatic
            }},
            0,
            initPos,
            lineOffset,
            2,
            selOffset,
            false
        )
    );
    linkElements(scenePtr, &buttonHandles, topDown);
    setInitSelectedElement(
        scenePtr,
        arrayListFront(WindEntity, &buttonHandles)
    );

    arrayListFree(WindEntity, &buttonHandles);

    scenePtr->messages.backMenuCommand
        = menu_backTo;
    scenePtr->messages.backSceneID = scene_main;
}

/* initializes each scene */
void initSystem(Game *gamePtr, Scene *scenePtr){
    if(scenePtr->messages.initFlag){
        return;
    }
    scenePtr->messages.initFlag = true;

    switch(scenePtr->id){
        case scene_main:
            initMainMenu(gamePtr, scenePtr);
            break;
        case scene_difficulty:
            initDiffMenu(gamePtr, scenePtr);
            break;
        //todo: init other scenes
        default:
            pgError(
                "unrecognized scene in init system; "
                SRC_LOCATION
            );
            return;
    }
}