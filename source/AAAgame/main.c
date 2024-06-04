#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <time.h>

#include "ZMath.h"
#include "Constructure.h"
#include "MokyoMidi.h"

#include <unistd.h>

void printInt(int *toPrint){
    printf("%d\n", *toPrint);
}

void printString(const String *toPrint){
    printf("%s\n", toPrint->_ptr);
}

void printWideString(const WideString *toPrint){
    printf("%ls\n", toPrint->_ptr);
}

size_t intHash(const void *intPtr){
    return *((int*)intPtr);
}

bool intEquals(
    const void *intPtr1, 
    const void *intPtr2
){
    return *((int*)intPtr1) == *((int*)intPtr2);
}

#define intToLong(i) ((long)i)

int main(){
    MidiHub midiHub = midiHubMake(false);
    MidiSequence testSequence
        = parseMidiFile("test.mid");
    
    midiHubStart(&midiHub, &testSequence);

    sleep(1000);

    midiHubFree(&midiHub);
    midiSequenceFree(&testSequence);

    printf("main completed\n");
    return 0;
}