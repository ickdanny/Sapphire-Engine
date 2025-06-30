#ifndef NECRO_PROGRAM_H
#define NECRO_PROGRAM_H

#include "Constructure.h"

#include "Necro_Literals.h"

/*
 * Holds a program i.e. a sequence of bytecode
 * instructions
 */
typedef struct NecroProgram{
    /* an arraylist of uint8_t */
    ArrayList code;
    /* an arraylist of uint16_t */
    ArrayList lineNumbers;
    /* collection of literal values */
    NecroLiterals literals;
} NecroProgram;

/*
 * Constructs and returns a new NecroProgram by value;
 * the enclosing program pointer is nullable
 */
NecroProgram necroProgramMake(
    NecroProgram *enclosingPtr
);

/*
 * Returns a pointer to the first instruction in the
 * specified NecroProgram; returns NULL if not possible
 */
uint8_t *necroProgramGetEntryPoint(
    NecroProgram *programPtr
);

/*
 * Pushes the specified byte to the back of the
 * specified program's code
 */
void necroProgramPushBackCode(
    NecroProgram *programPtr,
    uint8_t byte,
    uint16_t lineNumber
);

/*
 * Pushes the specified NecroValue to the back of the
 * specified program's literals and returns the index
 * where it was written
 */
size_t necroProgramPushBackLiteral(
    NecroProgram *programPtr,
    NecroValue literal
);

/*
 * Prints out the disassembly of the specified program 
 */
void necroProgramDisassemble(NecroProgram *programPtr);

/*
 * Prints out the disassembly of a single instruction
 * in the specified program and returns the new offset
 */
size_t necroProgramDisassembleInstruction(
    NecroProgram *programPtr,
    size_t offset
);

/*
 * Frees the memory associated with the specified
 * program
 */
void necroProgramFree(NecroProgram *programPtr);

#endif