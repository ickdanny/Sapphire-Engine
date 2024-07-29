#include "InitSystem.h"

#include "NativeFuncs.h"

/* point at the center of the screen */
static const Point2D screenCenter = {
    config_graphicsWidth / 2.0f,
    config_graphicsHeight / 2.0f
};

/* point at the center of the game screen */
static const Point2D gameCenter = {
    100.0f,
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

/* Adds the requested foreground to the given scene */
static void addForeground(
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
        config_foregroundDepth + relativeDepth,
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

    /*
     * next scene is either game or stage depending on
     * if the player pressed start or practice
     */
    SceneID nextScene = gamePtr->messages.gameState
        .gameMode == game_story
            ? scene_game
            : scene_stage;

    /* add the buttons for the difficulty menu */
    Point2D initPos = {
        config_graphicsWidth / 2,
        150.0f
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
                nextScene,
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
                nextScene,
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
    /* the lunatic button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_lunaticUnsel",
            "button_lunaticSel",
            menu_enterStopMusic,
            (MenuCommandData){.sceneData = {
                nextScene,
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

/* initializes the entities for the stage menu */
static void initStageMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    /* add the background for the difficulty menu */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_stage",
        0,
        screenCenter
    );

    /* add the buttons for the stage menu */
    Point2D initPos = {
        config_graphicsWidth / 2,
        157.0f
    };
    Vector2D lineOffset = {0.0f, -40.0f};
    Vector2D selOffset = {0.0f, 1.0f};

    ArrayList buttonHandles = arrayListMake(WindEntity,
        10
    );
    /* the stage 1 button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_s1Unsel",
            "button_s1Sel",
            menu_enter,
            (MenuCommandData){.sceneData = {
                scene_game,
                gb_stage1
            }},
            0,
            initPos,
            lineOffset,
            0,
            selOffset,
            true
        )
    );
    /* the stage 2 button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_s2Unsel",
            "button_s2Sel",
            menu_enter,
            (MenuCommandData){.sceneData = {
                scene_game,
                gb_stage2
            }},
            0,
            initPos,
            lineOffset,
            1,
            selOffset,
            false
        )
    );
    /* the stage 3 button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_s3Unsel",
            "button_s3Sel",
            menu_enterStopMusic,
            (MenuCommandData){.sceneData = {
                scene_game,
                gb_stage3
            }},
            0,
            initPos,
            lineOffset,
            2,
            selOffset,
            false
        )
    );
    /* the stage 4 button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_s4Unsel",
            "button_s4Sel",
            menu_enterStopMusic,
            (MenuCommandData){.sceneData = {
                scene_game,
                gb_stage4
            }},
            0,
            initPos,
            lineOffset,
            3,
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
    scenePtr->messages.backSceneID = scene_difficulty;
}

//todo: init music menu

/* initializes the entities for the options menu */
static void initOptionsMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    /* add the background for the difficulty menu */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_options",
        0,
        screenCenter
    );

    /* add the buttons for the main menu */
    Point2D initPos = {
        config_graphicsWidth / 2,
        160.0f
    };
    Vector2D lineOffset = {0.0f, -40.0f};
    Vector2D selOffset = {0.0f, 1.0f};

    ArrayList buttonHandles = arrayListMake(WindEntity,
        10
    );
    /* the fullscreen button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_fullscreenUnsel",
            "button_fullscreenSel",
            menu_toggleFullscreen,
            (MenuCommandData){0},
            0,
            initPos,
            lineOffset,
            0,
            selOffset,
            true
        )
    );
    /* the mute button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_soundUnsel",
            "button_soundSel",
            menu_toggleSound,
            (MenuCommandData){0},
            0,
            initPos,
            lineOffset,
            1,
            selOffset,
            false
        )
    );
    /* the exit button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_optionsExitUnsel",
            "button_optionsExitSel",
            menu_backWriteSettings,
            (MenuCommandData){.sceneData = {
                scene_main
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
        = menu_navFarDown;
}

/* initializes the entities for the loading screen */
static void initLoadingScreen(
    Game *gamePtr,
    Scene *scenePtr
){
    /* add the loading screen background */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_loading",
        0,
        gameCenter
    );
}

/*
 * adds the player to the specified scene (should be
 * the game scene) and returns the handle of the player
 */
static WindEntity addPlayer(
    Game *gamePtr,
    Scene *scenePtr,
    PlayerData playerData
){
    declareList(componentList, 20);
    addVisible(&componentList);
    addCollidable(&componentList);
    addPosition(
        &componentList,
        config_playerSpawn
    );
    addHitbox(
        &componentList,
        config_playerHitbox
    );
    /* add damage for pickup wisps */
    addDamage(&componentList, 9999999);
    addVelocity(&componentList, (Velocity){0});
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        "p_idle1",
        config_playerDepth,
        ((Vector2D){0.0f, -4.0f})
    );
    addPlayerData(&componentList, playerData);
    addInbound(&componentList, config_playerInbound);
    addPlayerCollisionTarget(
        &componentList,
        collision_player
    );
    addPickupCollisionTarget(
        &componentList,
        collision_none
    );
    addDeathCommand(&componentList, death_player);
    addDeathScripts(&componentList, ((DeathScripts){
        .scriptID1 = stringMakeC("remove_ghost"),
        .scriptID3 = stringMakeC("spawn_player_death")
    }));
    /* add player animations */
    Animations animations = animationListMake();
    Animation leftAnimation = animationMake(true);
    animationAddFrame(&leftAnimation, "p_left1");
    animationAddFrame(&leftAnimation, "p_left2");
    animationAddFrame(&leftAnimation, "p_left3");
    animationAddFrame(&leftAnimation, "p_left4");
    Animation idleAnimation = animationMake(true);
    animationAddFrame(&idleAnimation, "p_idle1");
    animationAddFrame(&idleAnimation, "p_idle2");
    animationAddFrame(&idleAnimation, "p_idle3");
    animationAddFrame(&idleAnimation, "p_idle4");
    Animation rightAnimation = animationMake(true);
    animationAddFrame(&rightAnimation, "p_right1");
    animationAddFrame(&rightAnimation, "p_right2");
    animationAddFrame(&rightAnimation, "p_right3");
    animationAddFrame(&rightAnimation, "p_right4");
    arrayListPushBack(Animation,
        &(animations.animations),
        leftAnimation
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        idleAnimation
    );
    arrayListPushBack(Animation,
        &(animations.animations),
        rightAnimation
    );
    animations.currentIndex = 1;
    animations.idleIndex = 1;
    animations._maxTick = 5;
    addAnimations(&componentList, &animations);

    WindEntity toRet = {0};
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        &toRet
    );
    return toRet;
}

/* gets the init power based on the stage */
static int getInitPower(int stage){
    switch(stage){
        case 1:
            return 0;
        case 2:
            // todo: return config_maxPower / 2;
            return 63;
        case 3:
        case 4:
            return config_maxPower;
        default:
            pgError(
                "Unexpected default stage; "
                SRC_LOCATION
            );
            return 0;
    }
}

/* initializes the entities for the overlay */
static void addOverlayElements(
    Game *gamePtr,
    Scene *scenePtr,
    int currentLives,
    int currentBombs,
    int power
){
    #define initX 242.0f
    #define initY (config_graphicsHeight - 29.0f)
    #define xOffset 13.0f
    #define yOffset -26.0f
    static const Vector2D offset = {xOffset, 0.0f};
    static const Point2D lifeInitPos = {initX, initY};
    static const Point2D bombInitPos
        = {initX, initY + yOffset};
    static const Point2D powerPos
        = {initX + 37.0f, initY + (2 * yOffset)};

    /* add the lives */
    Point2D pos = lifeInitPos;
    for(int i = 0; i < config_maxLives; ++i){
        declareList(componentList, 10);
        if(i < currentLives){
            addVisible(&componentList);
        }
        addPosition(&componentList, pos);
        addSpriteInstructionSimple(
            &componentList,
            gamePtr,
            "overlay_life",
            config_foregroundDepth + 10,
            (Vector2D){0}
        );

        addEntityAndFreeList(
            &componentList,
            scenePtr,
            &(scenePtr->messages.overlayData
                .lifeHandles[i])
        );

        pos = point2DAddVector2D(pos, offset);
    }

    /* add the bombs */
    pos = bombInitPos;
    for(int i = 0; i < config_maxBombs; ++i){
        declareList(componentList, 10);
        if(i < currentBombs){
            addVisible(&componentList);
        }
        addPosition(&componentList, pos);
        addSpriteInstructionSimple(
            &componentList,
            gamePtr,
            "overlay_bomb",
            config_foregroundDepth + 10,
            (Vector2D){0}
        );

        addEntityAndFreeList(
            &componentList,
            scenePtr,
            &(scenePtr->messages.overlayData
                .bombHandles[i])
        );

        pos = point2DAddVector2D(pos, offset);
    }

    /* add the power meter */
    declareList(componentList, 10);
    addVisible(&componentList);
    addPosition(&componentList, powerPos);
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        power == config_maxPower
            ? "overlay_power_max"
            : "overlay_power",
        config_foregroundDepth + 10,
        (Vector2D){0}
    );
    addSubImage(
        &componentList,
        ((SubImage){
            0.0f,
            0.0f,
            80.0f,
            14.0f
        })
    );

    addEntityAndFreeList(
        &componentList,
        scenePtr,
        &(scenePtr->messages.overlayData.powerHandle)
    );

    scenePtr->messages.overlayData.lifeIndex
        = currentLives - 1;
    scenePtr->messages.overlayData.bombIndex
        = currentBombs - 1;

    #undef initX
    #undef initY
    #undef xOffset
    #undef yOffset
}

/* initializes the stage script for the game */
static void addStageScript(
    Game *gamePtr,
    Scene *scenePtr,
    int stage
){
    declareList(componentList, 1);
    Scripts scripts = {0};
    String scriptID = {0};
    switch(stage){
        case 1:
            scriptID = stringMakeC("stage1");
            break;
        case 2:
            scriptID = stringMakeC("stage2");
            break;
        case 3:
            scriptID = stringMakeC("stage3");
            break;
        case 4:
            scriptID = stringMakeC("stage4");
            break;
        default:
            pgError(
                "unexpected default stage; "
                SRC_LOCATION
            );
            break;
    }
    scripts.vm1 = vmPoolRequest();
    unVirtualMachineLoad(
        scripts.vm1,
        resourcesGetScript(
            gamePtr->resourcesPtr,
            &scriptID
        )
    );
    addScripts(&componentList, scripts);
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        NULL
    );
    stringFree(&scriptID);
}

/*
 * signals the game to begin playback of the stage
 * track
 */
static void startStageTrack(
    Game *gamePtr,
    Scene *scenePtr,
    int stage
){
    String *trackIDPtr
        = &(gamePtr->messages.startMusicString);
    stringClear(trackIDPtr);
    switch(stage){
        case 1:
            stringAppendC(trackIDPtr, "02");
            break;
        case 2:
            stringAppendC(trackIDPtr, "04");
            break;
        case 3:
            stringAppendC(trackIDPtr, "06");
            break;
        case 4:
            stringAppendC(trackIDPtr, "08");
            break;
        default:
            pgError(
                "unexpected default stage; "
                SRC_LOCATION
            );
            break;
    }
}

/* initializes the entities for the game */
static void initGame(Game *gamePtr, Scene *scenePtr){
    const GameState gameState
        = gamePtr->messages.gameState;
    WindWorld *worldPtr = &(scenePtr->ecsWorld);
    
    /* create prng */
    scenePtr->messages.prng
        = zmtMake(gameState.prngSeed);
    
    /* add overlay gui */
    addForeground(
        gamePtr,
        scenePtr,
        "overlay_frame",
        0,
        screenCenter
    );

    /* add the player */
    PlayerData playerData = {0};
    if(gamePtr->messages.playerData.isPresent){
        playerData = gamePtr->messages.playerData.data;
        gamePtr->messages.playerData.isPresent = false;
    }
    else{
        playerData.lives = config_initLives;
        playerData.bombs = config_initBombs;
        playerData.continues = config_initContinues;
        playerData.power = getInitPower(
            gamePtr->messages.gameState.stage
        );
        playerData.stateMachine.state
            = player_normal;
        playerData.stateMachine.timer = -1;
    }
    addPlayer(gamePtr, scenePtr, playerData);

    /* add the background */
    #define backgroundWidth 200.0f
    #define backgroundHeight 250.0f

    char *backgroundSpriteName = NULL;
    float pixelScrollPerSec = 0.0f;
    switch(gameState.stage){
        case 1:
            backgroundSpriteName = "gamebg_1";
            pixelScrollPerSec = 15.0f;
            break;
        case 2:
            backgroundSpriteName = "gamebg_2";
            pixelScrollPerSec = 15.0f;
            break;
        case 3:
            backgroundSpriteName = "gamebg_3";
            pixelScrollPerSec = 15.0f;
            break;
        case 4:
            backgroundSpriteName = "gamebg_4";
            pixelScrollPerSec = 15.0f;
            break;
        default:
            pgError(
                "Unexpected stage in init game; "
                SRC_LOCATION
            );
            break;
    }

    declareList(componentList, 10);
    addVisible(&componentList);
    addPosition(
        &componentList,
        gameCenter
    );
    addSpriteInstructionSimple(
        &componentList,
        gamePtr,
        backgroundSpriteName,
        config_backgroundDepth,
        ((Vector2D){0})
    );
    addTilingInstruction(
        &componentList,
        ((Rectangle){
            0.0f,
            0.0f,
            backgroundWidth,
            backgroundHeight
        }),
        ((Point2D){0.0f, 0.0f})
    );
    addTileScroll(
        &componentList,
        ((Vector2D){
            0.0f,
            pixelScrollPerSec
        })
    );
    WindEntity toRet = {0};
    addEntityAndFreeList(
        &componentList,
        scenePtr,
        &toRet
    );
    
    #undef backgroundWidth
    #undef backgroundHeight

    addOverlayElements(
        gamePtr,
        scenePtr,
        playerData.lives,
        playerData.bombs,
        playerData.power
    );

    addStageScript(gamePtr, scenePtr, gameState.stage);

    startStageTrack(
        gamePtr,
        scenePtr,
        gameState.stage
    );
}

/* initializes the entities for the dialogue menu */
static void initDialogue(
    Game *gamePtr,
    Scene *scenePtr
){
    #define portraitY (config_graphicsHeight - 158.0)
    #define portraitLeftX 37.0f
    #define portraitRightX 280.0f

    #define textPos ((Point2D){ \
        80.0f, \
        config_graphicsHeight - 172.0f \
    })

    /* add the background entity */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_dialogue",
        0,
        ((Point2D){
            config_graphicsWidth / 2.0f,
            40.0f
        })
    );

    /* add the left portrait entity */
    declareList(leftList, 3);
    addVisible(&leftList);
    addPosition(
        &leftList,
        ((Point2D){
            portraitLeftX,
            portraitY
        })
    );
    addEntityAndFreeList(
        &leftList,
        scenePtr,
        &(scenePtr->messages.dialogueData
            .leftImageHandle)
    );

    /* add the right portrait entity */
    declareList(rightList, 3);
    addVisible(&rightList);
    addPosition(
        &rightList,
        ((Point2D){
            portraitRightX,
            portraitY
        })
    );
    addEntityAndFreeList(
        &rightList,
        scenePtr,
        &(scenePtr->messages.dialogueData
            .rightImageHandle)
    );

    /* add the text entity */
    declareList(textList, 3);
    addVisible(&textList);
    addPosition(&textList, textPos);
    addEntityAndFreeList(
        &textList,
        scenePtr,
        &(scenePtr->messages.dialogueData.textHandle)
    );

    #undef portraitY
    #undef portraitLeftX
    #undef portraitRightX
    #undef textPos
}

/* initializes the entities for the pause menu */
static void initPauseMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    /* add the pause menu background */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_pause",
        0,
        gameCenter
    );

    /* add the buttons for the pause menu */
    Point2D initPos = {
        gameCenter.x,
        149.0f
    };
    Vector2D lineOffset = {0.0f, -30.0f};
    Vector2D selOffset = {0.0f, 1.0f};

    ArrayList buttonHandles = arrayListMake(WindEntity,
        10
    );
    /* the resume button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_resumeUnsel",
            "button_resumeSel",
            menu_backTo,
            (MenuCommandData){
                .sceneData.sceneID = scene_game
            },
            0,
            initPos,
            lineOffset,
            0,
            selOffset,
            true
        )
    );
    /* the restart button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_restartUnsel",
            "button_restartSel",
            menu_restartGame,
            (MenuCommandData){0},
            0,
            initPos,
            lineOffset,
            1,
            selOffset,
            false
        )
    );
    /* the retire button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_retireUnsel",
            "button_retireSel",
            menu_gameOver,
            (MenuCommandData){0},
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
        = menu_navFarDown;
}

/* initializes the entities for the continue menu */
static void initContinueMenu(
    Game *gamePtr,
    Scene *scenePtr
){
    /* add the continue menu background */
    addBackground(
        gamePtr,
        scenePtr,
        "menubg_continue",
        0,
        gameCenter
    );

    /* add the buttons for the continue menu */
    Point2D initPos = {gameCenter.x, 130.0f};
    Vector2D lineOffset = {0.0f, -30.0f};
    Vector2D selOffset = {0.0f, 1.0f};

    ArrayList buttonHandles = arrayListMake(WindEntity,
        10
    );
    /* the accept button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_acceptUnsel",
            "button_acceptSel",
            menu_backTo,
            (MenuCommandData){
                .sceneData.sceneID = scene_game
            },
            0,
            initPos,
            lineOffset,
            0,
            selOffset,
            true
        )
    );
    /* the decline button */
    arrayListPushBack(WindEntity,
        &buttonHandles,
        addButtonInLine(
            gamePtr,
            scenePtr,
            "button_declineUnsel",
            "button_declineSel",
            menu_gameOver,
            (MenuCommandData){0},
            0,
            initPos,
            lineOffset,
            1,
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

    /* get player data from continue system */
    assertTrue(
        gamePtr->messages.playerData.isPresent,
        "error: expect player data to be present; "
        SRC_LOCATION
    );
    PlayerData playerData
        = gamePtr->messages.playerData.data;
    gamePtr->messages.playerData.data
        = (PlayerData){0};
    gamePtr->messages.playerData.isPresent = false;
    
    float iconY = 166.0f;
    float xShift = 15.0f;
    if(playerData.continues == 2){
        addBackground(
            gamePtr,
            scenePtr,
            "overlay_continue",
            100,
            (Point2D){gameCenter.x + xShift, iconY}
        );
    }
    if(playerData.continues >= 1){
        addBackground(
            gamePtr,
            scenePtr,
            "overlay_continue",
            100,
            (Point2D){gameCenter.x - xShift, iconY}
        );
    }

    scenePtr->messages.backMenuCommand
        = menu_navFarDown;
}

/* initializes each scene */
void initSystem(Game *gamePtr, Scene *scenePtr){
    if(scenePtr->messages.initFlag){
        return;
    }
    scenePtr->messages.initFlag = true;

    /* putting the vm pool initializer here */
    vmPoolInit(
        getNativeFuncSet(), 
        &(gamePtr->resourcesPtr->scriptResourcesPtr
            ->userFuncSet)
    );

    switch(scenePtr->id){
        case scene_main:
            initMainMenu(gamePtr, scenePtr);
            break;
        case scene_difficulty:
            initDiffMenu(gamePtr, scenePtr);
            break;
        case scene_stage:
            initStageMenu(gamePtr, scenePtr);
            break;
        //todo: init music scene
        //scene_music,
        case scene_options:
            initOptionsMenu(gamePtr, scenePtr);
            break;
        case scene_loading:
            initLoadingScreen(gamePtr, scenePtr);
            break;
        case scene_game:
            initGame(gamePtr, scenePtr);
            break;
        case scene_dialogue:
            initDialogue(gamePtr, scenePtr);
            break;
        case scene_pause:
            initPauseMenu(gamePtr, scenePtr);
            break;
        case scene_continue:
            initContinueMenu(gamePtr, scenePtr);
            break;
    /*
    scene_credits, //todo: init credits scene
    */
        default:
            pgError(
                "unrecognized scene in init system; "
                SRC_LOCATION
            );
            return;
    }
}