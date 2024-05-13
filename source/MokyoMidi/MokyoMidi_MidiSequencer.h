#ifndef MOKYOMIDI_MIDISEQUENCER_H
#define MOKYOMIDI_MIDISEQUENCER_H

//todo: clean up 
#include <chrono>
#include <mutex>
#include "windowsInclude.h"
#include "mmeInclude.h"

#include "MokyoMidi_Constants.h"
#include "MokyoMidi_MidiSequence.h"
#include "MokyoMidi_MidiOut.h"
#include "Utility/Scheduling.h"

/* 
 * A MidiSequener handles sending midi data to output
 * with the timing specified in a midi sequence so as
 * to actually produce music; not threadsafe
 */
typedef struct MidiSequencer{
    MidiOut *midiOutPtr;

    /* playback fields */

    /* a weak pointer to the midi sequence */
    MidiSequence *sequencePtr;
    /* a poiter to the current event unit */
    _EventUnit *currentPtr;
    /* 
     * a pointer to the first event unit after the
     * loop, or null if no loop has been encountered
     */
    _EventUnit *loopPtr;
    uint32_t microsecondsPerBeat;
    uint32_t timePerTick100ns; //todo: platform depends
} MidiSequencer;

/* 
 * Constructs and returns a new MidiSequencer with
 * the specified MidiOut by value
 */
MidiSequencer midiSequencerMake(MidiOut *midiOutPtr);

/* Begins playing back the specified midi sequence */
void midiSequencerStart(
    MidiSequencer *sequencerPtr,
    MidiSequence *sequencePtr
);

/* Stops playback */
void midiSequencerStop(MidiSequencer *sequencerPtr);

	private:
		//typedefs
		
		//on Windows, steady_clock wraps the performance counter
		using clockType = std::chrono::steady_clock;
		using timePointType = clockType::time_point;
				
		//threading fields
		std::thread playbackThread {};
		utility::EventHandle wakeupSwitch {};
		std::atomic_bool running { false };        //our flag, synchronizes our threads

#endif