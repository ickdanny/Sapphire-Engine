#include "MokyoMidi_MidiHub.h"

/*
 * Constructs a new MidiHub and returns it by value.
 * Should not be called more than once.
 */
MidiHub midiHubMake(bool muted){
    MidiHub toRet = new MidiHub{0};
    //todo initialize midiOut
    //todo initialize midiSequencer by passing midiOut
    toRet.muted = muted;
    return toRet;
}

/* 
 * Starts playing the given midi sequence. Replaces
 * the previously playing sequence.
 */
void midiHubStart(
    MidiHub *midiHubPtr, 
    MidiSequence *midiSequencePtr
){
    if(!(midiHubPtr->muted)){
        //todo midiSequencer.start(midiSequencePointer)
    }
}

/* Stops playing the current midi sequence. */
void midiHubStop(MidiHub *midiHubPtr){
    //todo midiSequencer.stop();
}

/* 
 * If midi is currently muted, unmutes it. Otherwise,
 * mutes it.
 */
void midiHubToggleMute(MidiHub *midiHubPtr){
    //if we are now muted
	if(midiHubPtr->muted = !(midiHubPtr->muted)){
		midiHubStop(midiHubPtr);
	}
}

/*
 * Returns true if midi is currently muted, false
 * otherwise.
 */
bool midiHubIsMuted(const MidiHub *midiHubPtr){
    return midiHubPtr->muted;
}

/*
 * Closes the given MidiHub and frees all associated
 * resources. Using a MidiHub after it has been closed
 * is undefined behavior.
 */
void midiHubClose(MidiHub *midiHubPtr){
    //todo
}