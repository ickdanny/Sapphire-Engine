#include "MokyoMidi_MidiSequence.h"

#include <stdio.h>

#include "ZMath.h"
#include "MokyoMidi_Constants.h"

/*
 * Constructs an empty MidiSequence and returns it by
 * value
 */
static MidiSequence midiSequenceMake(){
    MidiSequence toRet = {0};
    toRet.loopStartOffset = _noLoopStart;
    return toRet;
}

/* Stores the information from the midi file header */
typedef struct MidiFileHeader{
    /* identifier "MThd" */
	uint32_t id;
    /* always 6 (midi files are big endian) */
	uint32_t size;
    /* format id (midi files are big endian) */
	uint16_t format;
    /* number of tracks (midi files are big endian) */
	uint16_t tracks;
    /* 
     * number of ticks per quarter note (midi files
     * are big endian) 
     */
	uint16_t ticks;
} MidiFileHeader;

/* Stores the information from a midi track header */
typedef struct MidiTrackHeader{
    /* identifier "MTrk" */
    uint32_t id;
    /* track length (midi files are big endian) */
	uint32_t length;
} MidiTrackHeader;

/* Stores data used during the loading process */
typedef struct _LoadingData{
    FILE *filePtr;
    ArrayList *trackPtr;
    uint32_t index;
    bool encounteredEndOfTrack;
    uint8_t lastStatus;
    uint8_t status;
    uint8_t maskedStatus;
} _LoadingData;

/*
 * Initializes a _LoadingData struct and returns it
 * by value
 */
static _LoadingData loadingDataMake(
    FILE *filePtr,
    ArrayList *trackPtr
){
    _LoadingData toRet = {0};
    toRet.filePtr = filePtr;
    toRet.trackPtr = trackPtr;
    toRet.encounteredEndOfTrack = false;

    return toRet;
}

/* 
 * Reads the file header from a midi file; the file
 * position must be set to the start of the file 
 */
static MidiFileHeader readFileHeader(FILE *filePtr){
    MidiFileHeader fileHeader = {0};

    /* read in the id */
    fread(
        &(fileHeader.id),
        sizeof(fileHeader.id),
        1,
        filePtr
    );
    fileHeader.id = fromBigEndian32(fileHeader.id);

    /* read in the size */
    fread(
        &(fileHeader.size),
        sizeof(fileHeader.size),
        1,
        filePtr
    );
    fileHeader.size = fromBigEndian32(fileHeader.size);

    /* read in the format */
    fread(
        &(fileHeader.format),
        sizeof(fileHeader.format),
        1,
        filePtr
    );
    fileHeader.format = fromBigEndian16(
        fileHeader.format
    );

    /* read in the track count */
    fread(
        &(fileHeader.tracks),
        sizeof(fileHeader.tracks),
        1,
        filePtr
    );
    fileHeader.tracks = fromBigEndian16(
        fileHeader.tracks
    );

    /* read in the ticks per quarter note */
    fread(
        &(fileHeader.ticks),
        sizeof(fileHeader.ticks),
        1,
        filePtr
    );
    fileHeader.ticks = fromBigEndian16(
        fileHeader.ticks
    );

    /* check file header validity */
	if(fileHeader.id != mm_requiredHeaderID) {
		pgError(
            "Error MIDI file invalid header identifier"
        );
	}
	if(fileHeader.size < mm_minimumHeaderSize) {
		pgError("Error MIDI file header too small");
	}
	
	/* 
     * handle the case when the file header is longer
     * than expected
     */
	if(fileHeader.size > mm_minimumHeaderSize){
        /* skip to end of header in this case */
        fseek(
            filePtr,
            fileHeader.size - mm_minimumHeaderSize,
            SEEK_CUR
        );
	}
	
	return fileHeader;
}

/*
 * Reads a track header from a midi file; the file
 * position must be set to the start of the track
 */
static MidiTrackHeader readTrackHeader(FILE *filePtr){
	MidiTrackHeader trackHeader = {0};

    /* read in the id */
    fread(
        &(trackHeader.id),
        sizeof(trackHeader.id),
        1,
        filePtr
    );
    trackHeader.id = fromBigEndian32(trackHeader.id);

    /* read in the track length */
    fread(
        &(trackHeader.length),
        sizeof(trackHeader.length),
        1,
        filePtr
    );
    trackHeader.length = fromBigEndian32(
        trackHeader.length
    );
	
	/* check track header validity */
	if(trackHeader.id != mm_requiredTrackHeaderID) {
		pgError(
            "Error MIDI file invalid track identifier"
        );
	}
	
	return trackHeader;
}

/* Reads a byte from the given file and returns it */
static uint8_t readByteFromFile(FILE *filePtr){
    uint8_t toRet = 0;
    fread(&toRet, sizeof(toRet), 1, filePtr);
    return toRet;
}

/* Reads a variable length quantity from a midi file */
static uint32_t readVariableLength(FILE *filePtr){
	uint32_t toRet = 0;
	uint8_t byte = 0;
	
    /* read variable length loop */
	do {
        byte = readByteFromFile(filePtr);
		toRet = (toRet << 7)
            + (byte & ((uint8_t)0x7F));
	} while(byte & 0x80);
	
	/* don't swap endianness */
	
	return toRet;
}

/* Encodes a length as an EventUnit */
static _EventUnit encodeLength(uint32_t byteLength){
    _EventUnit eventUnit = {0};
	eventUnit.deltaTime = byteLength;
	uint32_t indexLength = ceilingIntegerDivide(
		byteLength,
		sizeof(_EventUnit)
	);
	eventUnit.event = indexLength;
	return eventUnit;
}

/* Loads a midi event with the provided loading data */
void loadMidiEvent(_LoadingData *loadingDataPtr){
	/* read in first byte */
	uint8_t byte = readByteFromFile(
        loadingDataPtr->filePtr
    );
    /* the EventUnit at index was added prior */
    arrayListGetPtr(_EventUnit,
        loadingDataPtr->trackPtr,
        loadingDataPtr->index
    )->event = ((uint32_t)(loadingDataPtr->status)) |
		(((uint32_t)(byte)) << 8);
	
	/* read second byte if has one */
	if(loadingDataPtr->maskedStatus != mm_programChange
        && loadingDataPtr->maskedStatus 
            != mm_channelPressure
    ){
		byte = readByteFromFile(
            loadingDataPtr->filePtr
        );
		arrayListGetPtr(_EventUnit,
            loadingDataPtr->trackPtr,
            loadingDataPtr->index
        )->event |= (((uint32_t)(byte)) << 16);
	}
	
	/* no need to swap endianness */
	
    /* save the status as last status */
	loadingDataPtr->lastStatus
        = loadingDataPtr->status;

	/* advance index by 1 */
	++(loadingDataPtr->index);
}

/* Loads a meta event with the provided loading data */
void loadMetaEvent(_LoadingData *loadingDataPtr){
	/* read in the meta event */
	uint8_t metaEventStatus = readByteFromFile(
        loadingDataPtr->filePtr
    );
	
	/* read in length */
	uint32_t length = readVariableLength(
        loadingDataPtr->filePtr
    );
	
	/* 
     * do not insert end of track events into the
     * translated track
     */
	if(metaEventStatus == mm_metaEndOfTrack){
		/* remove delta time from the current event */
        arrayListGetPtr(_EventUnit,
            loadingDataPtr->trackPtr,
            loadingDataPtr->index
        )->deltaTime = 0;

		loadingDataPtr->encounteredEndOfTrack = true;
		/* don't advance index */
	}
		
	/* insert everything else */
	else {
		/* 
         * first block =
         * deltaTime / 00 - 00 - event - FF 
         */
        arrayListGetPtr(_EventUnit,
            loadingDataPtr->trackPtr,
            loadingDataPtr->index
        )->event = (metaEventStatus << 8)
            | loadingDataPtr->status;

        /* advance index */
        ++(loadingDataPtr->index);
		
		/* second block = length / index length */
        _EventUnit lengthEncoding
            = encodeLength(length);
        arrayListSet(_EventUnit,
            loadingDataPtr->trackPtr,
            loadingDataPtr->index,
            lengthEncoding
        );
		uint32_t indexLength = lengthEncoding.event;

        /* advance index */
        ++(loadingDataPtr->index);
		
		if(length > 0){
            uint32_t writeStartIndex
                = loadingDataPtr->index;

            /* advance index by necessary amount */
			loadingDataPtr->index += indexLength;
            /* keep pushing to make room */
            for(uint32_t i = 0; i < indexLength; ++i){
                static _EventUnit dummy = {0};
                arrayListPushBack(_EventUnit,
                    loadingDataPtr->trackPtr,
                    dummy
                );
            }

            char *writeStartPtr 
                = (char*)arrayListGetPtr(_EventUnit,
                    loadingDataPtr->trackPtr,
                    writeStartIndex
                );

            /* read data into the translated track */
            fread(
                writeStartPtr,
                1,
                length,
                loadingDataPtr->filePtr
            );
		}
	}

    /* clear the last status */
	loadingDataPtr->lastStatus = 0;
}

/*
 * Loads a system exclusive event with the provided
 * loading data and optionally inserts a system
 * exclusive start
 */
void loadSysexEvent(
    _LoadingData *loadingDataPtr,
	bool insertSystemExclusiveStart
){
	/* read in length */
	uint32_t length = readVariableLength(
        loadingDataPtr->filePtr
    );
	
	/* first block = deltaTime / 00 - 00 - 00 - F0 */
	arrayListGetPtr(_EventUnit,
        loadingDataPtr->trackPtr,
        loadingDataPtr->index
    )->event = loadingDataPtr->status;

    /* advance index */
    ++(loadingDataPtr->index);
	
	/* second block = length / index length */
    _EventUnit lengthEncoding = {0};
	if(insertSystemExclusiveStart) {
		/*
         * add 1 to length for tge inserted F0 byte if
         * length > 0
         */ 
        lengthEncoding = encodeLength(
            length > 0 ? length + 1 : 0
        );
	}
	else{
        lengthEncoding = encodeLength(length);
	}
    arrayListSet(_EventUnit,
        loadingDataPtr->trackPtr,
        loadingDataPtr->index,
        lengthEncoding
    );
    uint32_t indexLength = lengthEncoding.event;

    /* advance index */
    ++(loadingDataPtr->index);
	
    /* insert data */
	if(length > 0){
        uint32_t writeStartIndex
            = loadingDataPtr->index;

        /* advance index by necessary amount */
		loadingDataPtr->index += indexLength;
        /* keep pushing to make room */
        for(uint32_t i = 0; i < indexLength; ++i){
            static _EventUnit dummy = {0};
            arrayListPushBack(_EventUnit,
                loadingDataPtr->trackPtr,
                dummy
            );
        }

        char *writeStartPtr
            = (char*)arrayListGetPtr(_EventUnit,
                loadingDataPtr->trackPtr,
                writeStartIndex
            );
        
		if(insertSystemExclusiveStart){
            /* insert F0 at beginning of data dump */
            *writeStartPtr = mm_sysexStart;
            ++writeStartPtr;
		}

        /* read data into the translated track */
        fread(
            writeStartPtr,
            1,
            length,
            loadingDataPtr->filePtr
        );
	}

	/* clear the last status */
	loadingDataPtr->lastStatus = 0;
}

/*
 * A heuristic for calculating the initial capacity
 * of the track arraylist
 */
#define initTrackCapacityHeuristic(trackLength) \
    ((trackLength) / 2)

/* 
 * Loads a single track from the specified midi file;
 * file position must point to the start of the track
 * header
 */
static ArrayList loadTrack(FILE *filePtr) {
	/* read in track header */
	MidiTrackHeader trackHeader = readTrackHeader(
        filePtr
    );
	
	/* init the track */
    ArrayList track = arrayListMake(_EventUnit,
        initTrackCapacityHeuristic(trackHeader.length)
    );

    _LoadingData loadingData = loadingDataMake(
        filePtr,
        &track
    );
	
	/* load the track */
	while(!loadingData.encounteredEndOfTrack){
        /* 
         * read in delta time and create new event
         * unit
         */
        _EventUnit eventUnit = {0};
        eventUnit.deltaTime 
            = readVariableLength(filePtr);
        arrayListPushBack(_EventUnit,
            &track,
            eventUnit
        );
		
		/* grab command byte */
		loadingData.status = readByteFromFile(filePtr);
		
		/* handle running status */
		if(loadingData.status < 0x80){
			loadingData.status
                = loadingData.lastStatus;
            fseek(filePtr, -1, SEEK_CUR);
		}
		
		/* handle midi events */
		loadingData.maskedStatus
            = loadingData.status & mm_statusMask;
		if(loadingData.maskedStatus 
            != mm_metaEventOrSysex
        ){
			loadMidiEvent(&loadingData);
		}
		/* handle meta events */
		else if(loadingData.status == mm_metaEvent){
			loadMetaEvent(&loadingData);
		}
		/* handle system exclusive events */
		else if(loadingData.status == mm_sysexStart){
			loadSysexEvent(
                &loadingData,
                true /* insert F0 at start */
            );
		}
		/* 
         * irrelevant whether continuation packet or
         * escape sequence
         */
		else if(loadingData.status == mm_sysexEnd){
			loadSysexEvent(
				&loadingData,
				false /* do not insert at start */
			);
		}
	}

    /* no shrink to fit; optimize later if needed */
	return track;
}

/* 
 * Compiles an arraylist of tracks into a single
 * track and returns it by value
 */
static ArrayList compileTracks(
	ArrayList *individualTracksPtr
){
    size_t numTracks = individualTracksPtr->size;

	/* case where we only have 1 track */
	if(numTracks == 1){
        /* make a shallow copy of the track */
        ArrayList toRet = arrayListGet(ArrayList,
            individualTracksPtr,
            0
        );
        /* erase the old track */
		arrayListClear(ArrayList, individualTracksPtr);

        return toRet;
	}
	
	/* 
     * create track to hold all elements in
     * individual tracks
     */
	size_t totalSize = 0;
    for(size_t i = 0; i < numTracks; ++i){
        totalSize += arrayListGet(ArrayList,
            individualTracksPtr,
            i
        ).size;
    }
	ArrayList compiledTrack
        = arrayListMake(_EventUnit, totalSize);
	
	/* delta time corresponding to each track */
    ArrayList deltaTimes
        = arrayListMake(uint32_t, numTracks);

    /* position in each individual track */
    ArrayList indices
        = arrayListMake(size_t, numTracks);

    /* init deltaTimes and indices */
    for(size_t i = 0; i < numTracks; ++i){
        /* push delta time */
        arrayListPushBack(uint32_t,
            &deltaTimes,
            /* get front event unit */
            arrayListFront(_EventUnit,
                /* from ith track */
                arrayListGetPtr(ArrayList,
                    individualTracksPtr,
                    i
                )
            ).deltaTime
        );
        /* push index */
        arrayListPushBack(size_t, &indices, 0);
    }
	
	/* find and insert events by chronological depth */
	while(individualTracksPtr->size > 1) {
		/* find next event to insert */
		uint32_t lowestDeltaTime = ~((uint32_t)0);
		size_t lowestDeltaTimeIndex = 0;
		
		for(size_t i = 0;
            i < individualTracksPtr->size;
            ++i
        ){
            uint32_t deltaTime = arrayListGet(uint32_t,
                &deltaTimes,
                i
            );
			if(deltaTime < lowestDeltaTime){
				lowestDeltaTime = deltaTime;
				lowestDeltaTimeIndex = i;
			}
		}
		
		/* handle deltaTimes and realTime */
        #define decrementDeltaTime(DTPTR) \
            (*DTPTR) -= lowestDeltaTime
        arrayListApply(uint32_t,
            &deltaTimes,
            decrementDeltaTime
        );
        #undef decrementDeltaTime
		
        size_t *trackIndexPtr = arrayListGetPtr(size_t,
            &indices,
            lowestDeltaTimeIndex
        );
        ArrayList *trackPtr
            = arrayListGetPtr(ArrayList,
                individualTracksPtr,
                lowestDeltaTimeIndex
            );
		
		/* truncate to just the status byte */
        uint8_t status
            = (uint8_t)(arrayListGet(_EventUnit,
                trackPtr,
                *trackIndexPtr
            ).event);
		
		/* handle normal midi message case */
		if((status & mm_statusMask)
            != mm_metaEventOrSysex
        ){
			/*
             * copy into our compiledTrack and advance
             * indices
             */
            _EventUnit eventUnit = {0};
            eventUnit.deltaTime = lowestDeltaTime;
            eventUnit.event = arrayListGet(_EventUnit,
                trackPtr,
                *trackIndexPtr
            ).event;
            ++(*trackIndexPtr);
            arrayListPushBack(_EventUnit,
                &compiledTrack,
                eventUnit
            );
		}
		/* 
         * handle meta and sysex 
         * (message with length)
         */
		else {
			/* 
             * copy status block into our compiled
             * track and advance indices
             */ 
            _EventUnit eventUnit = {0};
            eventUnit.deltaTime = lowestDeltaTime;
            eventUnit.event = arrayListGet(_EventUnit,
                trackPtr,
                *trackIndexPtr
            ).event;
            ++(*trackIndexPtr);
            arrayListPushBack(_EventUnit,
                &compiledTrack,
                eventUnit
            );

			/* 
             * trackIndex is now pointing to the
             * length block; copy the length block,
             * store index length, and advance indices
             */
            _EventUnit lengthEncoding
                = arrayListGet(_EventUnit,
                    trackPtr,
                    *trackIndexPtr
                );
            ++(*trackIndexPtr);
            arrayListPushBack(_EventUnit,
                &compiledTrack,
                lengthEncoding
            );

            uint32_t indexLength
                = lengthEncoding.event;

			/* 
             * track index is now pointing to the first
             * block after the length; copy as many
             * blocks as required by indexLength
             */ 
			for(uint32_t i = 0; i < indexLength; ++i){
                /* copy unit by unit */
                arrayListPushBack(_EventUnit,
                    &compiledTrack,
                    arrayListGet(_EventUnit,
                        trackPtr,
                        *trackIndexPtr
                    )
                );
                ++(*trackIndexPtr);
			}
		}
		
		/* either track is over -> remove it */
		if(*trackIndexPtr >= trackPtr->size){
            arrayListErase(uint32_t,
                &deltaTimes,
                lowestDeltaTimeIndex
            );
            arrayListErase(size_t,
                &indices,
                lowestDeltaTimeIndex
            );
            /* free the finished track */
            arrayListFree(_EventUnit,
                arrayListGetPtr(ArrayList,
                    individualTracksPtr,
                    lowestDeltaTimeIndex
                )
            );
            arrayListErase(ArrayList,
                individualTracksPtr,
                lowestDeltaTimeIndex
            );
		}
		/* 
         * or update new delta time for next event on
         * the track
         */
		else {
            arrayListSet(uint32_t,
                &deltaTimes,
                lowestDeltaTimeIndex,
                arrayListGet(_EventUnit,
                    trackPtr,
                    *trackIndexPtr
                ).deltaTime
            );
		}
	}
	/* 1 track left, append all into compiled track */
    ArrayList *trackPtr = arrayListFrontPtr(ArrayList,
        individualTracksPtr
    );
    size_t *trackIndexPtr = arrayListFrontPtr(size_t,
        &indices
    );
    while(*trackIndexPtr < trackPtr->size){
        /* copy unit by unit */
        arrayListPushBack(_EventUnit,
            &compiledTrack,
            arrayListGet(_EventUnit,
                trackPtr,
                *trackIndexPtr
            )
        );
        ++(*trackIndexPtr);
    }

    /* free last track */
    arrayListFree(_EventUnit,
        arrayListFrontPtr(ArrayList,
            individualTracksPtr
        )
    );

    /* free temporary arraylists */
    arrayListFree(uint32_t, &deltaTimes);
    arrayListFree(size_t, &indices);
	
	return compiledTrack;
}

/*
 * Reads the specified midi file and returns a
 * MidiSequence by value, ready for playback
 */
MidiSequence parseMidiFile(const char *filename){
    /* open the midi file as binary read */
    FILE *filePtr = fopen(filename, "rb");
    assertNotNull(filePtr, filename);

    /* initialize the midi sequence */
    MidiSequence midiSequence = midiSequenceMake();

    /* read in header */
    MidiFileHeader fileHeader = readFileHeader(
        filePtr
    );
	midiSequence.ticks = fileHeader.ticks;
	
    /* if single track format, just load */
	if(fileHeader.format == mm_formatSingleTrack){
		midiSequence.eventTrack = loadTrack(filePtr);
	}
    /* 
     * if multi track format, need to load each 
     * individual track and then compile them together 
     */
	else if(fileHeader.format
        == mm_formatMultiTrackSync
    ){
        ArrayList individualTrackList
            = arrayListMake(ArrayList,
                fileHeader.tracks
            );
		
        /* load each track */
		for(uint16_t i = 0; 
            i < fileHeader.tracks; 
            ++i
        ){
            arrayListSet(ArrayList, 
                &individualTrackList, 
                i, 
                loadTrack(filePtr)
            );
		}
		
        /* compile tracks together */
		midiSequence.eventTrack = compileTracks(
            &individualTrackList
        );
        /* 
         * compileTracks() will free individual tracks
         */

        /* free the individual track list */
        arrayListFree(ArrayList, &individualTrackList);
	}
	else {
		pgError("Error unsupported MIDI format");
	}
	
	return midiSequence;
}

/* Frees the given MidiSequence */
void midiSequenceFree(MidiSequence *midiSequencePtr){
    midiSequencePtr->ticks = 0;
    midiSequencePtr->loopStartOffset = _noLoopStart;
    arrayListFree(_EventUnit,
        &(midiSequencePtr->eventTrack)
    );
}