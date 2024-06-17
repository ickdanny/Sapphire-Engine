#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <time.h>

#include "ZMath.h"
#include "Constructure.h"
#include "MokyoMidi.h"
#include "Trifecta.h"
#include "BLoader.h"

#include "Config.h"
#include "Game.h"
#include "GameLoop.h"
#include "Resources.h"
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

#define intToLong(i) ((long)i)

/* A struct holding all the elements of the engine */
typedef struct Engine{
    Settings settings;
    Resources resources;
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
    resourcesFree(&(enginePtr->resources));
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

//todo: test func
void testSparseSet(){
    SparseSet sparseSet = sparseSetMake(
        float,
        50,
        5
    );
    assertTrue(
        sparseSet.sparseCapacity == 50, 
        "capacity should be 50"
    );
    assertTrue(
        sparseSet._size == 0,
        "size should be 0"
    );

    float four = 4.0f;

    sparseSetSet(float, &sparseSet, 20, 1.0f);
    sparseSetSet(float, &sparseSet, 10, 2.0f);
    sparseSetSet(float, &sparseSet, 40, 3.0f);
    sparseSetSetPtr(float, &sparseSet, 4, &four);

    assertTrue(
        sparseSetContains(float, &sparseSet, 10),
        "should contain 10"
    );
    assertTrue(
        sparseSetContains(float, &sparseSet, 4),
        "should contain 4"
    );

    assertTrue(
        sparseSetGet(float, &sparseSet, 40) == 3.0f,
        "should retrieve 3.0f"
    );

    sparseSetRemove(float, &sparseSet, 20);

    assertFalse(
        sparseSetContains(float, &sparseSet, 20),
        "should not contain 20"
    );

    SparseSetItr itr 
        = sparseSetItr(float, &sparseSet);
    assertTrue(
        sparseSetItrHasNext(float, &itr),
        "itr should have next"
    );
    float temp = sparseSetItrNext(float, &itr);
    assertTrue(
        temp == four,
        "expect first element to be four"
    );
    assertTrue(
        sparseSetItrIndex(float, &itr) == 4,
        "expect last index to be 4"
    );
    assertTrue(
        sparseSetItrNext(float, &itr) == 2.0f,
        "expect first element to be 2.0f"
    );
    assertTrue(
        sparseSetItrNext(float, &itr) == 3.0f,
        "expect first element to be 3.0f"
    );
    assertFalse(
        sparseSetItrHasNext(float, &itr),
        "itr should not have next"
    );

    sparseSetClear(float, &sparseSet);
    assertTrue(
        sparseSet.sparseCapacity == 50, 
        "capacity should be 50"
    );
    assertTrue(
        sparseSet._size == 0,
        "size should be 0"
    );

    for(int i = 0; i < 50; ++i){
        /* test to see if we can fill capacity */
        sparseSetSet(float, &sparseSet, i, (float)i);
    }
    itr = sparseSetItr(float, &sparseSet);
    for(int i = 0; i < 50; ++i){
        assertTrue(
            sparseSetItrNext(float, &itr) == (float)i,
            "should get i back"
        );
    }
    assertFalse(
        sparseSetItrHasNext(float, &itr),
        "itr should not have next"
    );

    sparseSetFree(float, &sparseSet);

    assertNull(sparseSet._densePtr, "dense null");
    assertNull(sparseSet._sparsePtr, "sparse null");
    assertNull(sparseSet._reflectPtr, "reflect null");

    printf("sparse set test concluded\n");
}

/* The entry point for the game */
int main(){
    testSparseSet();
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

    /* init resources (after window for OpenGL) */
    engine.resources = resourcesMake();
    resourcesLoadDirectory(
        &(engine.resources),
        "res/image"
    );
    resourcesLoadDirectory(
        &(engine.resources),
        "res/midi"
    );
    //todo: load more directories as needed

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
    // todo: game set exit callback

    /* make visible and start running */
    tfWindowMakeVisible(&(engine.window));
    gameLoopRun(&(engine.gameLoop));

    /* clean up after game ends */
    engineFree(&engine);

    #ifdef _DEBUG
    printf("main completed\n");
    #endif

    return 0;
}