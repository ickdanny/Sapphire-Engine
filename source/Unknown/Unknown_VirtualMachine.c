#include "Unknown_VirtualMachine.h"

#include "Unknown_Instructions.h"
#include "Unknown_Object.h"

#define stringMapInitCapacity 50
#define globalsMapInitCapacity 20

/*
 * Constructs and returns a new UNVirtualMachine by
 * value; the native function set pointer is nullable
 */
UNVirtualMachine unVirtualMachineMake(
    UNNativeFuncSet *nativeFuncSetPtr
){
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

    toRet.nativeFuncSetPtr = nativeFuncSetPtr;

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
    #define bufferSize 256
    static char buffer[bufferSize] = {0};
    pgWarning("Unknown runtime error");
    pgWarning(msg);

    /* print out stack trace */
    pgWarning("printing stack trace: ");
    for(int i = vmPtr->frameCount - 1; i >= 0; --i){
        UNCallFrame *framePtr = &(vmPtr->callStack[i]);
        UNObjectFunc *funcPtr = framePtr->funcPtr;
        size_t instructionIndex
            = framePtr->instructionPtr
                - (uint8_t*)funcPtr->program.code._ptr
                    - 1;
        uint16_t lineNumber = arrayListGet(uint16_t,
            &(framePtr->funcPtr->program.lineNumbers),
            instructionIndex
        );
        char *funcNamePtr = "unnamed script";
        if(funcPtr->namePtr != NULL){
            funcNamePtr
                = funcPtr->namePtr->string._ptr;
        }
        snprintf(
            buffer,
            bufferSize - 1,
            "line %u in %s",
            lineNumber,
            funcNamePtr
        );
        pgWarning(buffer);
    }

    pgError("halting due to Unknown runtime error");
    
    #undef bufferSize
}

/*
 * Associates the given C string with the given native
 * function as a global variable for the specified
 * virtual machine
 */
static void unVirtualMachineDefineNative(
    UNVirtualMachine *vmPtr,
    _UNNameFuncPair nameFuncPair
){
    assertNotNull(
        vmPtr,
        "null vm passed to define native; "
        SRC_LOCATION
    );
    assertNotNull(
        nameFuncPair._name,
        "null name passed to define native; "
        SRC_LOCATION
    );
    assertTrue(
        vmPtr->stringMapAllocated,
        "define native cannot be used before the "
        "string map is allocated; "
        SRC_LOCATION
    );
    unVirtualMachineStackPush(
        vmPtr,
        unObjectValue(unObjectStringCopy(
            nameFuncPair._name,
            (int)strlen(nameFuncPair._name),
            &(vmPtr->objectListHeadPtr),
            &(vmPtr->stringMap)
        ))
    );
    unVirtualMachineStackPush(
        vmPtr,
        unObjectValue(unObjectNativeFuncMake(
            nameFuncPair._func
        ))
    );
    UNObjectString *namePtr = unObjectAsString(
        vmPtr->stack[0]
    );
    /* error out if duplicate name */
    if(hashMapHasKeyPtr(UNObjectString*, UNValue,
        &(vmPtr->globalsMap),
        &(namePtr)
    )){
        pgWarning(nameFuncPair._name);
        unVirtualMachineRuntimeError(
            vmPtr,
            "Duplicate native function name"
        );
    }
    hashMapPutPtr(UNObjectString*, UNValue,
        &(vmPtr->globalsMap),
        &(namePtr),
        &(vmPtr->stack[1])
    );
    unVirtualMachineStackPop(vmPtr);
    unVirtualMachineStackPop(vmPtr);
}

/*
 * Reads the next byte in the specified call frame
 * and advances the instruction pointer
 */
#define readByte(FRAMEPTR) \
    (*(FRAMEPTR->instructionPtr++))

/*
 * Reads the next short in the specified call frame
 * and advances the instruction pointer twice
 */
#define readShort(FRAMEPTR) \
    (FRAMEPTR->instructionPtr += 2, \
        (uint16_t)((FRAMEPTR->instructionPtr[-2] \
                << 8 \
            ) | (FRAMEPTR->instructionPtr[-1])))

/*
 * Reads the next literal value in the specified call
 * frame and advances the instruction pointer
 */
#define readLiteral(FRAMEPTR) \
    unLiteralsGet( \
        &(FRAMEPTR->funcPtr->program.literals), \
        readByte(FRAMEPTR) \
    )

/*
 * Reads the next string in the specified call frame
 * and advances the instruction pointer
 */
#define readString(FRAMEPTR) \
    unObjectAsString(readLiteral(FRAMEPTR))

/*
 * Performs a binary arithmetic operation in the
 * specified virtual machine
 */
#define binaryNumberOperation( \
    VMPTR, \
    OP, \
    PUSHBOOL, \
    ERRMSG \
) \
    do{ \
        if(unIsInt( \
            unVirtualMachineStackPeek((VMPTR), 0) \
        )){ \
            if(unIsInt( \
                unVirtualMachineStackPeek((VMPTR), 1) \
            )){ \
                int b = unAsInt( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                int a = unAsInt( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                unVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? unBoolValue(a OP b) \
                        : unIntValue(a OP b) \
                ); \
            } \
            else if(unIsFloat( \
                unVirtualMachineStackPeek((VMPTR), 1) \
            )){ \
                float b = unAsInt( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                float a = unAsFloat( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                unVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? unBoolValue(a OP b) \
                        : unFloatValue(a OP b) \
                ); \
            } \
            else{ \
                unVirtualMachineRuntimeError( \
                    (VMPTR), \
                    (ERRMSG) \
                ); \
                return un_runtimeError; \
            } \
        } \
        else if(unIsFloat( \
            unVirtualMachineStackPeek((VMPTR), 0) \
        )){ \
            if(unIsInt( \
                unVirtualMachineStackPeek((VMPTR), 1) \
            )){ \
                float b = unAsFloat( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                float a = unAsInt( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                unVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? unBoolValue(a OP b) \
                        : unFloatValue(a OP b) \
                ); \
            } \
            else if(unIsFloat( \
                unVirtualMachineStackPeek((VMPTR), 1) \
            )){ \
                float b = unAsFloat( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                float a = unAsFloat( \
                    unVirtualMachineStackPop((VMPTR)) \
                ); \
                unVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? unBoolValue(a OP b) \
                        : unFloatValue(a OP b) \
                ); \
            } \
            else{ \
                unVirtualMachineRuntimeError( \
                    (VMPTR), \
                    (ERRMSG) \
                ); \
                return un_runtimeError; \
            } \
        } \
        else{ \
            unVirtualMachineRuntimeError( \
                (VMPTR), \
                (ERRMSG) \
            ); \
            return un_runtimeError; \
        } \
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
 * Calls the specified function; returns true if
 * successful, false otherwise
 */
static bool unVirtualMachineCall(
    UNVirtualMachine *vmPtr,
    UNObjectFunc *funcPtr,
    int numArgs,
    bool copyStrings
){
    /* error if arity mismatch */
    if(numArgs != funcPtr->arity){
        #define bufferSize 32
        static char buffer[bufferSize] = {0};
        snprintf(
            buffer,
            bufferSize - 1,
            "Expected %d args but got %d",
            funcPtr->arity,
            numArgs
        );
        unVirtualMachineRuntimeError(
            vmPtr,
            buffer
        );
        #undef bufferSize
        return false;
    }
    /* error if stack overflow */
    if(vmPtr->frameCount == UN_CALLSTACK_SIZE){
        unVirtualMachineRuntimeError(
            vmPtr,
            "Stack overflow"
        );
        return false;
    }
    UNCallFrame *framePtr = &(vmPtr->callStack[
        vmPtr->frameCount++
    ]);
    framePtr->funcPtr = funcPtr;
    framePtr->instructionPtr
        = funcPtr->program.code._ptr;
    framePtr->slots = vmPtr->stackPtr - numArgs - 1;

    /*
     * copy strings from the function if it actually
     * owns its string map
     */
    if(copyStrings && framePtr->funcPtr->program
        .literals.ownsStringMap
    ){
        /* copy strings from the function */
        hashMapAddAllFrom(UNObjectString*, UNValue,
            &(vmPtr->stringMap),
            funcPtr->program.literals.stringMapPtr
        );
    }

    return true;
}

/*
 * Calls the specified function passed as a value but
 * errors if the value is not a function; returns true
 * on success, false on failure
 */
static bool unVirtualMachineCallValue(
    UNVirtualMachine *vmPtr,
    UNValue callee,
    int numArgs
){
    if(unIsObject(callee)){
        switch(unObjectGetType(callee)){
            case un_funcObject:
                return unVirtualMachineCall(
                    vmPtr,
                    unObjectAsFunc(callee),
                    numArgs,
                    true
                );
            case un_nativeFuncObject: {
                UNNativeFunc nativeFunc
                    = unObjectAsNativeFunc(callee)
                        ->func;
                UNValue result = nativeFunc(
                    numArgs,
                    vmPtr->stackPtr - numArgs
                );
                vmPtr->stackPtr -= numArgs + 1;
                unVirtualMachineStackPush(
                    vmPtr,
                    result
                );
                return true;
            }
            default: /* non-callable object */
                break;
        }
    }
    unVirtualMachineRuntimeError(
        vmPtr,
        "can only call functions"
    );
    return false;
}

/*
 * Runs the specified virtual machine
 */
static UNInterpretResult unVirtualMachineRun(
    UNVirtualMachine *vmPtr
){
    UNCallFrame *framePtr = &(
        vmPtr->callStack[vmPtr->frameCount - 1]
    );
    /* initialize instruction to 0 just to be safe */
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
            &(framePtr->funcPtr->program),
            (size_t)(framePtr->instructionPtr
                - (uint8_t*)framePtr->funcPtr->program
                    .code._ptr)
        );
        #endif

        /* read next instruction opcode */
        instruction = readByte(framePtr);
        switch(instruction){
            case un_literal: {
                UNValue literal
                    = readLiteral(framePtr);
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
                    = readString(framePtr);
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
                    = readString(framePtr);
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
                    = readString(framePtr);
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
            case un_getLocal: {
                /* get the stack slot of the local */
                uint8_t slot = readByte(framePtr);
                /*
                 * push the value of the local to the
                 * top of the stack; get the local
                 * relative to the call frame
                 */
                unVirtualMachineStackPush(
                    vmPtr,
                    framePtr->slots[slot]
                );
                break;
            }
            case un_setLocal: {
                /* get the stack slot of the local */
                uint8_t slot = readByte(framePtr);
                /*
                 * write the value of the stack top
                 * to the slot (but don't pop it off
                 * since assignment is an expression);
                 * set the local relative to the call
                 * frame
                 */
                framePtr->slots[slot]
                    = unVirtualMachineStackPeek(
                        vmPtr,
                        0
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
                else {
                    binaryNumberOperation(
                        vmPtr,
                        +,
                        false,
                        "Operands of '+' should be "
                        "numbers or strings"
                    );
                }
                break;
            }
            case un_subtract: {
                binaryNumberOperation(
                    vmPtr,
                    -,
                    false,
                    "Operands of '-' should be numbers"
                );
                break;
            }
            case un_multiply: {
                binaryNumberOperation(
                    vmPtr,
                    *,
                    false,
                    "Operands of '*' should be numbers"
                );
                break;
            }
            case un_divide: {
                binaryNumberOperation(
                    vmPtr,
                    /,
                    false,
                    "Operands of '/' should be numbers"
                );
                break;
            }
            case un_modulo: {
                if(!unIsInt(
                        unVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) || !unIsInt(
                        unVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    )
                ){
                    unVirtualMachineRuntimeError(
                        vmPtr,
                        "Operands of '%' should be "
                        "integers only"
                    );
                    return un_runtimeError;
                }
                int b = unAsInt(
                    unVirtualMachineStackPop(vmPtr)
                );
                int a = unAsInt(
                    unVirtualMachineStackPop(vmPtr)
                );
                unVirtualMachineStackPush(
                    vmPtr,
                    unIntValue(a % b)
                );
                break;
            }
            case un_negate: {
                if(unIsInt(
                    unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    unVirtualMachineStackPush(
                        vmPtr,
                        unIntValue(-unAsInt(
                            unVirtualMachineStackPop(
                                vmPtr
                            )
                        ))
                    );
                }
                else if(unIsFloat(
                    unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    unVirtualMachineStackPush(
                        vmPtr,
                        unFloatValue(-unAsFloat(
                            unVirtualMachineStackPop(
                                vmPtr
                            )
                        ))
                    );
                }
                else{
                    unVirtualMachineRuntimeError(
                        vmPtr,
                        "Operand of unary '-' should "
                        "be number"
                    );
                    return un_runtimeError;
                }
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
                binaryNumberOperation(
                    vmPtr,
                    >,
                    true,
                    "Operands of comparison should be "
                    "numbers"
                );
                break;
            }
            case un_less: {
                binaryNumberOperation(
                    vmPtr,
                    <,
                    true,
                    "Operands of comparison should be "
                    "numbers"
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
            case un_print: {
                unValuePrint(unVirtualMachineStackPop(
                    vmPtr
                ));
                printf("\n");
                break;
            }
            case un_jump: {
                uint16_t offset = readShort(framePtr);
                framePtr->instructionPtr += offset;
                break;
            }
            case un_jumpIfFalse: {
                uint16_t offset = readShort(framePtr);
                /* peek; compiler pops if needed */
                UNValue condition
                    = unVirtualMachineStackPeek(
                        vmPtr,
                        0
                    );
                /* jump if false */
                if(condition.type == un_bool
                    && !unAsBool(condition)
                ){
                    framePtr->instructionPtr += offset;
                }
                break;
            }
            case un_loop: {
                uint16_t offset = readShort(framePtr);
                framePtr->instructionPtr -= offset;
                break;
            }
            case un_call: {
                int numArgs = readByte(framePtr);
                if(!unVirtualMachineCallValue(
                    vmPtr,
                    unVirtualMachineStackPeek(
                        vmPtr,
                        numArgs
                    ),
                    numArgs
                )){
                    return un_runtimeError;
                }
                /*
                 * if call value succeeded, it will
                 * create and push a new frame
                 */
                framePtr = &(vmPtr->callStack[
                    vmPtr->frameCount - 1
                ]);
                break;
            }
            case un_return: {
                UNValue result
                    = unVirtualMachineStackPop(vmPtr);
                --(vmPtr->frameCount);
                if(vmPtr->frameCount == 0){
                    unVirtualMachineStackPop(vmPtr);
                    return un_success;
                }

                vmPtr->stackPtr = framePtr->slots;
                /*
                 * push the return value back onto the
                 * stack; even "null" returns actually
                 * return the value FALSE
                 */
                unVirtualMachineStackPush(
                    vmPtr,
                    result
                );
                framePtr = &(vmPtr->callStack[
                    vmPtr->frameCount - 1
                ]);
                break;
            }
            case un_yield:
                return un_yielded;
            case un_end: 
                return un_success;
        }
    }
    return un_success;
}

/*
 * Loads all native functions defined in native func
 * set pointed to by the specified virtual machine
 */
static void unVirtualMachineLoadNativeFunctions(
    UNVirtualMachine *vmPtr
){
    assertNotNull(
        vmPtr,
        "null passed to load all native funcs; "
        SRC_LOCATION
    );

    /* do nothing if vm has no native func set */
    if(!(vmPtr->nativeFuncSetPtr)){
        return;
    }

    /* otherwise load native funcs one by one */
    for(size_t i = 0;
        i < vmPtr->nativeFuncSetPtr
            ->_nameFuncPairs.size;
        ++i
    ){
        unVirtualMachineDefineNative(
            vmPtr,
            arrayListGet(_UNNameFuncPair,
                &(vmPtr->nativeFuncSetPtr
                    ->_nameFuncPairs),
                i
            )
        );
    }
}

/*
 * Makes the specified virtual machine start
 * interpreting the specified program
 */
UNInterpretResult unVirtualMachineInterpret(
    UNVirtualMachine *vmPtr,
    UNObjectFunc *funcObjectProgramPtr
){
    unVirtualMachineReset(vmPtr);

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
        funcObjectProgramPtr
            ->program.literals.stringMapPtr
    );
    vmPtr->stringMapAllocated = true;

    /* load native functions */
    unVirtualMachineLoadNativeFunctions(vmPtr);

    /* store the func on the stack */
    unVirtualMachineStackPush(
        vmPtr,
        unObjectValue(funcObjectProgramPtr)
    );

    /* initialize first call frame */
    unVirtualMachineCall(
        vmPtr,
        funcObjectProgramPtr,
        0,
        false /* do not copy strings */
    );

    return unVirtualMachineRun(vmPtr);
}

/*
 * Has the specified virtual machine continue running
 * its program; should only be used if the virtual
 * machine has previously yielded
 */
UNInterpretResult unVirtualMachineResume(
    UNVirtualMachine *vmPtr
){
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
    vmPtr->stackPtr = vmPtr->stack;
    vmPtr->frameCount = 0;
    unVirtualMachineFreeObjects(vmPtr);
    unVirtualMachineFreeStringMap(vmPtr);

    /* clear all globals including native funcs */
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