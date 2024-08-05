#ifndef MOKYOMIDI_MIDISEQUENCER_H
#define MOKYOMIDI_MIDISEQUENCER_H

#include <stdatomic.h>

#include "Trifecta.h"

#include "MokyoMidi_Constants.h"
#include "MokyoMidi_MidiSequence.h"
#include "MokyoMidi_MidiOut.h"

/* 
 * A MidiSequener handles sending midi data to output
 * with the timing specified in a midi sequence so as
 * to actually produce music; not threadsafe
 */
typedef struct MidiSequencer{
    MMMidiOut *midiOutPtr;

    /* playback fields */

    /* a weak pointer to the midi sequence */
    MidiSequence *sequencePtr;
    /* a pointer to the current event unit */
    _EventUnit *currentPtr;
    /* 
     * a pointer to the first event unit after the
     * loop, or null if no loop has been encountered
     */
    _EventUnit *loopPtr;
    uint32_t microsecondsPerBeat;
    uint32_t timePerTick100ns;

    /* threading fields */
    Thread playbackThread;
    atomic_bool running;
} MidiSequencer;

/* 
 * Constructs and returns a new MidiSequencer with
 * the specified MMMidiOut by value
 */
MidiSequencer midiSequencerMake(MMMidiOut *midiOutPtr);

/* Begins playing back the specified midi sequence */
void midiSequencerStart(
    MidiSequencer *sequencerPtr,
    MidiSequence *sequencePtr
);

/* Stops playback */
void midiSequencerStop(MidiSequencer *sequencerPtr);

/* Frees the given MidiSequencer */
void midiSequencerFree(MidiSequencer *sequencerPtr);

#endif