#include "GameOverSystem.h"

/* Handles exiting the game when the player loses */
void gameOverSystem(Game *gamePtr, Scene *scenePtr){
    if(scenePtr->messages.playerStateEntry.state
        == player_gameOver
    ){
        String *trackIdPtr = &(gamePtr->messages
            .startMusicString);
        stringClear(trackIdPtr);
        stringAppendC(trackIdPtr, "01");

        /* send back to correct menu */
        SceneId backTo = scene_numScenes;
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
        gamePtr->messages.sceneExitToId = backTo;
    }
}