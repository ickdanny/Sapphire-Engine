#include "MokyoMidi_MidiOut.h"

#include "MokyoMidi_Constants.h"
#include "PGUtil.h"
#include "ZMath_Bitwise.h"

#ifdef WIN32

/*
 * Constructs a new MMMidiOut and returns it by value.
 */
MMMidiOut midiOutMake(){
    MMMidiOut toRet = {0};
    mmMidiOutStart(&toRet);
    return toRet;
}

/* Outputs a short message */
void mmMidiOutShortMsg(
    MMMidiOut *midiOutPtr, 
    uint32_t output
){
    MMRESULT result = midiOutShortMsg(
        midiOutPtr->midiOutHandle,
        output
    );
    if(result != MMSYSERR_NOERROR){
        pgError(
            "error outputting midi short msg; "
            SRC_LOCATION
        );
    }
}

/* Outputs a system exclusive message */
void mmMidiOutSysex(
    MMMidiOut *midiOutPtr,
    const void* bufferPtr,
    uint32_t byteLength
){
    /* prepare data to be output from MIDIHDR */
	MIDIHDR midiHDR = {0};
	midiHDR.lpData = (char*)bufferPtr;
	midiHDR.dwBufferLength = byteLength;
	midiHDR.dwBytesRecorded = byteLength;

    /* output the sysex */
    MMRESULT result = midiOutPrepareHeader(
        midiOutPtr->midiOutHandle,
        &midiHDR,
        sizeof(midiHDR)
    );
	if(result != MMSYSERR_NOERROR){
        pgError(
            "Error preparing sysEx; " SRC_LOCATION
        );
	}
	result = midiOutLongMsg(
        midiOutPtr->midiOutHandle,
        &midiHDR,
        sizeof(midiHDR)
    );
	if(result != MMSYSERR_NOERROR){
		pgError(
            "Error outputting sysEx; " SRC_LOCATION
        );
	}
	result = midiOutUnprepareHeader(
        midiOutPtr->midiOutHandle,
        &midiHDR,
        sizeof(midiHDR)
    );
	if(result != MMSYSERR_NOERROR){
		pgError(
            "Error unpreparing sysEx; " SRC_LOCATION
        );
	}
}

/* Starts a MidiOut's output */
void mmMidiOutStart(MMMidiOut *midiOutPtr){
    if(midiOutPtr->midiOutHandle != 0){
        mmMidiOutStop(midiOutPtr);
    }
    MMRESULT result = midiOutOpen(
        &(midiOutPtr->midiOutHandle),
        MIDI_MAPPER,
        0,
        0,
        CALLBACK_NULL
    );
    if(result != MMSYSERR_NOERROR){
        pgError(
            "error opening midi out; " SRC_LOCATION
        );
    }
}

/* Stops a MidiOut's output */
void mmMidiOutStop(MMMidiOut *midiOutPtr){
    if(midiOutPtr->midiOutHandle == 0){
        return;
    }
    MMRESULT result = midiOutClose(
        midiOutPtr->midiOutHandle
    );
    if(result != MMSYSERR_NOERROR){
        pgError(
            "error closing midi out; " SRC_LOCATION
        );
    }
    midiOutPtr->midiOutHandle = 0;
}

/* Frees the given MMMidiOut */
void mmMidiOutFree(MMMidiOut *midiOutPtr){
    if(!midiOutPtr){
        return;
    }
    mmMidiOutStop(midiOutPtr);
}

#endif /* end WIN32 */

#ifdef __APPLE__

/*
 * Throws an error and prints out the specified
 * OSStatus as an integer if it is nonzero
 */
void assertOSStatusZero(
    OSStatus status,
    const char *errMsg
){
    #define bufferSize 50

    if(status != 0){
        static char buffer[bufferSize] = {0};
        snprintf(
            buffer,
            bufferSize,
            "OSStatus: %d",
            status
        );
        pgWarning(buffer);
        pgError(errMsg);
    }
    #undef bufferSize
}

/*
 * Constructs a new MMMidiOut and returns it by value.
 */
MMMidiOut midiOutMake(){
    MMMidiOut toRet = {0};
    AUNode synthNode = 0;
    AUNode outNode = 0;
    AudioComponentDescription compDesc = {0};
	compDesc.componentManufacturer
        = kAudioUnitManufacturer_Apple;
    OSStatus retCode = 0;

    /* make new audio graph */
	retCode = NewAUGraph(&(toRet.graph));
    assertOSStatusZero(
        retCode,
        "failed to create AUGraph"
    );
    
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
    assertOSStatusZero(
        retCode,
        "failed to add synth node"
    );

    /* add the output to the graph */
	compDesc.componentType = kAudioUnitType_Output;
	compDesc.componentSubType
        = kAudioUnitSubType_DefaultOutput;  
	retCode = AUGraphAddNode(
        toRet.graph,
        &compDesc,
        &outNode
    );
    assertOSStatusZero(
        retCode,
        "failed to add out node"
    );
	
    /* open the graph */
	retCode = AUGraphOpen(toRet.graph);
    assertOSStatusZero(
        retCode,
        "failed to open AUGraph"
    );
	
    /* connect the synth to the output */
	retCode = AUGraphConnectNodeInput(
        toRet.graph,
        synthNode,
        0,
        outNode,
        0
    );
    assertOSStatusZero(
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
    assertOSStatusZero(
        retCode,
        "failed to get synthUnit"
    );

    /* initialize the graph */
    retCode = AUGraphInitialize(toRet.graph);
    assertOSStatusZero(
        retCode,
        "failed to initialize AUGraph"
    );

    /* start the graph */
    retCode = AUGraphStart(toRet.graph);
    assertOSStatusZero(
        retCode,
        "failed to start AUGraph"
    );

    return toRet;
}

/* Outputs a short message where the status is byte0 */
void mmMidiOutShortMsg(
    MMMidiOut *midiOutPtr,
    uint32_t output
){
    OSStatus retCode = MusicDeviceMIDIEvent(
        midiOutPtr->synthUnit,
        getByte(output, 0),
        getByte(output, 1),
        getByte(output, 2),
        0 /* sample offset */
    );
    assertOSStatusZero(
        retCode,
        "failed to output midi short msg"
    );
}

/* Outputs a system exclusive message */
void mmMidiOutSysex(
    MMMidiOut *midiOutPtr,
    const void* bufferPtr,
    uint32_t byteLength
){
    OSStatus retCode = MusicDeviceSysEx(
        midiOutPtr->synthUnit,
        bufferPtr,
        byteLength
    );
    assertOSStatusZero(
        retCode,
        "failed to output midi sysex msg"
    );
}

/* Starts a MidiOut's output */
void mmMidiOutStart(MMMidiOut *midiOutPtr){
    /* start the graph */
    AUGraphStart(midiOutPtr->graph);
}

/* Stops a MidiOut's output */
void mmMidiOutStop(MMMidiOut *midiOutPtr){
    /* stop the graph */
    AUGraphStop(midiOutPtr->graph);
}

/* Frees the given MMMidiOut */
void mmMidiOutFree(MMMidiOut *midiOutPtr){
    AUGraphStop(midiOutPtr->graph);
    DisposeAUGraph(midiOutPtr->graph);
    midiOutPtr->graph = NULL;
    midiOutPtr->synthUnit = NULL;
}

#endif /* end __APPLE__ */

/* Outputs a short message on all channels */
void mmMidiOutShortMsgOnAllChannels(
    MMMidiOut *midiOutPtr,
    uint32_t output
){
    /* 
     * make sure the last 4 bits are empty (it's where
     * we put the channel)
     */
	if(!(output << 28)) {
		for(int i = 0; i <= 0xF; ++i) {
			mmMidiOutShortMsg(midiOutPtr, output + i);
		}
	}
	else {
		pgError(
            "Error MIDI short msg must end in 0000"
        );
	}
}

/* Outputs a control change message on all channels */
void mmMidiOutControlChangeOnAllChannels(
    MMMidiOut *midiOutPtr,
    uint32_t data
){
    mmMidiOutShortMsgOnAllChannels(
        midiOutPtr,
		(data << 8) + mm_controlChange
	);
}

/* Resets the given MMMidiOut */
void mmMidiOutReset(MMMidiOut *midiOutPtr){
    /* 
     * Terminating a sysex message without sending an
	 * EOX(end - of - exclusive) byte might cause
     * problems for the receiving device
     */ 
	mmMidiOutShortMsg(midiOutPtr, mm_sysexEnd);

    mmMidiOutControlChangeOnAllChannels(
        midiOutPtr,
        mm_allNotesOff
    );
    mmMidiOutControlChangeOnAllChannels(
        midiOutPtr,
        mm_allSoundOff
    );
}