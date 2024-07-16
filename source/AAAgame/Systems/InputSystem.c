#include "InputSystem.h"

#include "MenuNavigationCommand.h"
#include "GameCommand.h"

/* checks to see if the specified key was pressed */
#define pressed(GAMEPTR, KEY) \
    (tfKeyTableGetState( \
        (GAMEPTR)->keyTablePtr, \
        KEY \
    ) == tf_statePress)

/* checks to see if the specified key is down */
#define down(GAMEPTR, KEY) \
    (pressed(GAMEPTR, KEY) || \
        (tfKeyTableGetState( \
            (GAMEPTR)->keyTablePtr, \
            KEY \
        ) == tf_stateDown))

/* parses input for a menu scene */
void parseMenuInput(
    Game *gamePtr,
    Scene *scenePtr
){
    ArrayList *menuNavigationCommandsPtr
        = &(scenePtr->messages.menuNavigationCommands);
    arrayListClear(MenuNavigationCommand,
        menuNavigationCommandsPtr
    );

    if(pressed(gamePtr, tf_escape)
        || pressed(gamePtr, tf_x)
        || pressed(gamePtr, tf_period)
    ){
        arrayListPushBack(MenuNavigationCommand,
            menuNavigationCommandsPtr,
            menuNav_back
        );
    }
    if(pressed(gamePtr, tf_z)
        || pressed(gamePtr, tf_slash)
    ){
        arrayListPushBack(MenuNavigationCommand,
            menuNavigationCommandsPtr,
            menuNav_select
        );
    }
    if(pressed(gamePtr, tf_up)
        || pressed(gamePtr, tf_w)
    ){
        arrayListPushBack(MenuNavigationCommand,
            menuNavigationCommandsPtr,
            menuNav_up
        );
    }
    if(pressed(gamePtr, tf_down)
        || pressed(gamePtr, tf_s)
    ){
        arrayListPushBack(MenuNavigationCommand,
            menuNavigationCommandsPtr,
            menuNav_down
        );
    }
    if(pressed(gamePtr, tf_left)
        || pressed(gamePtr, tf_a)
    ){
        arrayListPushBack(MenuNavigationCommand,
            menuNavigationCommandsPtr,
            menuNav_left
        );
    }
    if(pressed(gamePtr, tf_right)
        || pressed(gamePtr, tf_d)
    ){
        arrayListPushBack(MenuNavigationCommand,
            menuNavigationCommandsPtr,
            menuNav_right
        );
    }

    /* menus have no transparency */
    tfKeyTableLockAll(gamePtr->keyTablePtr);
}

/* parses input for a game scene */
void parseGameInput(
    Game *gamePtr,
    Scene *scenePtr
){
    ArrayList *gameCommandsPtr
        = &(scenePtr->messages.gameCommands);
    arrayListClear(GameCommand, gameCommandsPtr);
    if(pressed(gamePtr, tf_escape)){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_pause
        );
    }
    if(down(gamePtr, tf_shift)){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_focus
        );
    }
    if(down(gamePtr, tf_z)
        || down(gamePtr, tf_slash)
    ){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_shoot
        );
    }
    if(pressed(gamePtr, tf_x)
        || pressed(gamePtr, tf_period)
    ){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_bomb
        );
    }
    if(down(gamePtr, tf_up)
        || down(gamePtr, tf_w)
    ){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_up
        );
    }
    if(down(gamePtr, tf_down)
        || down(gamePtr, tf_s)
    ){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_down
        );
    }
    if(down(gamePtr, tf_left)
        || down(gamePtr, tf_a)
    ){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_left
        );
    }
    if(down(gamePtr, tf_right)
        || down(gamePtr, tf_d)
    ){
        arrayListPushBack(GameCommand,
            gameCommandsPtr,
            game_right
        );
    }

    /* game has no transparency */
    tfKeyTableLockAll(gamePtr->keyTablePtr);
}

/* parses input for a dialogue scene */
void parseDialogueInput(
    Game *gamePtr,
    Scene *scenePtr
){
    //todo parse dialogue input
    /*
    auto& readDialogueFlagChannel{
			scene.getChannel(SceneTopics::readDialogueFlag)
		};
		readDialogueFlagChannel.clear();

		if (isJustPressed(KeyValues::k_z) 
			|| isJustPressed(KeyValues::k_slash)
			|| isBeingPressed(KeyValues::k_control)) {
			readDialogueFlagChannel.addMessage();
		}

		keyInputTablePointer->lock(KeyValues::k_z);
		keyInputTablePointer->lock(KeyValues::k_slash);
		keyInputTablePointer->lock(KeyValues::k_x);
		keyInputTablePointer->lock(KeyValues::k_period);
		keyInputTablePointer->lock(KeyValues::k_control);
		keyInputTablePointer->lock(KeyValues::k_escape);	//simplify things; no pause
        */
}

/*
 * parses input from the keyboard and writes the
 * needed messages to the scene
 */
void inputSystem(Game *gamePtr, Scene *scenePtr){
    switch(scenePtr->id){
        case scene_main:
        case scene_difficulty:
        case scene_stage:
        case scene_music:
        case scene_options:
        case scene_pause:
        case scene_continues:
            parseMenuInput(gamePtr, scenePtr);
            break;
        case scene_game:
            parseGameInput(gamePtr, scenePtr);
            break;
        case scene_dialogue:
            parseDialogueInput(gamePtr, scenePtr);
            break;
        case scene_loading:
        case scene_credits:
            /* do nothing for scenes w/o input */
            return;
        default:
            pgError(
                "unexpected default in input system; "
                SRC_LOCATION
            );
            break;
    }
}