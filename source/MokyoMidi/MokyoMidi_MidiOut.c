#include "MokyoMidi_MidiOut.h"

#include "MokyoMidi_Constants.h"
#include "PGUtil.h"

/* Outputs a short message on all channels */
void midiOutShortMsgOnAllChannels(
    MidiOut *midiOutPtr,
    uint32_t output
){
    /* 
     * make sure the last 4 bits are empty (it's where
     * we put the channel)
     */
	if(!(output << 28)) {
		for(int i = 0; i <= 0xF; ++i) {
			midiOutShortMsg(midiOutPtr, output + i);
		}
	}
	else {
		pgError(
            "Error MIDI short msg must end in 0000"
        );
	}
}

/* Outputs a control change message on all channels */
void midiOutControlChangeOnAllChannels(
    MidiOut *midiOutPtr,
    uint32_t data
){
    midiOutShortMsgOnAllChannels(
        midiOutPtr,
		mm_controlChange + (data << 16)
	);
}

/* Resets the given MidiOut */
void midiOutReset(MidiOut *midiOutPtr){
    //todo
}