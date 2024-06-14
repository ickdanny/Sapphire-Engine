#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <time.h>

#include "ZMath.h"
#include "Constructure.h"
#include "MokyoMidi.h"
#include "Trifecta.h"

#include "Config.h"
#include "Game.h"
#include "GameLoop.h"
#include "Settings.h"

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
    Settings settings;
    /* todo: resources */
    TFWindow window;
    TFKeyTable keyTable;
    MidiHub midiHub;
    Game game;
    GameLoop gameLoop;
} Engine;

/* Frees the specified Engine */
void engineFree(Engine *enginePtr){
    writeSettingsToFile(
        &(enginePtr->settings),
        config_settingsFileName
    );
    //todo: free resources
    tfWindowFree(&(enginePtr->window));
    tfKeyTableFree(&(enginePtr->keyTable));
    midiHubFree(&(enginePtr->midiHub));
    gameFree(&(enginePtr->game));
}

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
    gameUpdate(&(enginePtr->game));
    tfWindowPumpMessages(&(enginePtr->window));
}

/* Draws the specified Engine passed as a void ptr */
void renderCallback(void *voidPtr){
    Engine *enginePtr = (Engine*)voidPtr;
    gameRender(&(enginePtr->game));
    tfWindowRender(&(enginePtr->window));
}

//todo test func
static void testMT(){
    ZMT prng = zmtMake(time(NULL));
    #define LOW 100000
    #define HIGH (((~((uint64_t)0)) / 4) * 3)
    int n = 1000;
    uint64_t randNum = 0;
    for(int i = 0; i < n; ++i){
        randNum = zmtULongDie(&prng, LOW, HIGH);
        printf("%3d: %lu\n", i, randNum);
        if(randNum < LOW || randNum > HIGH){
            pgError("limit failure");
        }
    }

    zmtFree(&prng);
}

/* The entry point for the game */
int main(){
    testMT();
    exit(0);
    Engine engine = {0};

    /* read settings */
    engine.settings = readOrCreateSettingsFromFile(
        config_settingsFileName
    );

    /* init window */
    engine.window = tfWindowMake(
        engine.settings.fullscreen,
        config_windowName,
        config_windowWidth,
        config_windowHeight,
        config_graphicsWidth,
        config_graphicsHeight,
        &engine /* user ptr to engine for callbacks */
    );

    /* init key table input */
    engine.keyTable = tfKeyTableMake(&(engine.window));

    /* init MIDI */
    engine.midiHub = midiHubMake(
        engine.settings.muted
    );

    engine.game = gameMake(
        &(engine.settings),
        &(engine.window),
        &(engine.keyTable),
        &(engine.midiHub)
    );
    //todo: set game fullscreen callback
    //todo: set game write settings callback

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

    /* clean up after game ends */
    engineFree(&engine);

    printf("main completed\n");
    return 0;
}