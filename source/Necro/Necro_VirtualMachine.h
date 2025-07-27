#ifndef NECRO_VIRTUALMACHINE_H
#define NECRO_VIRTUALMACHINE_H

#include "Necro_Program.h"
#include "Necro_Object.h"
#include "Necro_NativeFuncSet.h"

/* size of the stack */
#define NECRO_STACK_SIZE 256

/* size of the call stack */
#define NECRO_CALLSTACK_SIZE 16

/*
 * used to report back the result of interpreting a
 * program
 */
typedef enum NecroInterpretResult{
    necro_success,
    necro_yielded,
    necro_runtimeError
} NecroInterpretResult;

/* Stores information regarding a function call */
typedef struct NecroCallFrame{
    NecroObjectFunc *funcPtr;
    uint8_t *instructionPtr;
    NecroValue *slots;
    /*
     * points to the frame of the last call of the
     * enclosing procedure
     */
    struct NecroCallFrame *accessPtr;
} NecroCallFrame;

/*
 * the virtual machine which interprets necro programs
 */
typedef struct NecroVirtualMachine{
    /* stack used by VM to keep track of func calls */
    NecroCallFrame callStack[NECRO_CALLSTACK_SIZE];
    /* stores height of call stack i.e. num calls */
    int frameCount;
    /* stack used by VM to store all values */
    NecroValue stack[NECRO_STACK_SIZE];
    /* pointer to one past the top of the stack */
    NecroValue *stackPtr;
    /* pointer to the head of the object list */
    NecroObject *objectListHeadPtr;
    /*
     * hashmap of NecroObjectString* to NecroValue for
     * string interning
     */
    HashMap stringMap;
    bool stringMapAllocated;
    /*
     * hashmap of NecroObjectString* to NecroValue for
     * global variables
     */
    HashMap globalsMap;
    /* pointer to the set of native functions */
    NecroNativeFuncSet *nativeFuncSetPtr;
} NecroVirtualMachine;

/*
 * Constructs and returns a new NecroVirtualMachine by
 * value; the native function set pointer is nullable,
 * as is the user function set pointer
 */
NecroVirtualMachine necroVirtualMachineMake(
    NecroNativeFuncSet *nativeFuncSetPtr
);

/*
 * Makes the specified virtual machine start
 * interpreting the specified program
 */
NecroInterpretResult necroVirtualMachineInterpret(
    NecroVirtualMachine *vmPtr,
    NecroObjectFunc *funcObjectProgramPtr
);

/*
 * Loads the specified program into the given virtual
 * machine but does not start running it; resume
 * should be called to run it
 */
void necroVirtualMachineLoad(
    NecroVirtualMachine *vmPtr,
    NecroObjectFunc *funcObjectProgramPtr
);

/*
 * Has the specified virtual machine continue running
 * its program; should only be used if the virtual
 * machine has previously yielded
 */
NecroInterpretResult necroVirtualMachineResume(
    NecroVirtualMachine *vmPtr
);

/*
 * Resets the state of the given NecroVirtualMachine
 */
void necroVirtualMachineReset(
    NecroVirtualMachine *vmPtr
);

/*
 * Frees the memory associated with the given
 * NecroVirtualMachine
 */
void necroVirtualMachineFree(
    NecroVirtualMachine *vmPtr
);

#endif