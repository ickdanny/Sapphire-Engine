#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "Constructure.h"

/* The set of actions which a dialogue file can take */
typedef enum DialogueCommand{
    dialogue_error,         /* crash if seen */
    dialogue_setLeftImage,  /* sets image on left */
    dialogue_setRightImage, /* sets image on right */
    dialogue_setText,       /* sets text */
    dialogue_setTrack,      /* starts new music */
    dialogue_stop,          /* waits for next input */
    dialogue_end,           /* ends dialogue */
} DialogueCommand;

/*
 * Instructs the dialogue to take the specified action
 */
typedef struct DialogueInstruction{
    /* the command this instruction will run */
    DialogueCommand command;
    /* optional: the data for the instruction */
    String data;
} DialogueInstruction;

/*
 * Frees the memory associated with the specified
 * dialogue instruction
 */
void dialogueInstructionFree(
    DialogueInstruction *instrPtr
);

/*
 * Holds data for a single dialogue i.e. a single
 * conversation
 */
typedef struct Dialogue{
    /* array list of DialogueInstruction */
    ArrayList instructionList;
} Dialogue;

/*
 * Reads the specified dialogue file and returns a
 * Dialogue by value, ready to be used
 */
Dialogue parseDialogueFile(const char *fileName);

/*
 * Frees the memory associated with the specified
 * dialogue
 */
void dialogueFree(Dialogue *dialoguePtr);

#endif