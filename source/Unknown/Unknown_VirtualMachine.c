#include "Unknown_VirtualMachine.h"

#include "Unknown_Instructions.h"
#include "Unknown_Object.h"

#define stringMapInitCapacity 50
#define globalsMapInitCapacity 20

/*
 * Constructs and returns a new UNVirtualMachine by
 * value
 */
UNVirtualMachine unVirtualMachineMake(){
    UNVirtualMachine toRet = {0};
    /*
     * allocate the globals map; only free when
     * VM is freed, cleared upon reset
     */
    toRet.globalsMap = hashMapMake(
        UNObjectString*,
        UNValue,
        globalsMapInitCapacity,
        _unObjectStringPtrHash,
        _unObjectStringPtrEquals
    );

    /*
     * do not allocate the string map; defer to when
     * the virtual machine actually starts to run
     * a program (since we copy the map from literals)
     */
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
#define readByte(VMPTR) \
    (*(vmPtr->instructionPtr++))

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
 * Reads the next string in the specified virtual
 * machine and advances the instruction pointer
 */
#define readString(VMPTR) \
    unObjectAsString(readLiteral(VMPTR))

/*
 * Performs a binary arithmetic operation in the
 * specified virtual machine
 */
#define binaryOperation(VMPTR, VALUETYPE, OP) \
    do{ \
        if(!unIsNumber( \
                unVirtualMachineStackPeek((VMPTR), 0) \
            ) \
            || !unIsNumber( \
                unVirtualMachineStackPeek((VMPTR), 1) \
            ) \
        ){ \
            unVirtualMachineRuntimeError( \
                (VMPTR), \
                "Operands should be numbers" \
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
 * Concatenates two strings for the specified virtual
 * machine and pushes the result to the stack
 */
static void unVirtualMachineConcatenate(
    UNVirtualMachine *vmPtr
){
    UNObjectString *b = unObjectAsString(
        unVirtualMachineStackPop(vmPtr)
    );
    UNObjectString *a = unObjectAsString(
        unVirtualMachineStackPop(vmPtr)
    );
    UNObjectString *concatenation
        = unObjectStringConcat(
            a,
            b,
            &(vmPtr->objectListHeadPtr),
            &(vmPtr->stringMap)
        );
    unVirtualMachineStackPush(
        vmPtr,
        unObjectValue(concatenation)
    );
}

/*
 * Runs the specified virtual machine
 */
static UNInterpretResult unVirtualMachineRun(
    UNVirtualMachine *vmPtr
){
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
            case un_pop: {
                unVirtualMachineStackPop(vmPtr);
                break;
            }
            case un_defineGlobal: {
                /*
                 * associate the name of the global
                 * with its value (top of stack)
                 */
                UNObjectString *name
                    = readString(vmPtr);
                UNValue value
                    = unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    );
                hashMapPutPtr(
                    UNObjectString*,
                    UNValue,
                    &(vmPtr->globalsMap),
                    &name,
                    &value
                );
                unVirtualMachineStackPop(vmPtr);
                break;
            }
            case un_getGlobal: {
                /*
                 * get the name of the global from the
                 * top of the stack
                 */
                UNObjectString *name
                    = readString(vmPtr);
                if(!hashMapHasKey(
                    UNObjectString*,
                    UNValue,
                    &(vmPtr->globalsMap),
                    name
                )){
                    pgWarning(name->string._ptr);
                    unVirtualMachineRuntimeError(
                        vmPtr,
                        "undefined variable" 
                    );
                    return un_runtimeError;
                }
                UNValue value = hashMapGet(
                    UNObjectString*,
                    UNValue,
                    &(vmPtr->globalsMap),
                    name
                );
                unVirtualMachineStackPush(
                    vmPtr,
                    value
                );
                break;
            }
            case un_setGlobal: {
                /*
                 * get the name of the global from the
                 * top of the stack
                 */
                UNObjectString *name
                    = readString(vmPtr);
                if(!hashMapHasKey(
                    UNObjectString*,
                    UNValue,
                    &(vmPtr->globalsMap),
                    name
                )){
                    pgWarning(name->string._ptr);
                    unVirtualMachineRuntimeError(
                        vmPtr,
                        "undefined variable" 
                    );
                    return un_runtimeError;
                }
                UNValue value
                    = unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    );
                hashMapPutPtr(
                    UNObjectString*,
                    UNValue,
                    &(vmPtr->globalsMap),
                    &name,
                    &value
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
                /*
                 * if both operands are strings,
                 * concatenate them
                 */
                if(unIsString(
                        unVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) && unIsString(
                        unVirtualMachineStackPeek(
                            vmPtr, 1
                        )
                    )
                ){
                    unVirtualMachineConcatenate(vmPtr);
                }
                /*
                 * otherwise, they could be two numbers
                 */
                else if(unIsNumber(
                        unVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) && unIsNumber(
                        unVirtualMachineStackPeek(
                            vmPtr, 1
                        )
                    )
                ){
                    double b = unAsNumber(
                        unVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    double a = unAsNumber(
                        unVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    unVirtualMachineStackPush(
                        vmPtr,
                        unNumberValue(a + b)
                    );
                }
                /* otherwise, error */
                else{
                    unVirtualMachineRuntimeError(
                        vmPtr,
                        "Operands of '+' should be "
                        "numbers or strings"
                    );
                    return un_runtimeError;
                }
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
                if(!unIsNumber(
                    unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
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
                if(!unIsBool(
                    unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
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
            case un_print:{
                unValuePrint(unVirtualMachineStackPop(
                    vmPtr
                ));
                printf("\n");
                break;
            }
            case un_return:{
                //todo: return temporarily does nothing
                printf("(return not implemented)\n");
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
    /*
     * copy compile-time strings from the program
     * literals into the runtime string interning
     * hashmap; the strings are owned either by the
     * literals, in the case of compile-time strings,
     * or by the object list, in the case of runtime
     * strings, and should not be freed from the map
     * itself.
     */
    vmPtr->stringMap = hashMapCopy(
        UNObjectString*,
        UNValue,
        &(programPtr->literals.stringMap)
    );
    vmPtr->stringMapAllocated = true;

    return unVirtualMachineRun(vmPtr);
}

/*
 * Frees all objects in the object list of the given
 * virtual machine
 */
static void unVirtualMachineFreeObjects(
    UNVirtualMachine *vmPtr
){
    UNObject *currentPtr = vmPtr->objectListHeadPtr;
    UNObject *nextPtr = NULL;
    while(currentPtr){
        nextPtr = currentPtr->nextPtr;
        unObjectFree(currentPtr);
        currentPtr = nextPtr;
    }
}

/*
 * Frees the string map of the given virtual machine
 * if it is allocated, does nothing otherwise
 */
static void unVirtualMachineFreeStringMap(
    UNVirtualMachine *vmPtr
){
    /*
     * Free the string map but not any of the strings
     * themselves since they are not owned by the
     * string map
     */
    if(vmPtr->stringMapAllocated){
        hashMapFree(UNObjectString*, UNValue,
            &(vmPtr->stringMap)
        );
        vmPtr->stringMapAllocated = false;
    }
}

/*
 * Resets the state of the given UNVirtualMachine
 */
void unVirtualMachineReset(UNVirtualMachine *vmPtr){
    vmPtr->programPtr = NULL;
    vmPtr->instructionPtr = 0;
    vmPtr->stackPtr = vmPtr->stack;
    unVirtualMachineFreeObjects(vmPtr);
    unVirtualMachineFreeStringMap(vmPtr);
    hashMapClear(
        UNObjectString*,
        UNValue,
        &(vmPtr->globalsMap)
    );
}

/*
 * Frees the memory associated with the given
 * UNVirtualMachine
 */
void unVirtualMachineFree(UNVirtualMachine *vmPtr){
    unVirtualMachineFreeObjects(vmPtr);
    unVirtualMachineFreeStringMap(vmPtr);
    hashMapFree(
        UNObjectString*,
        UNValue,
        &(vmPtr->globalsMap)
    );
    memset(vmPtr, 0, sizeof(*vmPtr));
}