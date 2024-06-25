#ifndef UNKNOWN_PROGRAM_H
#define UNKNOWN_PROGRAM_H

#include "Constructure.h"

#include "Unknown_Literals.h"

/*
 * Holds a program i.e. a sequence of bytecode
 * instructions
 */
typedef struct UNProgram{
    /* an arraylist of uint8_t */
    ArrayList code;
    /* an arraylist of uint16_t */
    ArrayList lineNumbers;
    /* collection of literal values */
    UNLiterals literals;
} UNProgram;

/* Constructs and returns a new UNProgram by value */
UNProgram unProgramMake();

/*
 * Returns a pointer to the first instruction in the
 * specified UNProgram; returns NULL if not possible
 */
uint8_t *unProgramGetEntryPoint(UNProgram *programPtr);

/*
 * Pushes the specified byte to the back of the
 * specified program's code
 */
void unProgramPushBackCode(
    UNProgram *programPtr,
    uint8_t byte,
    uint16_t lineNumber
);

/*
 * Pushes the specified todo UNValue to the back of the
 * specified program's literals and returns the index
 * where it was written
 */
size_t unProgramPushBackLiteral(
    UNProgram *programPtr,
    UNValue literal
);

/*
 * Prints out the disassembly of the specified program 
 */
void unProgramDisassemble(UNProgram *programPtr);

/*
 * Prints out the disassembly of a single instruction
 * in the specified program and returns the new offset
 */
size_t unProgramDisassembleInstruction(
    UNProgram *programPtr,
    size_t offset
);

/*
 * Frees the memory associated with the specified
 * program
 */
void unProgramFree(UNProgram *programPtr);


#endif