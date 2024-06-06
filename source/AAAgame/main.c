#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <time.h>

#include "ZMath.h"
#include "Constructure.h"
#include "MokyoMidi.h"
#include "Trifecta.h"

#include "Config.h"
#include "GameLoop.h"

#include <unistd.h>

void printInt(int *toPrint){
    printf("%d\n", *toPrint);
}

void printString(const String *toPrint){
    printf("%s\n", toPrint->_ptr);
}

void printWideString(const WideString *toPrint){
    printf("%ls\n", toPrint->_ptr);
}

size_t intHash(const void *intPtr){
    return *((int*)intPtr);
}

bool intEquals(
    const void *intPtr1, 
    const void *intPtr2
){
    return *((int*)intPtr1) == *((int*)intPtr2);
}

#define intToLong(i) ((long)i)

/* A struct holding all the elements of the engine */
typedef struct Engine{
    /* todo: settings */
    /* todo: resources */
    TFWindow window;
    /* todo: input */
    MidiHub midiHub;
    /* todo: game */
    GameLoop gameLoop;
} Engine;

/*
 * A callback which stops the gameloop in the
 * specified Engine passed as a void ptr
 */
void stopGameLoopCallback(void *voidPtr){
    Engine *enginePtr = (Engine*)voidPtr;
    gameLoopStop(&(enginePtr->gameLoop));
}

/* Updates the specified Engine passed as a void ptr */
void updateCallback(void *voidPtr){
    Engine *enginePtr = (Engine*)voidPtr;
    //todo: update game
    //todo: pump messages? idk
}

/* Draws the specified Engine passed as a void ptr */
void renderCallback(void *voidPtr){
    Engine *enginePtr = (Engine*)voidPtr;
    //todo: render game
    tfWindowRender(&(enginePtr->window));
}

/* The entry point for the game */
int main(){
    Engine engine = {0};
    engine.window = tfWindowMake(
        true, //todo fullscreen from settings
        config_windowName,
        config_graphicsWidth,
        config_graphicsHeight,
        &engine /* user ptr to engine for callbacks */
    );

    engine.gameLoop = gameLoopMake(
        config_updatesPerSecond,
        config_maxUpdatesPerFrame,
        &engine, /* update user ptr */
        &engine, /* render user ptr */
        updateCallback,
        renderCallback
    );

    /* set callbacks */
    tfWindowSetExitCallback(
        &(engine.window),
        stopGameLoopCallback
    );
    /* todo: game set exit callback */

    /* make visible and start running */
    tfWindowMakeVisible(&(engine.window));
    gameLoopRun(&(engine.gameLoop));

    /* after game has ended clean up */
    tfWindowFree(&(engine.window));

    //todo: write settings

    printf("main completed\n");
    return 0;
}