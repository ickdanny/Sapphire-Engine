#ifndef UNKNOWN_VIRTUALMACHINE_H
#define UNKNOWN_VIRTUALMACHINE_H

#include "Unknown_Program.h"

/* size of the stack */
#define UN_STACK_SIZE 256

/*
 * used to report back the result of interpreting a
 * program
 */
typedef enum UNInterpretResult{
    un_ok,
    un_compileError,
    un_runtimeError
} UNInterpretResult;

/* the virtual machine which interprets UN programs */
typedef struct UNVirtualMachine{
    /* pointer to currently executing program */
    UNProgram *programPtr;
    /* pointer to next executing instruction */
    uint8_t *instructionPtr;
    /* stack used by VM to store all values */
    UNValue stack[UN_STACK_SIZE];
    /* pointer to one past the top of the stack */
    UNValue *stackPtr;
    /* pointer to the head of the object list */
    UNObject *objectListHeadPtr;
    /* hashmap of UNObjectString* to UNValue */
    HashMap stringMap;
    bool stringMapAllocated;

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