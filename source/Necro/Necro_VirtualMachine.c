#include "Necro_VirtualMachine.h"

#include "Necro_Instruction.h"
#include "Necro_Object.h"

#define stringMapInitCapacity 50
#define globalsMapInitCapacity 20

/* define for verbose output */
/* #define VM_VERBOSE */

/*
 * Constructs and returns a new NecroVirtualMachine by
 * value; the native function set pointer is nullable,
 * as is the user function set pointer
 */
NecroVirtualMachine necroVirtualMachineMake(
    NecroNativeFuncSet *nativeFuncSetPtr
){
    NecroVirtualMachine toRet = {0};
    /*
     * allocate the globals map; only free when
     * VM is freed, cleared upon reset
     */
    toRet.globalsMap = hashMapMake(
        NecroObjectString*,
        NecroValue,
        globalsMapInitCapacity,
        _necroObjectStringPtrHash,
        _necroObjectStringPtrEquals
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
void necroVirtualMachineStackPush(
    NecroVirtualMachine *vmPtr,
    NecroValue value
){
    if(vmPtr->stackPtr
        == vmPtr->stack + NECRO_STACK_SIZE
    ){
        pgError("Necro stack overflow\n");
    }
    *(vmPtr->stackPtr) = value;
    ++(vmPtr->stackPtr);
}

/*
 * Pops the topmost value off the stack of the
 * specified virtual machine
 */
NecroValue necroVirtualMachineStackPop(
    NecroVirtualMachine *vmPtr
){
    if(vmPtr->stackPtr == vmPtr->stack){
        pgError("Necro stack underflow\n");
    }
    --(vmPtr->stackPtr);
    return *(vmPtr->stackPtr);
}

/*
 * Peeks at the topmost value on the stack of the
 * specified virtual machine
 */
NecroValue necroVirtualMachineStackPeek(
    NecroVirtualMachine *vmPtr,
    size_t distanceFromTop
){
    return vmPtr->stackPtr[-1 - distanceFromTop];
}

/*
 * Throws a runtime error for the specified virtual
 * machine
 */
void necroVirtualMachineRuntimeError(
    NecroVirtualMachine *vmPtr,
    const char *msg
){
    #define bufferSize 256
    static char buffer[bufferSize] = {0};
    pgWarning("Necro runtime error");
    pgWarning(msg);

    /* print out stack trace */
    pgWarning("printing stack trace: ");
    for(int i = vmPtr->frameCount - 1; i >= 0; --i){
        NecroCallFrame *framePtr
            = &(vmPtr->callStack[i]);
        NecroObjectFunc *funcPtr = framePtr->funcPtr;
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

    pgError("halting due to Necro runtime error");
    
    #undef bufferSize
}

/*
 * Associates the given C string with the given native
 * function as a global variable for the specified
 * virtual machine
 */
static void necroVirtualMachineDefineNativeFunc(
    NecroVirtualMachine *vmPtr,
    _NecroNameNativeFuncPair nameNativeFuncPair
){
    assertNotNull(
        vmPtr,
        "null vm passed to define native; "
        SRC_LOCATION
    );
    assertNotNull(
        nameNativeFuncPair._name,
        "null name passed to define native; "
        SRC_LOCATION
    );
    assertTrue(
        vmPtr->stringMapAllocated,
        "define native cannot be used before the "
        "string map is allocated; "
        SRC_LOCATION
    );
    necroVirtualMachineStackPush(
        vmPtr,
        necroObjectValue(necroObjectStringCopy(
            nameNativeFuncPair._name,
            (int)strlen(nameNativeFuncPair._name),
            &(vmPtr->objectListHeadPtr),
            &(vmPtr->stringMap)
        ))
    );
    necroVirtualMachineStackPush(
        vmPtr,
        necroObjectValue(necroObjectNativeFuncMake(
            nameNativeFuncPair._func,
            &(vmPtr->objectListHeadPtr)
        ))
    );
    NecroObjectString *namePtr = necroObjectAsString(
        vmPtr->stack[0]
    );
    /* error out if duplicate name */
    if(hashMapHasKeyPtr(NecroObjectString*, NecroValue,
        &(vmPtr->globalsMap),
        &(namePtr)
    )){
        pgWarning(nameNativeFuncPair._name);
        necroVirtualMachineRuntimeError(
            vmPtr,
            "Duplicate native function name"
        );
    }
    hashMapPutPtr(NecroObjectString*, NecroValue,
        &(vmPtr->globalsMap),
        &(namePtr),
        &(vmPtr->stack[1])
    );
    necroVirtualMachineStackPop(vmPtr);
    necroVirtualMachineStackPop(vmPtr);
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
    necroLiteralsGet( \
        &(FRAMEPTR->funcPtr->program.literals), \
        readByte(FRAMEPTR) \
    )

/*
 * Reads the next string in the specified call frame
 * and advances the instruction pointer
 */
#define readString(FRAMEPTR) \
    necroObjectAsString(readLiteral(FRAMEPTR))

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
        if(necroIsInt( \
            necroVirtualMachineStackPeek((VMPTR), 0) \
        )){ \
            if(necroIsInt( \
                necroVirtualMachineStackPeek( \
                    (VMPTR), \
                    1 \
                ) \
            )){ \
                int b = necroAsInt( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                int a = necroAsInt( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                necroVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? necroBoolValue(a OP b) \
                        : necroIntValue(a OP b) \
                ); \
            } \
            else if(necroIsFloat( \
                necroVirtualMachineStackPeek( \
                    (VMPTR), \
                    1 \
                ) \
            )){ \
                float b = necroAsInt( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                float a = necroAsFloat( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                necroVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? necroBoolValue(a OP b) \
                        : necroFloatValue(a OP b) \
                ); \
            } \
            else{ \
                necroVirtualMachineRuntimeError( \
                    (VMPTR), \
                    (ERRMSG) \
                ); \
                return necro_runtimeError; \
            } \
        } \
        else if(necroIsFloat( \
            necroVirtualMachineStackPeek((VMPTR), 0) \
        )){ \
            if(necroIsInt( \
                necroVirtualMachineStackPeek( \
                    (VMPTR), \
                    1 \
                ) \
            )){ \
                float b = necroAsFloat( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                float a = necroAsInt( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                necroVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? necroBoolValue(a OP b) \
                        : necroFloatValue(a OP b) \
                ); \
            } \
            else if(necroIsFloat( \
                necroVirtualMachineStackPeek( \
                    (VMPTR), \
                    1 \
                ) \
            )){ \
                float b = necroAsFloat( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                float a = necroAsFloat( \
                    necroVirtualMachineStackPop( \
                        (VMPTR) \
                    ) \
                ); \
                necroVirtualMachineStackPush( \
                    (VMPTR), \
                    (PUSHBOOL) \
                        ? necroBoolValue(a OP b) \
                        : necroFloatValue(a OP b) \
                ); \
            } \
            else{ \
                necroVirtualMachineRuntimeError( \
                    (VMPTR), \
                    (ERRMSG) \
                ); \
                return necro_runtimeError; \
            } \
        } \
        else{ \
            necroVirtualMachineRuntimeError( \
                (VMPTR), \
                (ERRMSG) \
            ); \
            return necro_runtimeError; \
        } \
    } while(false)

/*
 * Performs a member get operation in the specified
 * virtual machine
 */
#define memberGetOperation( \
    VMPTR, \
    TYPENAME, \
    NECROISFUNC, \
    NECROASFUNC, \
    MEMBERNAME, \
    ERRMSG \
) \
    do{ \
        if(!NECROISFUNC( \
            necroVirtualMachineStackPeek((VMPTR), 0) \
        )){ \
            necroVirtualMachineRuntimeError( \
                (VMPTR), \
                (ERRMSG) \
            ); \
            return necro_runtimeError; \
        } \
        TYPENAME composite = NECROASFUNC( \
            necroVirtualMachineStackPop((VMPTR)) \
        ); \
        necroVirtualMachineStackPush( \
            (VMPTR), \
            necroFloatValue(composite.MEMBERNAME) \
        ); \
    } while(false)

/*
 * Performs a global member set operation in the
 * specified virtual machine
 */
#define globalMemberSetOperation( \
    VMPTR, \
    FRAMEPTR, \
    NECROISFUNC, \
    VALUEASNAME, \
    MEMBERNAME, \
    ERRMSG \
) \
    do{ \
        NecroObjectString *name \
            = readString((FRAMEPTR)); \
        if(!hashMapHasKey( \
            NecroObjectString*, \
            NecroValue, \
            &((VMPTR)->globalsMap), \
            name \
        )){ \
            pgWarning(name->string._ptr); \
            pgWarning((ERRMSG)); \
            necroVirtualMachineRuntimeError( \
                (VMPTR), \
                "undefined variable; " SRC_LOCATION \
            ); \
            return necro_runtimeError; \
        } \
        NecroValue value \
            = necroVirtualMachineStackPeek( \
                (VMPTR), \
                0 \
            ); \
        float valueAsFloat = 0; \
        if(necroIsInt(value)){ \
            valueAsFloat = necroAsInt(value); \
        } \
        else if(necroIsFloat(value)){ \
            valueAsFloat = necroAsFloat(value); \
        } \
        else{ \
            pgWarning((ERRMSG)); \
            necroVirtualMachineRuntimeError( \
                (VMPTR), \
                "members can only be set to numbers" \
            ); \
            return necro_runtimeError; \
        } \
        NecroValue *globalPtr = hashMapGetPtr( \
            NecroObjectString*, \
            NecroValue, \
            &((VMPTR)->globalsMap), \
            &name \
        ); \
        if(!NECROISFUNC(*globalPtr)){ \
            pgWarning((ERRMSG)); \
            necroVirtualMachineRuntimeError( \
                (VMPTR), \
                "invalid type for member assignment" \
            ); \
            return necro_runtimeError; \
        } \
        globalPtr->as.VALUEASNAME.MEMBERNAME \
            = valueAsFloat; \
    } while(false)

/*
 * Performs a local member set operation in the
 * specified virtual machine
 */
#define localMemberSetOperation( \
    VMPTR, \
    FRAMEPTR, \
    NECROISFUNC, \
    VALUEASNAME, \
    MEMBERNAME, \
    ERRMSG \
) \
    do{ \
        uint8_t slot = readByte((FRAMEPTR)); \
        uint8_t jumps = readByte((FRAMEPTR)); \
        NecroCallFrame *frameToAccess = (FRAMEPTR); \
        for(int i = 0; i < jumps; ++i){ \
            frameToAccess \
                = frameToAccess->accessPtr; \
        } \
        NecroValue value \
            = necroVirtualMachineStackPeek( \
                (VMPTR), \
                0 \
            ); \
        float valueAsFloat = 0; \
        if(necroIsInt(value)){ \
            valueAsFloat = necroAsInt(value); \
        } \
        else if(necroIsFloat(value)){ \
            valueAsFloat = necroAsFloat(value); \
        } \
        else{ \
            pgWarning((ERRMSG)); \
            necroVirtualMachineRuntimeError( \
                (VMPTR), \
                "members can only be set to numbers" \
            ); \
            return necro_runtimeError; \
        } \
        NecroValue *localPtr \
            = &(frameToAccess->slots[slot]); \
        if(!NECROISFUNC(*localPtr)){ \
            pgWarning((ERRMSG)); \
            necroVirtualMachineRuntimeError( \
                (VMPTR), \
                "invalid type for member assignment" \
            ); \
            return necro_runtimeError; \
        } \
        localPtr->as.VALUEASNAME.MEMBERNAME \
            = valueAsFloat; \
    } while(false)

/*
 * Concatenates two strings for the specified virtual
 * machine and pushes the result to the stack
 */
static void necroVirtualMachineConcatenate(
    NecroVirtualMachine *vmPtr
){
    NecroObjectString *b = necroObjectAsString(
        necroVirtualMachineStackPop(vmPtr)
    );
    NecroObjectString *a = necroObjectAsString(
        necroVirtualMachineStackPop(vmPtr)
    );
    NecroObjectString *concatenation
        = necroObjectStringConcat(
            a,
            b,
            &(vmPtr->objectListHeadPtr),
            &(vmPtr->stringMap)
        );
    necroVirtualMachineStackPush(
        vmPtr,
        necroObjectValue(concatenation)
    );
}

/*
 * Sets up the access link for the given frame pointer
 */
static void _setAccessPtr(
    NecroCallFrame *prevPtr,
    NecroCallFrame *framePtr
){
    /*
     * set up access links according to Dragon Book
     * 7.3.6
     */
    int prevDepth = prevPtr->funcPtr->depth;
    int depth = framePtr->funcPtr->depth;
    framePtr->accessPtr = prevPtr;
    if(depth <= prevDepth){
        int numHops = prevDepth - depth + 1;
        for(int i = 0; i < numHops; ++i){
            framePtr->accessPtr
                = framePtr->accessPtr->accessPtr;
        }
    }
}

/*
 * Calls the specified function; returns true if
 * successful, false otherwise
 */
static bool necroVirtualMachineCall(
    NecroVirtualMachine *vmPtr,
    NecroObjectFunc *funcPtr,
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
        necroVirtualMachineRuntimeError(
            vmPtr,
            buffer
        );
        #undef bufferSize
        return false;
    }
    /* error if stack overflow */
    if(vmPtr->frameCount == NECRO_CALLSTACK_SIZE){
        necroVirtualMachineRuntimeError(
            vmPtr,
            "Stack overflow"
        );
        return false;
    }
    NecroCallFrame *prevPtr = &(vmPtr->callStack[
        vmPtr->frameCount - 1
    ]);
    NecroCallFrame *framePtr = &(vmPtr->callStack[
        vmPtr->frameCount++
    ]);
    framePtr->funcPtr = funcPtr;
    framePtr->instructionPtr
        = funcPtr->program.code._ptr;
    framePtr->slots = vmPtr->stackPtr - numArgs - 1;
    _setAccessPtr(prevPtr, framePtr);

    /*
     * copy strings from the function if it actually
     * owns its string map
     */
    if(copyStrings && framePtr->funcPtr->program
        .literals.ownsStringMap
    ){
        /* copy strings from the function */
        hashMapAddAllFrom(
            NecroObjectString*,
            NecroValue,
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
static bool necroVirtualMachineCallValue(
    NecroVirtualMachine *vmPtr,
    NecroValue callee,
    int numArgs
){
    if(necroIsObject(callee)){
        switch(necroObjectGetType(callee)){
            case necro_funcObject:
                return necroVirtualMachineCall(
                    vmPtr,
                    necroObjectAsFunc(callee),
                    numArgs,
                    true
                );
            case necro_nativeFuncObject: {
                NecroNativeFunc nativeFunc
                    = necroObjectAsNativeFunc(callee)
                        ->func;
                NecroValue result = nativeFunc(
                    numArgs,
                    vmPtr->stackPtr - numArgs
                );
                vmPtr->stackPtr -= numArgs + 1;
                necroVirtualMachineStackPush(
                    vmPtr,
                    result
                );
                return true;
            }
            default: /* non-callable object */
                break;
        }
    }
    necroVirtualMachineRuntimeError(
        vmPtr,
        "can only call functions"
    );
    return false;
}

/*
 * Runs the specified virtual machine
 */
static NecroInterpretResult necroVirtualMachineRun(
    NecroVirtualMachine *vmPtr
){
    NecroCallFrame *framePtr = &(
        vmPtr->callStack[vmPtr->frameCount - 1]
    );
    /* initialize instruction to 0 just to be safe */
    uint8_t instruction = 0;

    while(true){
        /* debug printing */
        #ifdef VM_VERBOSE
        printf("Stack:");
        for(NecroValue *slotPtr = vmPtr->stack;
            slotPtr < vmPtr->stackPtr;
            ++slotPtr
        ){
            printf("[");
            necroValuePrint(*slotPtr);
            printf("]");
        }
        printf("\n");
        necroProgramDisassembleInstruction(
            &(framePtr->funcPtr->program),
            (size_t)(framePtr->instructionPtr
                - (uint8_t*)framePtr->funcPtr->program
                    .code._ptr)
        );
        #endif

        /* read next instruction opcode */
        instruction = readByte(framePtr);
        switch(instruction){
            case necro_literal: {
                NecroValue literal
                    = readLiteral(framePtr);
                necroVirtualMachineStackPush(
                    vmPtr,
                    literal
                );
                break;
            }
            case necro_pop: {
                necroVirtualMachineStackPop(vmPtr);
                break;
            }
            case necro_defineGlobal: {
                /*
                 * associate the name of the global
                 * with its value (top of stack)
                 */
                NecroObjectString *name
                    = readString(framePtr);
                NecroValue value
                    = necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    );
                hashMapPutPtr(
                    NecroObjectString*,
                    NecroValue,
                    &(vmPtr->globalsMap),
                    &name,
                    &value
                );
                necroVirtualMachineStackPop(vmPtr);
                break;
            }
            case necro_getGlobal: {
                /*
                 * get the name of the global from the
                 * top of the stack
                 */
                NecroObjectString *name
                    = readString(framePtr);

                NecroValue value = {0};

                if(hashMapHasKey(
                    NecroObjectString*,
                    NecroValue,
                    &(vmPtr->globalsMap),
                    name
                )){
                    value = hashMapGet(
                        NecroObjectString*,
                        NecroValue,
                        &(vmPtr->globalsMap),
                        name
                    );
                }
                else{
                    /*
                     * special case: user defined funcs
                     * have different string interning,
                     * so try to do a full O(n)
                     * charwise string compare
                     */
                    vmPtr->globalsMap._equalsFunc
                        = _necroObjectStringPtrCharwiseEquals;

                    if(hashMapHasKey(
                        NecroObjectString*,
                        NecroValue,
                        &(vmPtr->globalsMap),
                        name
                    )){
                        value = hashMapGet(
                            NecroObjectString*,
                            NecroValue,
                            &(vmPtr->globalsMap),
                            name
                        );
                        vmPtr->globalsMap._equalsFunc
                            = _necroObjectStringPtrEquals;
                    }
                    else{
                        pgWarning(name->string._ptr);
                        necroVirtualMachineRuntimeError(
                            vmPtr,
                            "undefined variable; " 
                            SRC_LOCATION
                        );
                        vmPtr->globalsMap._equalsFunc
                            = _necroObjectStringPtrEquals;
                        return necro_runtimeError;
                    }
                }
                necroVirtualMachineStackPush(
                    vmPtr,
                    value
                );
                break;
            }
            case necro_setGlobal: {
                /*
                 * get the name of the global from the
                 * top of the stack
                 */
                NecroObjectString *name
                    = readString(framePtr);
                if(!hashMapHasKey(
                    NecroObjectString*,
                    NecroValue,
                    &(vmPtr->globalsMap),
                    name
                )){
                    pgWarning(name->string._ptr);
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "undefined variable; "
                        SRC_LOCATION
                    );
                    return necro_runtimeError;
                }
                NecroValue value
                    = necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    );
                hashMapPutPtr(
                    NecroObjectString*,
                    NecroValue,
                    &(vmPtr->globalsMap),
                    &name,
                    &value
                );
                break;
            }
            case necro_getLocal: {
                /* get the stack slot of the local */
                uint8_t slot = readByte(framePtr);
                /* get number of access jumps */
                uint8_t jumps = readByte(framePtr);

                NecroCallFrame *frameToAccess
                    = framePtr;
                for(int i = 0; i < jumps; ++i){
                    frameToAccess
                        = frameToAccess->accessPtr;
                }
                /*
                 * push the value of the local to the
                 * top of the stack; get the local
                 * relative to the call frame
                 */
                necroVirtualMachineStackPush(
                    vmPtr,
                    frameToAccess->slots[slot]
                );
                break;
            }
            case necro_setLocal: {
                /* get the stack slot of the local */
                uint8_t slot = readByte(framePtr);
                /* get number of access jumps */
                uint8_t jumps = readByte(framePtr);

                NecroCallFrame *frameToAccess
                    = framePtr;
                for(int i = 0; i < jumps; ++i){
                    frameToAccess
                        = frameToAccess->accessPtr;
                }
                /*
                 * write the value of the stack top
                 * to the slot (but don't pop it off
                 * since assignment is an expression);
                 * set the local relative to the call
                 * frame
                 */
                frameToAccess->slots[slot]
                    = necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    );
                break;
            }
            case necro_true: {
                necroVirtualMachineStackPush(
                    vmPtr,
                    necroBoolValue(true)
                );
                break;
            }
            case necro_false: {
                necroVirtualMachineStackPush(
                    vmPtr,
                    necroBoolValue(false)
                );
                break;
            }
            case necro_add: {
                /*
                 * if both operands are strings,
                 * concatenate them
                 */
                if(necroIsString(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) && necroIsString(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    )
                ){
                    necroVirtualMachineConcatenate(vmPtr);
                }
                /*
                 * otherwise if both operands are
                 * vectors, add them
                 */
                else if(
                    necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) && necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    )
                ){
                    Polar b = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Polar a = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Polar sum = polarAdd(a, b);
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroVectorValue(sum)
                    );
                }
                /* otherwise, check point + vector */
                else if(
                    necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) && necroIsPoint(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    )
                ){
                    Polar b = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Point2D a = necroAsPoint(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Point2D sum
                        = point2DAddPolar(a, b);
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroPointValue(sum)
                    );
                }
                /*
                 * otherwise, they could be two numbers
                 */
                else {
                    binaryNumberOperation(
                        vmPtr,
                        +,
                        false,
                        "Invalid operands for '+'"
                    );
                }
                break;
            }
            case necro_subtract: {
                /*
                 * if both operands are vectors,
                 * subtract them
                 */
                if(
                    necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) && necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    )
                ){
                    Polar b = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Polar a = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Polar diff = polarSubtract(a, b);
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroVectorValue(diff)
                    );
                }
                /* otherwise, check point - vector */
                else if(
                    necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) && necroIsPoint(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    )
                ){
                    Polar b = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Point2D a = necroAsPoint(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    Point2D diff
                        = point2DSubtractPolar(a, b);
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroPointValue(diff)
                    );
                }
                else{
                    binaryNumberOperation(
                        vmPtr,
                        -,
                        false,
                        "Invalid operands for '-'"
                    );
                }
                break;
            }
            case necro_multiply: {
                /*
                 * check for scalar multiplication of
                 * vector
                 */
                if(
                    necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    ) && (necroIsInt(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) || necroIsFloat(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ))
                ){
                    NecroValue aValue
                        = necroVirtualMachineStackPop(
                            vmPtr
                        );
                    Polar polar = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    float scalar = 
                        aValue.type == necro_int
                            ? necroAsInt(aValue)
                            : necroAsFloat(aValue);
                    Polar multiple = polarMultiply(
                        polar,
                        scalar
                    );
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroVectorValue(multiple)
                    );
                }
                else{
                    binaryNumberOperation(
                        vmPtr,
                        *,
                        false,
                        "Invalid operands for '*' "
                        "(in case of vector multiply, "
                        "it must be vector * scalar)"
                    );
                }
                break;
            }
            case necro_divide: {
                /*
                 * check for scalar division of
                 * vector
                 */
                if(
                    necroIsVector(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    ) && (necroIsInt(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) || necroIsFloat(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ))
                ){
                    NecroValue aValue
                        = necroVirtualMachineStackPop(
                            vmPtr
                        );
                    Polar polar = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    float scalar = 
                        aValue.type == necro_int
                            ? necroAsInt(aValue)
                            : necroAsFloat(aValue);
                    Polar quotient = polarDivide(
                        polar,
                        scalar
                    );
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroVectorValue(quotient)
                    );
                }
                else{
                    binaryNumberOperation(
                        vmPtr,
                        /,
                        false,
                        "Invalid operands for '/' "
                        "(in case of vector divide, "
                        "it must be vector / scalar)"
                    );
                }
                break;
            }
            case necro_modulo: {
                if(!necroIsInt(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            0
                        )
                    ) || !necroIsInt(
                        necroVirtualMachineStackPeek(
                            vmPtr,
                            1
                        )
                    )
                ){
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "Operands of '%' should be "
                        "integers only"
                    );
                    return necro_runtimeError;
                }
                int b = necroAsInt(
                    necroVirtualMachineStackPop(vmPtr)
                );
                int a = necroAsInt(
                    necroVirtualMachineStackPop(vmPtr)
                );
                necroVirtualMachineStackPush(
                    vmPtr,
                    necroIntValue(a % b)
                );
                break;
            }
            case necro_negate: {
                if(necroIsInt(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroIntValue(-necroAsInt(
                            necroVirtualMachineStackPop(
                                vmPtr
                            )
                        ))
                    );
                }
                else if(necroIsFloat(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroFloatValue(-necroAsFloat(
                            necroVirtualMachineStackPop(
                                vmPtr
                            )
                        ))
                    );
                }
                else if(necroIsVector(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    Polar vector = necroAsVector(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                    necroVirtualMachineStackPush(
                        vmPtr,
                        necroVectorValue(
                            polarNegate(vector)
                        )
                    );
                }
                else{
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "Operand of unary '-' should "
                        "be number or vector"
                    );
                    return necro_runtimeError;
                }
                break;
            }
            case necro_equal: {
                NecroValue b = necroVirtualMachineStackPop(
                    vmPtr
                );
                NecroValue a = necroVirtualMachineStackPop(
                    vmPtr
                );
                necroVirtualMachineStackPush(
                    vmPtr,
                    necroBoolValue(
                        necroValueEquals(a, b)
                    )
                );
                break;
            }
            case necro_greater: {
                binaryNumberOperation(
                    vmPtr,
                    >,
                    true,
                    "Operands of comparison should be "
                    "numbers"
                );
                break;
            }
            case necro_less: {
                binaryNumberOperation(
                    vmPtr,
                    <,
                    true,
                    "Operands of comparison should be "
                    "numbers"
                );
                break;
            }
            case necro_not: {
                if(!necroIsBool(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "Operand of unary '!' should "
                        "be bool"
                    );
                    return necro_runtimeError;
                }
                necroVirtualMachineStackPush(
                    vmPtr,
                    necroBoolValue(!necroAsBool(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    ))
                );
                break;
            }
            case necro_makeVector: {
                float r = 0;
                float theta = 0;

                /* top of the stack should be theta */
                if(necroIsInt(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    theta = necroAsInt(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else if(necroIsFloat(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    theta = necroAsFloat(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else{
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "Theta operand of '<>' should "
                        "be a number"
                    );
                    return necro_runtimeError;
                }
                /*
                 * after getting theta, top of stack
                 * should be R
                 */
                if(necroIsInt(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    r = necroAsInt(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else if(necroIsFloat(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    r = necroAsFloat(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else{
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "R operand of '<>' should "
                        "be a number"
                    );
                    return necro_runtimeError;
                }
                Polar vector = {r, theta};
                necroVirtualMachineStackPush(
                    vmPtr,
                    necroVectorValue(vector)
                );
                break;
            }
            case necro_makePoint: {
                float x = 0;
                float y = 0;

                /* top of the stack should be y */
                if(necroIsInt(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    y = necroAsInt(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else if(necroIsFloat(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    y = necroAsFloat(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else{
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "Y operand of '[]' should "
                        "be a number"
                    );
                    return necro_runtimeError;
                }
                /*
                 * after getting y, top of stack
                 * should be x
                 */
                if(necroIsInt(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    x = necroAsInt(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else if(necroIsFloat(
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    )
                )){
                    x = necroAsFloat(
                        necroVirtualMachineStackPop(
                            vmPtr
                        )
                    );
                }
                else{
                    necroVirtualMachineRuntimeError(
                        vmPtr,
                        "X operand of '[]' should "
                        "be a number"
                    );
                    return necro_runtimeError;
                }
                Point2D point = {x, y};
                necroVirtualMachineStackPush(
                    vmPtr,
                    necroPointValue(point)
                );
                break;
            }
            case necro_getR: {
                memberGetOperation(
                    vmPtr,
                    Polar,
                    necroIsVector,
                    necroAsVector,
                    magnitude,
                    "Expect operand of \".r\" to be "
                    "a vector"
                );
                break;
            }
            case necro_getTheta: {
                memberGetOperation(
                    vmPtr,
                    Polar,
                    necroIsVector,
                    necroAsVector,
                    angle,
                    "Expect operand of \".t\" to be "
                    "a vector"
                );
                break;
            }
            case necro_getX: {
                memberGetOperation(
                    vmPtr,
                    Point2D,
                    necroIsPoint,
                    necroAsPoint,
                    x,
                    "Expect operand of \".x\" to be "
                    "a point"
                );
                break;
            }
            case necro_getY: {
                memberGetOperation(
                    vmPtr,
                    Point2D,
                    necroIsPoint,
                    necroAsPoint,
                    y,
                    "Expect operand of \".y\" to be "
                    "a point"
                );
                break;
            }
            case necro_setRGlobal: {
                globalMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsVector,
                    vector,
                    magnitude,
                    "Error for setRGlobal"
                );
                break;
            }
            case necro_setThetaGlobal: {
                globalMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsVector,
                    vector,
                    angle,
                    "Error for setThetaGlobal"
                );
                break;
            }
            case necro_setXGlobal: {
                globalMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsPoint,
                    point,
                    x,
                    "Error for setXGlobal"
                );
                break;
            }
            case necro_setYGlobal: {
                globalMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsPoint,
                    point,
                    y,
                    "Error for setYGlobal"
                );
                break;
            }
            case necro_setRLocal: {
                localMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsVector,
                    vector,
                    magnitude,
                    "Error for setRLocal"
                );
                break;
            }
            case necro_setThetaLocal: {
                localMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsVector,
                    vector,
                    angle,
                    "Error for setThetaLocal"
                );
                break;
            }
            case necro_setXLocal: {
                localMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsPoint,
                    point,
                    x,
                    "Error for setXLocal"
                );
                break;
            }
            case necro_setYLocal: {
                localMemberSetOperation(
                    vmPtr,
                    framePtr,
                    necroIsPoint,
                    point,
                    y,
                    "Error for setYLocal"
                );
                break;
            }
            case necro_print: {
                necroValuePrint(
                    necroVirtualMachineStackPop(vmPtr)
                );
                printf("\n");
                break;
            }
            case necro_jump: {
                uint16_t offset = readShort(framePtr);
                framePtr->instructionPtr += offset;
                break;
            }
            case necro_jumpIfFalse: {
                uint16_t offset = readShort(framePtr);
                /* peek; compiler pops if needed */
                NecroValue condition
                    = necroVirtualMachineStackPeek(
                        vmPtr,
                        0
                    );
                /* jump if false */
                if(condition.type == necro_bool
                    && !necroAsBool(condition)
                ){
                    framePtr->instructionPtr += offset;
                }
                break;
            }
            case necro_loop: {
                uint16_t offset = readShort(framePtr);
                framePtr->instructionPtr -= offset;
                break;
            }
            case necro_call: {
                int numArgs = readByte(framePtr);
                if(!necroVirtualMachineCallValue(
                    vmPtr,
                    necroVirtualMachineStackPeek(
                        vmPtr,
                        numArgs
                    ),
                    numArgs
                )){
                    return necro_runtimeError;
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
            case necro_return: {
                NecroValue result
                    = necroVirtualMachineStackPop(
                        vmPtr
                    );
                --(vmPtr->frameCount);
                if(vmPtr->frameCount == 0){
                    necroVirtualMachineStackPop(vmPtr);
                    return necro_success;
                }

                vmPtr->stackPtr = framePtr->slots;
                /*
                 * push the return value back onto the
                 * stack; even "null" returns actually
                 * return the value FALSE
                 */
                necroVirtualMachineStackPush(
                    vmPtr,
                    result
                );
                framePtr = &(vmPtr->callStack[
                    vmPtr->frameCount - 1
                ]);
                break;
            }
            case necro_yield:
                return necro_yielded;
            case necro_end: 
                return necro_success;
        }
    }
    return necro_success;
}

/*
 * Loads all native functions defined in native func
 * set pointed to by the specified virtual machine
 */
static void necroVirtualMachineLoadNativeFunctions(
    NecroVirtualMachine *vmPtr
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
            ->_nameNativeFuncPairs.size;
        ++i
    ){
        necroVirtualMachineDefineNativeFunc(
            vmPtr,
            arrayListGet(_NecroNameNativeFuncPair,
                &(vmPtr->nativeFuncSetPtr
                    ->_nameNativeFuncPairs),
                i
            )
        );
    }
}

/*
 * Makes the specified virtual machine start
 * interpreting the specified program
 */
NecroInterpretResult necroVirtualMachineInterpret(
    NecroVirtualMachine *vmPtr,
    NecroObjectFunc *funcObjectProgramPtr
){
    necroVirtualMachineLoad(
        vmPtr,
        funcObjectProgramPtr
    );
    return necroVirtualMachineRun(vmPtr);
}

/*
 * Loads the specified program into the given virtual
 * machine but does not start running it; resume
 * should be called to run it
 */
void necroVirtualMachineLoad(
    NecroVirtualMachine *vmPtr,
    NecroObjectFunc *funcObjectProgramPtr
){
    assertNotNull(
        vmPtr,
        "null vm passed to load; "
        SRC_LOCATION
    );
    assertNotNull(
        funcObjectProgramPtr,
        "null func object passed to load; "
        SRC_LOCATION
    );
    necroVirtualMachineReset(vmPtr);

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
        NecroObjectString*,
        NecroValue,
        funcObjectProgramPtr
            ->program.literals.stringMapPtr
    );
    vmPtr->stringMapAllocated = true;

    /* load native functions */
    necroVirtualMachineLoadNativeFunctions(vmPtr);

    /* store the func on the stack */
    necroVirtualMachineStackPush(
        vmPtr,
        necroObjectValue(funcObjectProgramPtr)
    );

    /* initialize first call frame */
    necroVirtualMachineCall(
        vmPtr,
        funcObjectProgramPtr,
        0,
        false /* do not copy strings */
    );
}

/*
 * Has the specified virtual machine continue running
 * its program; should only be used if the virtual
 * machine has previously yielded
 */
NecroInterpretResult necroVirtualMachineResume(
    NecroVirtualMachine *vmPtr
){
    return necroVirtualMachineRun(vmPtr);
}

/*
 * Frees all objects in the object list of the given
 * virtual machine
 */
static void necroVirtualMachineFreeObjects(
    NecroVirtualMachine *vmPtr
){
    NecroObject *currentPtr = vmPtr->objectListHeadPtr;
    NecroObject *nextPtr = NULL;
    while(currentPtr){
        nextPtr = currentPtr->nextPtr;
        necroObjectFree(currentPtr);
        currentPtr = nextPtr;
    }
    
    vmPtr->objectListHeadPtr = NULL;
}

/*
 * Frees the string map of the given virtual machine
 * if it is allocated, does nothing otherwise
 */
static void necroVirtualMachineFreeStringMap(
    NecroVirtualMachine *vmPtr
){
    /*
     * Free the string map but not any of the strings
     * themselves since they are not owned by the
     * string map
     */
    if(vmPtr->stringMapAllocated){
        hashMapFree(NecroObjectString*, NecroValue,
            &(vmPtr->stringMap)
        );
        vmPtr->stringMapAllocated = false;
    }
}

/*
 * Resets the state of the given NecroVirtualMachine
 */
void necroVirtualMachineReset(
    NecroVirtualMachine *vmPtr
){
    vmPtr->stackPtr = vmPtr->stack;
    vmPtr->frameCount = 0;
    necroVirtualMachineFreeObjects(vmPtr);
    necroVirtualMachineFreeStringMap(vmPtr);

    /* clear all globals including native funcs */
    hashMapClear(
        NecroObjectString*,
        NecroValue,
        &(vmPtr->globalsMap)
    );
}

/*
 * Frees the memory associated with the given
 * NecroVirtualMachine
 */
void necroVirtualMachineFree(
    NecroVirtualMachine *vmPtr
){
    necroVirtualMachineFreeObjects(vmPtr);
    necroVirtualMachineFreeStringMap(vmPtr);
    hashMapFree(
        NecroObjectString*,
        NecroValue,
        &(vmPtr->globalsMap)
    );
    memset(vmPtr, 0, sizeof(*vmPtr));
}