#ifndef MOKYOMIDI_MIDISEQUENCE_H
#define MOKYOMIDI_MIDISEQUENCE_H

#include <stdint.h>

#include "Constructure.h"

#define _noLoopStart (~((size_t)0))

/* Represents a single MIDI event */
typedef struct _EventUnit{
    uint32_t deltaTime;
    uint32_t event;
} _EventUnit;

/* 
 * Stores both the midi data associated with a midi
 * sequence as well as data used for playback.
 */
typedef struct MidiSequence{
    uint32_t ticks;

    /* 
     * Offset into eventTrack for loopStart, max value
     * for invalid. Guaranteed to actually be an event
     * since we the marker will show in the event
     * track.
     */
    size_t loopStartOffset;

    ArrayList eventTrack;
} MidiSequence;

/*
 * Reads the specified midi file and returns a
 * MidiSequence by value, ready for playback
 */
MidiSequence parseMidiFile(const char *filename);

/* Frees the given MidiSequence */
void midiSequenceFree(MidiSequence *midiSequencePtr);

#endif