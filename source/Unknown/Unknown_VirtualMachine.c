#include "Unknown_VirtualMachine.h"

#include "Unknown_Instructions.h"

/*
 * Constructs and returns a new UNVirtualMachine by
 * value
 */
UNVirtualMachine unVirtualMachineMake(){
    UNVirtualMachine toRet = {0};
    return toRet;
}

/* Pushes the specified value onto the stack */
void unVirtualMachineStackPush(
    UNVirtualMachine *vmPtr,
    UNValue value
){
    if(vmPtr->stackPtr
        == vmPtr->stack + UN_STACK_SIZE
    ){
        pgError("Unknown stack overflow\n");
    }
    *(vmPtr->stackPtr) = value;
    ++(vmPtr->stackPtr);
}

/* Pops the topmost value off the stack */
UNValue unVirtualMachineStackPop(
    UNVirtualMachine *vmPtr
){
    if(vmPtr->stackPtr == vmPtr->stack){
        pgError("Unknown stack underflow\n");
    }
    --(vmPtr->stackPtr);
    return *(vmPtr->stackPtr);
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
 * Performs a binary arithmetic operation in the
 * specified virtual machine
 */
#define binaryOperation(VMPTR, OP) \
    do{ \
        UNValue b = unVirtualMachineStackPop(VMPTR); \
        UNValue a = unVirtualMachineStackPop(VMPTR); \
        unVirtualMachineStackPush(VMPTR, a OP b); \
    } while(false)

/*
 * Runs the specified virtual machine
 */
static UNInterpretResult unVirtualMachineRun(
    UNVirtualMachine *vmPtr
){
    //todo run the virtual machine
    uint8_t instruction = 0;
    while(true){
        #ifdef _DEBUG
        printf("Stack:");
        for(UNValue *slotPtr = vmPtr->stack;
            slotPtr < vmPtr->stackPtr;
            ++slotPtr
        ){
            printf("[");
            unValuePrint(*slotPtr);
            printf("]");
        }
        printf("\n");
        unProgramDisassembleInstruction(
            vmPtr->programPtr,
            (size_t)(vmPtr->instructionPtr
                - (uint8_t*)vmPtr->programPtr
                    ->code._ptr)
        );
        #endif
        /* read next instruction opcode */
        instruction = readByte(vmPtr);
        switch(instruction){
            case un_literal: {
                UNValue literal = readLiteral(vmPtr);
                unVirtualMachineStackPush(
                    vmPtr,
                    literal
                );
                unValuePrint(literal);
                printf("\n");
                break;
            }
            case un_add: {
                binaryOperation(vmPtr, +);
                break;
            }
            case un_subtract: {
                binaryOperation(vmPtr, -);
                break;
            }
            case un_multiply: {
                binaryOperation(vmPtr, *);
                break;
            }
            case un_divide: {
                binaryOperation(vmPtr, /);
                break;
            }
            case un_negate: {
                unVirtualMachineStackPush(
                    vmPtr,
                    -unVirtualMachineStackPop(vmPtr)
                );
                break;
            }
            case un_return:{
                unValuePrint(unVirtualMachineStackPop(
                    vmPtr
                ));
                printf("\n");
                return UN_OK;
            }
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
    vmPtr->instructionPtr = unProgramGetEntryPoint(
        programPtr
    );
    return unVirtualMachineRun(vmPtr);
}

/*
 * Resets the state of the given UNVirtualMachine
 */
void unVirtualMachineReset(UNVirtualMachine *vmPtr){
    vmPtr->programPtr = NULL;
    vmPtr->instructionPtr = 0;
    vmPtr->stackPtr = vmPtr->stack;
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