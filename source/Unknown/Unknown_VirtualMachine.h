#ifndef UNKNOWN_VIRTUALMACHINE_H
#define UNKNOWN_VIRTUALMACHINE_H

#include "Unknown_Program.h"
#include "Unknown_Object.h"
#include "Unknown_NativeFuncSet.h"
#include "Unknown_UserFuncSet.h"

/* size of the stack */
#define UN_STACK_SIZE 256

/* size of the call stack */
#define UN_CALLSTACK_SIZE 16

/*
 * used to report back the result of interpreting a
 * program
 */
typedef enum UNInterpretResult{
    un_success,
    un_yielded,
    un_runtimeError
} UNInterpretResult;

/* Stores information regarding a function call */
typedef struct UNCallFrame{
    UNObjectFunc *funcPtr;
    uint8_t *instructionPtr;
    UNValue *slots;
} UNCallFrame;

/* the virtual machine which interprets UN programs */
typedef struct UNVirtualMachine{
    /* stack used by VM to keep track of func calls */
    UNCallFrame callStack[UN_CALLSTACK_SIZE];
    /* stores height of call stack i.e. num calls */
    int frameCount;
    /* stack used by VM to store all values */
    UNValue stack[UN_STACK_SIZE];
    /* pointer to one past the top of the stack */
    UNValue *stackPtr;
    /* pointer to the head of the object list */
    UNObject *objectListHeadPtr;
    /*
     * hashmap of UNObjectString* to UNValue for string
     * interning
     */
    HashMap stringMap;
    bool stringMapAllocated;
    /*
     * hashmap of UNObjectString* to UNValue for global
     * variables
     */
    HashMap globalsMap;
    /* pointer to the set of native functions */
    UNNativeFuncSet *nativeFuncSetPtr;
    /* pointer to the set of user functions */
    UNUserFuncSet *userFuncSetPtr;
} UNVirtualMachine;

/*
 * Constructs and returns a new UNVirtualMachine by
 * value; the native function set pointer is nullable,
 * as is the user function set pointer
 */
UNVirtualMachine unVirtualMachineMake(
    UNNativeFuncSet *nativeFuncSetPtr,
    UNUserFuncSet *userFuncSetPtr
);

/*
 * Makes the specified virtual machine start
 * interpreting the specified program
 */
UNInterpretResult unVirtualMachineInterpret(
    UNVirtualMachine *vmPtr,
    UNObjectFunc *funcObjectProgramPtr
);

/*
 * Loads the specified program into the given virtual
 * machine but does not start running it; resume
 * should be called to run it
 */
void unVirtualMachineLoad(
    UNVirtualMachine *vmPtr,
    UNObjectFunc *funcObjectProgramPtr
);

/*
 * Has the specified virtual machine continue running
 * its program; should only be used if the virtual
 * machine has previously yielded
 */
UNInterpretResult unVirtualMachineResume(
    UNVirtualMachine *vmPtr
);

/*
 * Resets the state of the given UNVirtualMachine
 */
void unVirtualMachineReset(UNVirtualMachine *vmPtr);

/*
 * Frees the memory associated with the given
 * UNVirtualMachine
 */
void unVirtualMachineFree(UNVirtualMachine *vmPtr);

#endif