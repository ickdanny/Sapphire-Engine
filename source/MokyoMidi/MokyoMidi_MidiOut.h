#ifndef MOKYOMIDI_MIDIOUT_H
#define MOKYOMIDI_MIDIOUT_H

#include <stdint.h>

#ifdef __APPLE__

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#endif /* end __APPLE__ */

#ifdef WIN32

#include "Trifecta_Win32.h"

/* name collisions */

#define midiOutShortMsg _midiOutShortMsg
#define midiOutReset _midiOutReset

#include "mmeapi.h"

#undef midiOutShortMsg
#undef midiOutReset

#endif /* end WIN32 */

/*
 * The MidiOut class provides wrapper functions for
 * basic midi output.
 */
typedef struct MidiOut{
    #ifdef __APPLE__

    AUGraph graph;
    AudioUnit synthUnit;

    #endif /* end __APPLE__ */

    #ifdef WIN32
    HMIDIOUT midiOutHandle;
    #endif /* end WIN32 */
    //todo other platforms
} MidiOut;

/*
 * Constructs a new MidiOut and returns it by value.
 */
MidiOut midiOutMake();

/* Starts a MidiOut's output */
void midiOutStart(MidiOut *midiOutPtr);

/* Stops a MidiOut's output */
void midiOutStop(MidiOut *midiOutPtr);

/* Outputs a short message */
void midiOutShortMsg(
    MidiOut *midiOutPtr, 
    uint32_t output
);

/* Outputs a short message on all channels */
void midiOutShortMsgOnAllChannels(
    MidiOut *midiOutPtr,
    uint32_t output
);

/* Outputs a control change message on all channels */
void midiOutControlChangeOnAllChannels(
    MidiOut *midiOutPtr,
    uint32_t data
);

/* Outputs a system exclusive message */
void midiOutSysex(
    MidiOut *midiOutPtr,
    const void* bufferPtr,
    uint32_t byteLength
);

/* Resets the given MidiOut */
void midiOutReset(MidiOut *midiOutPtr);

/* Frees the given MidiOut */
void midiOutFree(MidiOut *midiOutPtr);

#endif