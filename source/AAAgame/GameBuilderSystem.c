#include "GameBuilderSystem.h"

uint32_t currentTimeToPrngSeed(){
    TimePoint currentTime = getCurrentTime();
    if(sizeof(currentTime) < sizeof(uint32_t)){
        pgError(
            "timepoint is smaller than uint32_t; "
            SRC_LOCATION
        );
        return 0;
    }
    return *((uint32_t*)(&currentTime));
}

void finalizeGameState(GameState *gameStatePtr){
    if(gameStatePtr->gameMode == game_story){
        gameStatePtr->stage = 1;
    }
    gameStatePtr->prngSeed = currentTimeToPrngSeed();
}

/*
 * Builds the game metadata piece by piece depending
 * on the menu navigation of the player
 */
void gameBuilderSystem(Game *gamePtr, Scene *scenePtr){
    GameState *gameStatePtr
        = &(gamePtr->messages.gameState);
    switch(scenePtr->messages.gameBuilderCommand){
        case gb_start:
            gameStatePtr->gameMode = game_story;
            break;
        case gb_practice:
            gameStatePtr->gameMode = game_practice;
            break;
        case gb_normal:
            gameStatePtr->difficulty = game_normal;
            if(gameStatePtr->gameMode == game_story){
                finalizeGameState(gameStatePtr);
            }
            break;
        case gb_hard:
            gameStatePtr->difficulty = game_hard;
            if(gameStatePtr->gameMode == game_story){
                finalizeGameState(gameStatePtr);
            }
            break;
        case gb_lunatic:
            gameStatePtr->difficulty = game_lunatic;
            if(gameStatePtr->gameMode == game_story){
                finalizeGameState(gameStatePtr);
            }
            break;
        case gb_stage1:
            gameStatePtr->stage = 1;
            finalizeGameState(gameStatePtr);
            break;
        case gb_stage2:
            gameStatePtr->stage = 2;
            finalizeGameState(gameStatePtr);
            break;
        case gb_stage3:
            gameStatePtr->stage = 3;
            finalizeGameState(gameStatePtr);
            break;
        case gb_stage4:
            gameStatePtr->stage = 4;
            finalizeGameState(gameStatePtr);
            break;
        case gb_reset:
            /* reset stage to 1 if needed and prng */
            finalizeGameState(gameStatePtr);
            break;
        case gb_none:
            /* do nothing */
            break;
        default:
            pgError(
                "unexpected default game builder "
                "command; " SRC_LOCATION
            );
            break;
    }
}