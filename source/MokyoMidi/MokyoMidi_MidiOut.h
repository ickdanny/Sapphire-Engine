#ifndef MOKYOMIDI_MIDIOUT_H
#define MOKYOMIDI_MIDIOUT_H

#include <stdint.h>

/*
 * The MidiOut class provides wrapper functions for
 * basic midi output.
 */
typedef struct MidiOut{


    #ifdef __APPLE__
    
    #endif
    //todo
} MidiOut;

/*
 * Constructs a new MidiOut and returns it by value.
 */
MidiOut midiOutMake();

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
//todo interface
void midiOutSystemExclusive(MIDIHDR* midiHDR);

/* Resets the given MidiOut */
void midiOutReset(MidiOut *midiOutPtr);

//void midiOutOpen, void midiOutClose

#endif