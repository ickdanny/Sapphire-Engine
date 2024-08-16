#include "MokyoMidi_MidiOut.h"

#include "MokyoMidi_Constants.h"
#include "PGUtil.h"
#include "ZMath_Bitwise.h"

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
MMMidiOut mmMidiOutMake(){
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

#ifdef WIN32

/*
 * Constructs a new MMMidiOut and returns it by value.
 */
MMMidiOut mmMidiOutMake(){
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

#ifdef __linux__

#include <alloca.h>

#define defaultDestClient 128
#define defaultDestPort 0

/*
 * Retrieves the address of the port prefixed by the
 * given string; returns true if successful, false
 * otherwise
 */
static bool getPortAddressString(
    snd_seq_t *seqPtr,
    snd_seq_addr_t *addressOutPtr,
    const char *prefix
){
    int retVal = snd_seq_parse_address(
        seqPtr,
        addressOutPtr,
        prefix
    );
    return retVal == 0;
}

/*
 * Retrieves the address of the port specified by the
 * given client and port IDs; returns true if
 * successful, false otherwise
 */
static bool getPortAddressNumeric(
    snd_seq_t *seqPtr,
    snd_seq_addr_t *addressOutPtr,
    int client,
    int port
){
    #define bufferSize 50
    static char buffer[bufferSize] = {0};

    snprintf(
        buffer,
        bufferSize,
        "%d:%d",
        client,
        port
    );
    return getPortAddressString(
        seqPtr,
        addressOutPtr,
        buffer
    );

    #undef bufferSize
}

/*
 * Connects the specified sender port to the dest port;
 * returns true if successful, false otherwise
 */
static bool connectPorts(
    snd_seq_t *seqPtr,
    snd_seq_addr_t sender,
    snd_seq_addr_t dest
){
    snd_seq_port_subscribe_t *subInfoPtr;
    snd_seq_port_subscribe_alloca(&subInfoPtr);
    snd_seq_port_subscribe_set_sender(
        subInfoPtr,
        &sender
    );
    snd_seq_port_subscribe_set_dest(
        subInfoPtr,
        &dest
    );
    snd_seq_port_subscribe_set_queue(
        subInfoPtr,
        SND_SEQ_QUEUE_DIRECT
    );
    snd_seq_port_subscribe_set_exclusive(
        subInfoPtr,
        false
    );
    snd_seq_port_subscribe_set_time_update(
        subInfoPtr,
        false
    );
    snd_seq_port_subscribe_set_time_real(
        subInfoPtr,
        false
    );

    /* check for preexisting connection */
    int retVal = snd_seq_get_port_subscription(
        seqPtr,
        subInfoPtr
    );
    if(retVal == 0){
        /*
         * if successfully got preexisting sub info,
         * this indicates that a connection has
         * already been established, meaning this
         * function should fail
         */
        pgWarning(
            "ALSA connection to port 128:0 already "
            "established"
        );
        return false;
    }

    /* attempt to make connection */
    retVal = snd_seq_subscribe_port(
        seqPtr,
        subInfoPtr
    );
    if(retVal != 0){
        pgWarning(
            "failed to make ALSA connection to port "
            "128:0"
        );
        return false;
    }
    return true;
}

/*
 * Disconnects the specified sender port from the dest
 * port; returns true if successful, false otherwise
 */
static bool disconnectPorts(
    snd_seq_t *seqPtr,
    snd_seq_addr_t sender,
    snd_seq_addr_t dest
){
    snd_seq_port_subscribe_t *subInfoPtr;
    snd_seq_port_subscribe_alloca(&subInfoPtr);
    snd_seq_port_subscribe_set_sender(
        subInfoPtr,
        &sender
    );
    snd_seq_port_subscribe_set_dest(
        subInfoPtr,
        &dest
    );
    snd_seq_port_subscribe_set_queue(
        subInfoPtr,
        SND_SEQ_QUEUE_DIRECT
    );
    snd_seq_port_subscribe_set_exclusive(
        subInfoPtr,
        false
    );
    snd_seq_port_subscribe_set_time_update(
        subInfoPtr,
        false
    );
    snd_seq_port_subscribe_set_time_real(
        subInfoPtr,
        false
    );

    /* check for preexisting connection */
    int retVal = snd_seq_get_port_subscription(
        seqPtr,
        subInfoPtr
    );
    if(retVal != 0){
        /*
         * if failed to get preexisting sub info,
         * this indicates that a connection has not
         * already been established, meaning this
         * function should fail
         */
        pgWarning(
            "ALSA connection to port 128:0 not "
            "established"
        );
        return false;
    }

    /* attempt to remove connection */
    retVal = snd_seq_unsubscribe_port(
        seqPtr,
        subInfoPtr
    );
    if(retVal != 0){
        pgWarning(
            "failed to remove ALSA connection to port "
            "128:0"
        );
        return false;
    }
    return true;
}

/*
 * Constructs a new MMMidiOut and returns it by value.
 */
MMMidiOut mmMidiOutMake(){
    MMMidiOut toRet = {0};
    toRet.valid = false;
    int retVal = 0;

    /* create alsa seq */
    retVal = snd_seq_open(
        &(toRet.seqPtr),
        "default",
        SND_SEQ_OPEN_OUTPUT,
        0
    );
    /* if failed to open, return invalid midiOut */
    if(retVal != 0){
        pgWarning("failed to open alsa seq");
        return toRet;
    }

    /* get client for seq */
    int client = snd_seq_client_id(toRet.seqPtr);
    /* if failed to get client, return invalid */
    if(client < 0){
        pgWarning("failed to get alsa seq client");
        snd_seq_close(toRet.seqPtr);
        return toRet;
    }

    /* create rawmidi port */
    retVal = snd_rawmidi_open(
        NULL, /* uninterested in input */
        &(toRet.rawmidiPtr),
        "virtual", /* creates alsa seq port */
        SND_RAWMIDI_SYNC /* immediate output */
    );
    /* if failed to create, return invalid */
    if(retVal != 0){
        pgWarning("failed to create rawmidi");
        snd_seq_close(toRet.seqPtr);
        return toRet;
    }

    /*
     * attempt to open rawmidi port; first assume that
     * ALSA created the client one above the sequencer
     */
    if(!getPortAddressNumeric(
        toRet.seqPtr,
        &(toRet.sender),
        client + 1,
        0
    )){
        /*
         * failed to get next client; attempt to grab
         * any port with the prefix "Virtual RawMIDI"
         */
        if(!getPortAddressString(
            toRet.seqPtr,
            &(toRet.sender),
            "Virtual RawMIDI"
        )){
            /* failed to find port; return invalid */
            pgWarning(
                "failed to find virtual rawmidi port"
            );
            snd_seq_close(toRet.seqPtr);
            snd_rawmidi_close(toRet.rawmidiPtr);
            return toRet;
        }
    }

    /*
     * if the sender is found, the midiout is valid
     * regardless of if it gets connected to any
     * destination port
     */
    toRet.valid = true;

    /* attempt to open destination at 128:0 */
    if(getPortAddressNumeric(
        toRet.seqPtr,
        &(toRet.dest),
        defaultDestClient,
        defaultDestPort
    )){
        /* if opened default port 128:0, connect */
        connectPorts(
            toRet.seqPtr,
            toRet.sender,
            toRet.dest
        );
    }

    return toRet;
}

/* Starts a MMMidiOut's output */
void mmMidiOutStart(MMMidiOut *midiOutPtr){
    if(!midiOutPtr->valid){
        return;
    }
    connectPorts(
        midiOutPtr->seqPtr,
        midiOutPtr->sender,
        midiOutPtr->dest
    );
}

/* Stops a MMMidiOut's output */
void mmMidiOutStop(MMMidiOut *midiOutPtr){
    if(!midiOutPtr->valid){
        return;
    }
    disconnectPorts(
        midiOutPtr->seqPtr,
        midiOutPtr->sender,
        midiOutPtr->dest
    );
}

/* Outputs a short message */
void mmMidiOutShortMsg(
    MMMidiOut *midiOutPtr, 
    uint32_t output
){
    uint8_t msg[3] = {
        getByte(output, 0),
        getByte(output, 1),
        getByte(output, 2)
    };

    /* most midi messages have 2 data bytes */
    int numBytesToWrite = 3;
    switch(msg[0] & mm_statusMask){
        /*
         * program change and channel pressure
         * messages only have 1 data byte
         */
        case mm_programChange:
        case mm_channelPressure:
            numBytesToWrite = 2;
            break;
        default:
            /* do nothing */
    }
    snd_rawmidi_write(
        midiOutPtr->rawmidiPtr,
        msg,
        numBytesToWrite
    );
}

/* Outputs a system exclusive message */
void mmMidiOutSysex(
    MMMidiOut *midiOutPtr,
    const void* bufferPtr,
    uint32_t byteLength
){
    snd_rawmidi_write(
        midiOutPtr->rawmidiPtr,
        bufferPtr,
        byteLength
    );
}

/* Frees the given MMMidiOut */
void mmMidiOutFree(MMMidiOut *midiOutPtr){
    if(midiOutPtr->valid){
        mmMidiOutStop(midiOutPtr);
        snd_seq_close(midiOutPtr->seqPtr);
        snd_rawmidi_close(midiOutPtr->rawmidiPtr);
    }
    memset(midiOutPtr, 0, sizeof(*midiOutPtr));
}

#endif /* end __linux__ */

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