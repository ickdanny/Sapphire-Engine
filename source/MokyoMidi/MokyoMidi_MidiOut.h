#ifndef MOKYOMIDI_MIDIOUT_H
#define MOKYOMIDI_MIDIOUT_H

#include <stdint.h>

#ifdef __APPLE__

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#endif /* end __APPLE__ */

#ifdef WIN32

#include "Trifecta_Win32.h"
#include "mmeapi.h"

#endif /* end WIN32 */

/*
 * The MMMidiOut class provides wrapper functions for
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
} MMMidiOut;

/*
 * Constructs a new MMMidiOut and returns it by value.
 */
MMMidiOut midiOutMake();

/* Starts a MidiOut's output */
void mmMidiOutStart(MMMidiOut *midiOutPtr);

/* Stops a MidiOut's output */
void mmMidiOutStop(MMMidiOut *midiOutPtr);

/* Outputs a short message */
void mmMidiOutShortMsg(
    MMMidiOut *midiOutPtr, 
    uint32_t output
);

/* Outputs a short message on all channels */
void mmMidiOutShortMsgOnAllChannels(
    MMMidiOut *midiOutPtr,
    uint32_t output
);

/* Outputs a control change message on all channels */
void mmMidiOutControlChangeOnAllChannels(
    MMMidiOut *midiOutPtr,
    uint32_t data
);

/* Outputs a system exclusive message */
void mmMidiOutSysex(
    MMMidiOut *midiOutPtr,
    const void* bufferPtr,
    uint32_t byteLength
);

/* Resets the given MMMidiOut */
void mmMidiOutReset(MMMidiOut *midiOutPtr);

/* Frees the given MMMidiOut */
void mmMidiOutFree(MMMidiOut *midiOutPtr);

#endif