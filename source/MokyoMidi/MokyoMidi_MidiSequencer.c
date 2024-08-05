#include "MokyoMidi_MidiSequencer.h"

#include "ZMath.h"
#include "Trifecta.h"

#define ratio100nsToSeconds ((uint64_t)10000000)

/* 
 * Constructs and returns a new MidiSequencer with
 * the specified MMMidiOut by value
 */
MidiSequencer midiSequencerMake(MMMidiOut *midiOutPtr){
    MidiSequencer toRet = {0};
    toRet.midiOutPtr = midiOutPtr;
    atomic_init(&(toRet.running), false);

    return toRet;
}

static
uint32_t microsecondsPerBeatToTimePerTick100ns(
	uint32_t microsecondsPerBeat,
	uint16_t ticks
) {
	return (10 * microsecondsPerBeat) / ticks;
}

static
uint32_t calculateInitialTimePerTick100ns(
    uint16_t ticks
){
	/* a leading 1 means SMPTE FPS time */
    if(ticks & 0x8000){
		uint32_t fps = smpteFpsDecode(
            getByte(ticks, 1)
        );
		uint8_t subframeResolution = (uint8_t)ticks;
		uint32_t subframesPerSecond
            = fps * subframeResolution;
		return ratio100nsToSeconds / subframesPerSecond;
	}
	/* a leading 0 means ticks per beat */
	return
        microsecondsPerBeatToTimePerTick100ns(
	    	mm_defaultMicrosecondsPerBeat,
	    	ticks
	    );
}

static void midiSequencerOutputMidiEvent(
    MidiSequencer *sequencerPtr
){
    mmMidiOutShortMsg(
        sequencerPtr->midiOutPtr,
        (sequencerPtr->currentPtr++)->event
    );
}

static void midiSequencerOutputSysexEvent(
    MidiSequencer *sequencerPtr
){
    ++(sequencerPtr->currentPtr);
    /* currentPtr now points to length block */

    uint32_t byteLength
        = sequencerPtr->currentPtr->deltaTime;
	uint32_t indexLength
        = sequencerPtr->currentPtr->event;
    ++(sequencerPtr->currentPtr);
    /* currentPtr now points to first data */

    mmMidiOutSysex(
        sequencerPtr->midiOutPtr,
        sequencerPtr->currentPtr,
        byteLength
    );

    (sequencerPtr->currentPtr) += indexLength;
    /* currentPtr now points to 1 past last data */
}

static void midiSequencerHandleMetaEvent(
    MidiSequencer *sequencerPtr
){
    /* status is the second byte (following 0xFF) */
	uint8_t metaEventStatus = getByte(
        sequencerPtr->currentPtr->event,
        1
    );
	++(sequencerPtr->currentPtr);

	/* current now points to the length block */
	uint32_t byteLength
        = sequencerPtr->currentPtr->deltaTime;
	uint32_t indexLength
        = sequencerPtr->currentPtr->event;
	++(sequencerPtr->currentPtr);

	/* current now points to the first data entry */

    /* handle tempo event */
	if(metaEventStatus == mm_metaTempo){
        /* grab the tempo data */
		sequencerPtr->microsecondsPerBeat
            = fromBigEndian32(
                sequencerPtr->currentPtr->deltaTime
            ) >> 8;
		sequencerPtr->timePerTick100ns
            = microsecondsPerBeatToTimePerTick100ns(
			    sequencerPtr->microsecondsPerBeat,
			    sequencerPtr->sequencePtr->ticks
		    );
	}

    else if(metaEventStatus == mm_metaMarker){
        /* if loop start, set loop ptr */
        if(byteLength == 9){
            /* copy text to buffer */
            char buffer[10] = {0};
            strncpy(
                buffer,
                (char*)(sequencerPtr->currentPtr),
                byteLength
            );
            /* check for string equivalence */
            if(strncmp(buffer, "loopStart", 9) == 0){
                /* found loopStart */
                sequencerPtr->currentPtr
                    += indexLength;
                sequencerPtr->loopPtr
                    = sequencerPtr->currentPtr;
                /* 
                 * since we set current to 1 past
                 * last data entry before setting
                 * loop ptr, return
                 */
                return;
            }
        }
        /* if loop end, bring back to start */
        else if(byteLength == 7){
            /* copy text to buffer */
            char buffer[8] = {0};
            strncpy(
                buffer,
                (char*)(sequencerPtr->currentPtr),
                byteLength
            );
            /* check for string equivalence */
            if(strncmp(buffer, "loopEnd", 7) == 0){
                /* found loopEnd */
                sequencerPtr->currentPtr
                    = sequencerPtr->loopPtr;
                /* reset for stray note off events */
                mmMidiOutReset(sequencerPtr->midiOutPtr);
                /* 
                 * since loop ptr points directly to
                 * next event, return
                 */
                return;
            }
        }
    }
	
	sequencerPtr->currentPtr += indexLength;
	/* index now points 1 past the last data entry */
}

static void midiSequencerStopPlaybackThread(
    MidiSequencer *sequencerPtr
){
    if(sequencerPtr->running){
        atomic_store(&(sequencerPtr->running), false);
        threadKill(sequencerPtr->playbackThread);
        threadJoin(sequencerPtr->playbackThread);
    }
}

/*
 * Resets the playback fields for the given
 * MidiSequencer
 */
static void midiSequencerResetPlaybackFields(
    MidiSequencer *sequencerPtr
){
    sequencerPtr->sequencePtr = NULL;
    sequencerPtr->currentPtr = NULL;
    sequencerPtr->loopPtr = NULL;
	sequencerPtr->microsecondsPerBeat
        = mm_defaultMicrosecondsPerBeat;
	sequencerPtr->timePerTick100ns = 0;
}

/* 
 * Sequences and outputs midi events for the given
 * MidiSequencer
 */
static void midiSequencerPlayback(
    MidiSequencer *sequencerPtr
){
    /* initialize playback */
    mmMidiOutReset(sequencerPtr->midiOutPtr);
    sequencerPtr->currentPtr = arrayListFrontPtr(
        _EventUnit,
        &(sequencerPtr->sequencePtr->eventTrack)
    );

    /* timing variables */
    sequencerPtr->microsecondsPerBeat
        = mm_defaultMicrosecondsPerBeat;
    sequencerPtr->timePerTick100ns
        = calculateInitialTimePerTick100ns(
            sequencerPtr->sequencePtr->ticks
        );
    ArrayList *eventTrackPtr
        = &(sequencerPtr->sequencePtr->eventTrack);
    /* points to 1 past the last event */
    _EventUnit *endPtr 
        = (sequencerPtr->currentPtr)
            + eventTrackPtr->size;
    _EventUnit eventUnit = {0};
    uint64_t sleepDuration100ns = 0;
    uint8_t status = 0;
    
    /* begin playback */
    TimePoint targetTime = getCurrentTime();
    while(sequencerPtr->currentPtr != endPtr){
        /* sleep for delta time */
        eventUnit = *(sequencerPtr->currentPtr);
        if(eventUnit.deltaTime != 0){
            sleepDuration100ns = eventUnit.deltaTime
                * sequencerPtr->timePerTick100ns;
            /* check to see if need to exit */
            if(!(sequencerPtr->running)){
                /* do not output reset in this case */
                midiSequencerResetPlaybackFields(
                    sequencerPtr
                );
                return;
            }

            targetTime = addTimeNano(
                targetTime,
                sleepDuration100ns * 100
            );
            sleepUntil(targetTime);

            /* also check running after sleep */
            if(!(sequencerPtr->running)){
                /* do not output reset in this case */
                midiSequencerResetPlaybackFields(
                    sequencerPtr
                );
                return;
            }
        }

        /* handle event */
        status = getByte(eventUnit.event, 0);
        /* midi event case */
        if((status & mm_statusMask)
            != mm_metaEventOrSysex
        ){
            midiSequencerOutputMidiEvent(sequencerPtr);
        }
        /* meta event or sysex cases */
        else{
            switch(status){
                case mm_metaEvent:
                    /* may change tempo or loop back */
                    midiSequencerHandleMetaEvent(
                        sequencerPtr
                    );
                    break;
                case mm_sysexStart:
                case mm_sysexEnd:
                    midiSequencerOutputSysexEvent(
                        sequencerPtr
                    );
                    break;
                default:
                    pgError("error bad MIDI status");
            }
        }
    }

    /* cleanup after playback */
    mmMidiOutReset(sequencerPtr->midiOutPtr);
    midiSequencerResetPlaybackFields(sequencerPtr);
    sequencerPtr->running = false;
}

/* 
 * A wrapper for midiSequencerPlayback() which can
 * be used by threadCreate()
 */
static DECLARE_RUNNABLE_FUNC(
    midiSeqeuncerPlaybackWrapper,
    arg
){
    initThread();
    midiSequencerPlayback((MidiSequencer*)arg);
    return 0;
}

/* Begins playing back the specified midi sequence */
void midiSequencerStart(
    MidiSequencer *sequencerPtr,
    MidiSequence *sequencePtr
){
    midiSequencerStopPlaybackThread(sequencerPtr);
    sequencerPtr->sequencePtr = sequencePtr;
    sequencerPtr->running = true;

    /* Create and run the playback thread */
    CreateReturn createReturn = threadCreate(
        midiSeqeuncerPlaybackWrapper,
        sequencerPtr
    );
    if(createReturn.success){
        sequencerPtr->playbackThread
            = createReturn.thread;
    }
    else{
        pgError("Failed to create playback thread");
    }
}

/* Stops playback */
void midiSequencerStop(MidiSequencer *sequencerPtr){
    midiSequencerStopPlaybackThread(sequencerPtr);
    mmMidiOutReset(sequencerPtr->midiOutPtr);
}

/* Frees the given MidiSequencer */
void midiSequencerFree(MidiSequencer *sequencerPtr){
    midiSequencerStop(sequencerPtr);
}