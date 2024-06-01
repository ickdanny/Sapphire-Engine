#include "MokyoMidi_MidiSequencer.h"

#include "ZMath.h"

#define ratio100nsToSeconds ((uint64_t)10000000)

/* 
 * Constructs and returns a new MidiSequencer with
 * the specified MidiOut by value
 */
MidiSequencer midiSequencerMake(MidiOut *midiOutPtr){
    MidiSequencer toRet = {0};
    toRet.midiOutPtr = midiOutPtr;
    atomic_init(&(toRet.running), false);

    return toRet;
}

static
uint32_t convertMicrosecondsPerBeatToTimePerTick100ns(
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
            getByte(ticks, 2)
        );
		uint8_t subframeResolution = (uint8_t)ticks;
		uint32_t subframesPerSecond
            = fps * subframeResolution;
		return ratio100nsToSeconds / subframesPerSecond;
	}
	/* a leading 0 means ticks per beat */
	return
        convertMicrosecondsPerBeatToTimePerTick100ns(
	    	defaultMicrosecondsPerBeat,
	    	ticks
	    );
}

static timePointType getCurrentTime(){
    //todo: windows: clockType::now() in c++
}

static void midiSequencerOutputMidiEvent(
    MidiSequencer *sequencerPtr
){
    midiOutShortMsg(
        sequencerPtr->midiOutPtr,
        (sequencerPtr->currentPtr++)->event
    );
}

static void midiSequencerOutputSystemExclusiveEvent(
    MidiSequencer *sequencerPtr
){

}

static void midiSequencerHandleMetaEvent(
    MidiSequencer *sequencerPtr
){
    /* status is the second byte (following 0xFF) */
	uint8_t metaEventStatus = getByte(
        sequencerPtr->currentPtr->event,
        2
    );
	++(sequencerPtr->currentPtr);

	/* current now points to the length block */
	uint32_t byteLength
        = sequencerPtr->currentPtr->deltaTime;
	uint32_t indexLength
        = sequencerPtr->currentPtr->event;
	++(sequencerPtr->currentPtr);

	//current now points to the first data entry

    /* handle tempo event */
	if(metaEventStatus == mm_metaTempo){
        /* grab the tempo data */
		microsecondsPerBeat
            = fromBigEndian32(iter->deltaTime) >> 8;
		timePerTick100ns= convertMicrosecondsPerBeatToTimePerTick100ns(
			microsecondsPerBeat,
			midiSequencePointer->ticks
		);
	}

    else if(metaEventStatus == mm_metaMarker){
        //todo: test if loop start and end
        /* if loop start, set loop ptr */
        if(is loop start){
            sequencerPtr->loopPtr
                = sequencerPtr->currentPtr;
        }
        /* if loop end, bring back to start */
        else if(is loop end){
            sequencerPtr->currentPtr
                = sequencerPtr->loopPtr;
        }
    }
	
	iter += indexLength;
	//index now points to 1 past the last data entry
}

static void midiSequencerStopPlaybackThread(
    MidiSequencer *sequencerPtr
){
    atomic_store(&(sequencerPtr->running), false);

    //todo: wakeupSwitch.signal();
    threadJoin(sequencerPtr->playbackThread);
    //todo: wakeupSwitch.unsignal();
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
    midiOutReset(sequencerPtr->midiOutPtr);

    /* timing variables */
    sequencerPtr->microsecondsPerBeat
        = mm_defaultMicrosecondsPerBeat;
    sequencerPtr->timePerTick100ns
        = calculateInitialTimePerTick100ns(
            sequencerPtr->sequencePtr->ticks
        );

    /* cleanup after playback */
    midiOutReset(sequencerPtr->midiOutPtr);
    midiSequencerResetPlaybackFields(sequencerPtr);
    sequencerPtr->running = false;
}

/* 
 * A wrapper for midiSequencerPlayback() which can
 * be used by threadCreate()
 */
static void* midiSeqeuncerPlaybackWrapper(void * arg){
    midiSequencerPlayback((MidiSequencer*)arg);
    return NULL;
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
    stopPlaybackThread();
    midiOutReset(sequencerPtr->midiOutPtr);
}




//JOIFSAJFOIASJFOISAJOFIASJFOSAIJD

	

	
	void MidiSequencer::playback() {
		using ratioTimePointTo100ns = std::ratio<
			clockType::period::num * ratio100nsToSeconds,
			clockType::period::den
		>;
		
		midiOutPointer->outputReset();
		
		//timing variables
		microsecondsPerBeat = defaultMicrosecondsPerBeat;
		timePerTick100ns = calculateInitialTimePerTick100ns(
			midiSequencePointer->ticks
		);
		
		timePointType prevTimeStamp { getCurrentTime() };
		timePointType currentTimeStamp {};
		
		long long previousSleepDuration100ns { 0 };
		
		//loop variables
		iter = midiSequencePointer->compiledTrack.begin();
		auto endIter { midiSequencePointer->compiledTrack.end() };
		loopPointIter = endIter;
		
		//helper function for calculating sleep duration
		auto calculateSleepDuration100ns {
			[&]() {
				//calculate how long we should wait
				long long sleepDuration100ns {
					static_cast<long long>(iter->deltaTime) * timePerTick100ns
				};
				
				//account for the time we spent already
				currentTimeStamp = getCurrentTime();
				long long timeElapsed {
					((currentTimeStamp - prevTimeStamp).count()
						* ratioTimePointTo100ns::num)
						/ ratioTimePointTo100ns::den
				};
				long long timeLost { timeElapsed - previousSleepDuration100ns };
				sleepDuration100ns -= timeLost;
				
				prevTimeStamp = currentTimeStamp;
				return sleepDuration100ns;
			}
		};
		
		//begin playback
		while( iter != endIter ) {
			//sleep for delta time
			if( iter->deltaTime != 0 ) {
				long long sleepDuration100ns {
					calculateSleepDuration100ns()
				};
				previousSleepDuration100ns = sleepDuration100ns;
				
				//check to see if need to exit before sleep
				if( !running.load() ) {
					//do not output reset in this case
					resetPlaybackFields();
					return;
				}
				
				sleep100ns(sleepDuration100ns);
				
				//also check after sleep
				if( !running.load() ) {
					//do not output reset in this case
					resetPlaybackFields();
					return;
				}
			}
			
			//handle event
			uint8_t status { getByte(iter->event, 1) };
			//midi event case
			if( (status & statusMask) != metaEventOrSystemExclusive ) {
				outputMidiEvent();
			}
				//meta event or system exclusive cases
			else {
				switch( status ) {
					case metaEvent:
						//may change tempo or loop back
						handleMetaEvent();
						break;
						//continuation events and escape sequences start with sysEx end
						//the data is encoded the same way
					case systemExclusiveStart:
					case systemExclusiveEnd: outputSystemExclusiveEvent();
						break;
					default: throw std::runtime_error { "Error unrecognized MIDI status" };
				}
			}
		}
		
		//output reset if we finish naturally
		midiOutPointer->outputReset();
		resetPlaybackFields();
		running.store(false);
	}

	
	void MidiSequencer::outputSystemExclusiveEvent() {
		++iter;
		//index now points to the length block
		
		uint32_t byteLength { iter->deltaTime };
		uint32_t indexLength { iter->event };
		++iter;
		//index now points to the first data entry
		
		//prepare data to be output from MIDIHDR
		MIDIHDR midiHDR {};
		midiHDR.lpData = reinterpret_cast<char*>(&(*iter)); //probably UB
		midiHDR.dwBufferLength = byteLength;
		midiHDR.dwBytesRecorded = byteLength;
		
		iter += indexLength;
		
		midiOutPointer->outputSystemExclusive(&midiHDR);
		
		//index now points to 1 past the last data entry
	}