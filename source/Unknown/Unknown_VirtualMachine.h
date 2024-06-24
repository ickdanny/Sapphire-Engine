#ifndef UNKNOWN_VIRTUALMACHINE_H
#define UNKNOWN_VIRTUALMACHINE_H

#include "Unknown_Program.h"

/*
 * used to report back the result of interpreting a
 * program
 */
typedef enum UNInterpretResult{
    UN_OK,
    UN_COMPILE_ERROR,
    UN_RUNTIME_ERROR
} UNInterpretResult;

/* the virtual machine which interprets UN programs */
typedef struct UNVirtualMachine{
    /* pointer to currently executing program */
    UNProgram *programPtr;
    /* pointer to next executing instruction */
    uint8_t *instructionPtr;
} UNVirtualMachine;

/*
 * Constructs and returns a new UNVirtualMachine by
 * value
 */
UNVirtualMachine unVirtualMachineMake();

/*
 * Resets the state of the given UNVirtualMachine
 */
void unVirtualMachineReset(UNVirtualMachine *vmPtr);

/*
 * Makes the specified virtual machine start
 * interpreting the specified program
 */
UNInterpretResult unVirtualMachineInterpret(
    UNVirtualMachine *vmPtr,
    UNProgram *programPtr
);

/*
 * Frees the memory associated with the given
 * UNVirtualMachine
 */
void unVirtualMachineFree(UNVirtualMachine *vmPtr);

#endif