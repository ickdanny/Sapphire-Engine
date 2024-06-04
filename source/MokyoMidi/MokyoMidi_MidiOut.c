#include "MokyoMidi_MidiOut.h"

#include "MokyoMidi_Constants.h"
#include "PGUtil.h"
#include "ZMath_Bitwise.h"

#ifdef __WIN32__

/*
 * Constructs a new MidiOut and returns it by value.
 */
MidiOut midiOutMake(){
    //todo
}

/* Outputs a short message */
void midiOutShortMsg(
    MidiOut *midiOutPtr, 
    uint32_t output
){
    //todo
}

/* Outputs a system exclusive message */
void midiOutSysex(
    MidiOut *midiOutPtr,
    const void* bufferPtr,
    uint32_t byteLength
){
    //prepare data to be output from MIDIHDR
	MIDIHDR midiHDR = {0};
	midiHDR.lpData = (char*)bufferPtr;
	midiHDR.dwBufferLength = byteLength;
	midiHDR.dwBytesRecorded = byteLength;

    //todo
}

/* Starts a MidiOut's output */
void midiOutStart(MidiOut *midiOutPtr){
    //todo
}

/* Stops a MidiOut's output */
void midiOutStop(MidiOut *midiOutPtr){
    //todo
}

/* Frees the given MidiOut */
void midiOutFree(MidiOut *midiOutPtr){
    //todo
    midiOutPtr->graph = NULL;
    midiOutPtr->synthUnit = NULL;
}

#endif /* end __WIN32__ */

#ifdef __APPLE__

/*
 * Constructs a new MidiOut and returns it by value.
 */
MidiOut midiOutMake(){
    MidiOut toRet = {0};
    AUNode synthNode = 0;
    AUNode outNode = 0;
    AudioComponentDescription compDesc = {0};
	compDesc.componentManufacturer
        = kAudioUnitManufacturer_Apple;
    OSStatus retCode = 0;

    /* make new audio graph */
	retCode = NewAUGraph(&(toRet.graph));
    assertZero(retCode, "failed to create AUGraph");
    
    
    /* add the synth to the graph */
	compDesc.componentType
        = kAudioUnitType_MusicDevice;
	compDesc.componentSubType
        = kAudioUnitSubType_DLSSynth;
	retCode = AUGraphAddNode(
        toRet.graph,
        &compDesc,
        &synthNode
    );
    assertZero(retCode, "failed to add synth node");

    /* add the output to the graph */
	compDesc.componentType = kAudioUnitType_Output;
	compDesc.componentSubType
        = kAudioUnitSubType_DefaultOutput;  
	retCode = AUGraphAddNode(
        toRet.graph,
        &compDesc,
        &outNode
    );
    assertZero(retCode, "failed to add out node");
	
    /* open the graph */
	retCode = AUGraphOpen(toRet.graph);
    assertZero(retCode, "failed to open AUGraph");
	
    /* connect the synth to the output */
	retCode = AUGraphConnectNodeInput(
        toRet.graph,
        synthNode,
        0,
        outNode,
        0
    );
    assertZero(
        retCode,
        "failed to connect synth to output"
    );
	
    /* retrieve the synth unit from the graph */
	retCode = AUGraphNodeInfo(
        toRet.graph,
        synthNode,
        0,
        &(toRet.synthUnit)
    );
    assertZero(retCode, "failed to get synthUnit");

    /* initialize the graph */
    retCode = AUGraphInitialize(toRet.graph);
    assertZero(retCode, "failed to initialize AUGraph");

    /* start the graph */
    retCode = AUGraphStart(toRet.graph);
    assertZero(retCode, "failed to start AUGraph");

    return toRet;
}

/* Outputs a short message where the status is byte0 */
void midiOutShortMsg(
    MidiOut *midiOutPtr, 
    uint32_t output
){
    MusicDeviceMIDIEvent(
        midiOutPtr->synthUnit,
        getByte(output, 0),
        getByte(output, 1),
        getByte(output, 2),
        0 /* sample offset */
    );
}

/* Outputs a system exclusive message */
void midiOutSysex(
    MidiOut *midiOutPtr,
    const void* bufferPtr,
    uint32_t byteLength
){
    MusicDeviceSysEx(
        midiOutPtr->synthUnit,
        bufferPtr,
        byteLength
    );
}

/* Starts a MidiOut's output */
void midiOutStart(MidiOut *midiOutPtr){
    /* start the graph */
    AUGraphStart(midiOutPtr->graph);
}

/* Stops a MidiOut's output */
void midiOutStop(MidiOut *midiOutPtr){
    /* stop the graph */
    AUGraphStop(midiOutPtr->graph);
}

/* Frees the given MidiOut */
void midiOutFree(MidiOut *midiOutPtr){
    AUGraphStop(midiOutPtr->graph);
    DisposeAUGraph(midiOutPtr->graph);
    midiOutPtr->graph = NULL;
    midiOutPtr->synthUnit = NULL;
}

#endif /* end __APPLE__ */

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
		(data << 8) + mm_controlChange
	);
}

/* Resets the given MidiOut */
void midiOutReset(MidiOut *midiOutPtr){
    /* 
     * Terminating a sysex message without sending an
	 * EOX(end - of - exclusive) byte might cause
     * problems for the receiving device
     */ 
	midiOutShortMsg(midiOutPtr, mm_sysexEnd);

    midiOutControlChangeOnAllChannels(
        midiOutPtr,
        mm_allNotesOff
    );
    midiOutControlChangeOnAllChannels(
        midiOutPtr,
        mm_allSoundOff
    );
}