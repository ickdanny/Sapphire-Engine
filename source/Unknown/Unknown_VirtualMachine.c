#include "Unknown_VirtualMachine.h"

#include "Unknown_Instructions.h"

/*
 * Constructs and returns a new UNVirtualMachine by
 * value
 */
UNVirtualMachine unVirtualMachineMake(){
    UNVirtualMachine toRet = {0};
    toRet.instructionPtr = 0;
    return toRet;
    //todo: no state yet in vm
}

/*
 * Reads the next byte in the specified virtual
 * machine and advances the instruction pointer
 */
#define readByte(VMPTR) (*(vmPtr->instructionPtr++))

/*
 * Reads the next literal value in the specified
 * virtual machine and advances the instruction pointer
 */
#define readLiteral(VMPTR) \
    unLiteralsGet( \
        &(VMPTR->programPtr->literals), \
        readByte(VMPTR) \
    )

/*
 * Runs the specified virtual machine
 */
static UNInterpretResult unVirtualMachineRun(
    UNVirtualMachine *vmPtr
){
    //todo run the virtual machine
    uint8_t instruction = 0;
    while(true){
        /* read next instruction opcode */
        instruction = readByte(vmPtr);
        switch(instruction){
            case UN_LITERAL: {
                UNValue literal = readLiteral(vmPtr);
                unValuePrint(literal);
                printf("\n");
                break;
            }
            case UN_RETURN:
                return UN_OK; //todo: temp just exit
        }
    }
}

/*
 * Makes the specified virtual machine start
 * interpreting the specified program
 */
UNInterpretResult unVirtualMachineInterpret(
    UNVirtualMachine *vmPtr,
    UNProgram *programPtr
){
    unVirtualMachineReset(vmPtr);
    vmPtr->programPtr = programPtr;
    return unVirtualMachineRun(vmPtr);
}

/*
 * Resets the state of the given UNVirtualMachine
 */
void unVirtualMachineReset(UNVirtualMachine *vmPtr){
    vmPtr->programPtr = NULL;
    vmPtr->instructionPtr = 0;
    //todo: no state yet in vm
}

/*
 * Frees the memory associated with the given
 * UNVirtualMachine
 */
void unVirtualMachineFree(UNVirtualMachine *vmPtr){
    //todo: no state yet in vm
    memset(vmPtr, 0, sizeof(*vmPtr));
}