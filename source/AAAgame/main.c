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

//todo test function
static void testBitset(){
    #define bufferSize 500
    char buffer[bufferSize] = {0};
    Bitset bitset = bitsetMake(0);
    printBitset(&bitset, buffer, bufferSize);
    printf("new bitset: %s\n", buffer);

    bool prev = bitsetSet(&bitset, 32);
    assertFalse(prev, "expect prev 0");
    printBitset(&bitset, buffer, bufferSize);
    printf("set 32: %s\n", buffer);
    printf("size: %lu\n", bitset._blockArray.size);

    prev = bitsetUnset(&bitset, 32);
    assertTrue(prev, "expect prev 1");
    printBitset(&bitset, buffer, bufferSize);
    printf("unset 32: %s\n", buffer);
    printf("size: %lu\n", bitset._blockArray.size);
    
    prev = bitsetFlip(&bitset, 73);
    assertFalse(prev, "expect prev 0");
    printBitset(&bitset, buffer, bufferSize);
    printf("flip 73: %s\n", buffer);
    printf("size: %lu\n", bitset._blockArray.size);

    prev = bitsetFlip(&bitset, 73);
    assertTrue(prev, "expect prev 1");
    printBitset(&bitset, buffer, bufferSize);
    printf("flip 73 again: %s\n", buffer);
    printf("size: %lu\n", bitset._blockArray.size);

    bitsetSet(&bitset, 14);
    assertTrue(bitsetGet(&bitset, 14), "failed get");

    bitsetSet(&bitset, 43);
    bitsetClear(&bitset);
    printBitset(&bitset, buffer, bufferSize);
    printf("clear: %s\n", buffer);

    assertFalse(bitsetAny(&bitset), "any fail");
    bitsetSet(&bitset, 100);
    assertTrue(bitsetAny(&bitset), "any fail 2");

    bitsetClear(&bitset);

    bitsetSet(&bitset, 0);
    bitsetSet(&bitset, 13);
    bitsetSet(&bitset, 100);
    assertTrue(bitsetCount(&bitset) == 3, "count fail");
    printBitset(&bitset, buffer, bufferSize);
    printf("bitset 1: %s\n", buffer);

    Bitset bitset2 = bitsetMake(0);
    bitsetSet(&bitset2, 0);
    bitsetSet(&bitset2, 5);
    bitsetSet(&bitset2, 160);
    printBitset(&bitset2, buffer, bufferSize);
    printf("bitset 2: %s\n", buffer);

    bitsetAnd(&bitset, &bitset2);
    printBitset(&bitset, buffer, bufferSize);
    printf("AND: %s\n", buffer);

    //todo: more tests

    bitsetFree(&bitset);

    printf("test concluded\n");

    #undef bufferSize
}

/* The entry point for the game */
int main(){
    testBitset();
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