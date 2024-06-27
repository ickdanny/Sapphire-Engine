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

/*
 * Pushes the specified value onto the stack of the
 * specified virtual machine
 */
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

/*
 * Pops the topmost value off the stack of the
 * specified virtual machine
 */
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
 * Peeks at the topmost value on the stack of the
 * specified virtual machine
 */
UNValue unVirtualMachineStackPeek(
    UNVirtualMachine *vmPtr,
    size_t distanceFromTop
){
    return vmPtr->stackPtr[-1 - distanceFromTop];
}

/*
 * Throws a runtime error for the specified virtual
 * machine
 */
void unVirtualMachineRuntimeError(
    UNVirtualMachine *vmPtr,
    const char *msg
){
    #define bufferSize 32
    char buffer[bufferSize] = {0};
    pgWarning("Unknown runtime error");
    pgWarning(msg);
    size_t instructionIndex
        = vmPtr->instructionPtr
            - (uint8_t*)vmPtr->programPtr->code._ptr
                - 1;
    uint16_t lineNumber = arrayListGet(uint16_t,
        &(vmPtr->programPtr->lineNumbers),
        instructionIndex
    );
    snprintf(
        buffer,
        bufferSize - 1,
        "line %u",
        lineNumber
    );
    pgError(buffer);
    
    #undef bufferSize
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
#define binaryOperation(VMPTR, VALUETYPE, OP) \
    do{ \
        if(unVirtualMachineStackPeek((VMPTR), 0).type \
                != un_number \
            || unVirtualMachineStackPeek((VMPTR), 1) \
                .type != un_number \
        ){ \
            unVirtualMachineRuntimeError( \
                (VMPTR), \
                "Operand should be numbers" \
            ); \
            return un_runtimeError; \
        } \
        double b = unAsNumber( \
            unVirtualMachineStackPop(VMPTR) \
        ); \
        double a = unAsNumber( \
            unVirtualMachineStackPop(VMPTR) \
        ); \
        unVirtualMachineStackPush( \
            VMPTR, \
            VALUETYPE(a OP b) \
        ); \
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

        /* debug printing */
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
                break;
            }
            case un_true: {
                unVirtualMachineStackPush(
                    vmPtr,
                    unBoolValue(true)
                );
                break;
            }
            case un_false: {
                unVirtualMachineStackPush(
                    vmPtr,
                    unBoolValue(false)
                );
                break;
            }
            case un_add: {
                binaryOperation(
                    vmPtr,
                    unNumberValue,
                    +
                );
                break;
            }
            case un_subtract: {
                binaryOperation(
                    vmPtr,
                    unNumberValue,
                    -
                );
                break;
            }
            case un_multiply: {
                binaryOperation(
                    vmPtr,
                    unNumberValue,
                    *
                );
                break;
            }
            case un_divide: {
                binaryOperation(
                    vmPtr,
                    unNumberValue,
                    /
                );
                break;
            }
            case un_negate: {
                if(unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    ).type != un_number
                ){
                    unVirtualMachineRuntimeError(
                        vmPtr,
                        "Operand of unary '-' should "
                        "be number"
                    );
                    return un_runtimeError;
                }
                unVirtualMachineStackPush(
                    vmPtr,
                    unNumberValue(-unAsNumber(
                        unVirtualMachineStackPop(vmPtr)
                    ))
                );
                break;
            }
            case un_equal: {
                UNValue b = unVirtualMachineStackPop(
                    vmPtr
                );
                UNValue a = unVirtualMachineStackPop(
                    vmPtr
                );
                unVirtualMachineStackPush(
                    vmPtr,
                    unBoolValue(unValueEquals(a, b))
                );
                break;
            }
            case un_greater: {
                binaryOperation(
                    vmPtr,
                    unBoolValue,
                    >
                );
                break;
            }
            case un_less: {
                binaryOperation(
                    vmPtr,
                    unBoolValue,
                    <
                );
                break;
            }
            case un_not: {
                if(unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    ).type != un_bool
                ){
                    unVirtualMachineRuntimeError(
                        vmPtr,
                        "Operand of unary '!' should "
                        "be bool"
                    );
                    return un_runtimeError;
                }
                unVirtualMachineStackPush(
                    vmPtr,
                    unBoolValue(!unAsBool(
                        unVirtualMachineStackPop(vmPtr)
                    ))
                );
                break;
            }
            case un_return:{
                unValuePrint(unVirtualMachineStackPop(
                    vmPtr
                ));
                printf("\n");
                return un_ok;
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