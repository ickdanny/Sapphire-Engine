#include "MokyoMidi_MidiSequence.h"

#include <stdio.h>

#include "ZMath.h"

/* Returns the size of the specified file in bytes */
static long getFileSize(FILE *filePtr){
    /* save old position indicator */
    long oldPos = ftell(filePtr);

    /* seek to end of file */
    fseek(filePtr, 0, SEEK_END);
    
    /* get value of position indicator */
    long fileSize = ftell(filePtr);

    /* seek back to original position indicator */
    fseek(filePtr, oldPos, SEEK_SET);

    return fileSize;
}

/*
 * Constructs an empty MidiSequence with the specified
 * initial event capacity and returns it by value.
 */
MidiSequence midiSequenceMake(
    size_t initEventCapacity
){
    MidiSequence toRet = {0};
    toRet.loopStartOffset = _noLoopStart;
    toRet.eventTrack = arrayListMake(
        _EventUnit, 
        initEventCapacity
    );
    return toRet;
}

/*
 * Reads the specified midi file and returns a
 * MidiSequence by value, ready for playback
 */
MidiSequence parseMidiFile(const char *filename){
    FILE *filePtr = fopen(filename, "rb");
    assertNotNull(filePtr, filename);

    long fileSize = getFileSize(filePtr);
    //TODO: get rid of magic number
    MidiSequence midiSequence = midiSequenceMake(2000);
}

/* Frees the given MidiSequence */
void midiSequenceFree(MidiSequence *midiSequencePtr){
    midiSequencePtr->ticks = 0;
    midiSequencePtr->loopStartOffset = _noLoopStart;
    arrayListFree(
        _EventUnit,
        &(midiSequencePtr->eventTrack)
    );
}