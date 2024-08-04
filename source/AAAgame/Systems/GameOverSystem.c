#include "GameOverSystem.h"

/* Handles exiting the game when the player loses */
void gameOverSystem(Game *gamePtr, Scene *scenePtr){
    if(scenePtr->messages.playerStateEntry.state
        == player_gameOver
    ){
        String *trackIDPtr = &(gamePtr->messages
            .startMusicString);
        stringClear(trackIDPtr);
        stringAppendC(trackIDPtr, "01");

        /* send back to correct menu */
        SceneID backTo = scene_numScenes;
		switch(gamePtr->messages.gameState.gameMode){
			case game_story:
				backTo = scene_main;
				break;
			case game_practice:
				backTo = scene_stage;
				break;
			default:
                pgError(
                    "Unexpected default game mode; "
                    SRC_LOCATION
                );
		}
        gamePtr->messages.sceneExitToID = backTo;
    }
}