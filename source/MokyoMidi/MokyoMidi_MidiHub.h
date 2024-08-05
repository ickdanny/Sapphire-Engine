#ifndef MOKYOMIDI_MIDIHUB_H
#define MOKYOMIDI_MIDIHUB_H

#include <stdbool.h>

#include "MokyoMidi_MidiOut.h"
#include "MokyoMidi_MidiSequence.h"
#include "MokyoMidi_MidiSequencer.h"

/* 
 * The MidiHub provides the interface for users to
 * play midi sequences
 */
typedef struct MidiHub{
    MMMidiOut *midiOutPtr;
    MidiSequencer midiSequencer;
    bool muted;
} MidiHub;

/*
 * Constructs a new MidiHub and returns it by value.
 * Should not be called twice unless the first MidiHub
 * has been closed
 */
MidiHub midiHubMake(bool muted);

/* 
 * Starts playing the given midi sequence. Replaces
 * the previously playing sequence
 */
void midiHubStart(
    MidiHub *midiHubPtr, 
    MidiSequence *midiSequencePtr
);

/* Stops playing the current midi sequence */
void midiHubStop(MidiHub *midiHubPtr);

/* 
 * If midi is currently muted, unmutes it. Otherwise,
 * mutes it
 */
void midiHubToggleMute(MidiHub *midiHubPtr);

/*
 * Returns true if midi is currently muted, false
 * otherwise
 */
bool midiHubIsMuted(const MidiHub *midiHubPtr);

/*
 * Closes the given MidiHub and frees all associated
 * resources. Using a MidiHub after it has been closed
 * is undefined behavior
 */
void midiHubFree(MidiHub *midiHubPtr);

#endif