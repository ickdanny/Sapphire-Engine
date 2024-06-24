#include "Unknown_Program.h"

#include "Unknown_Instructions.h"

#define codeInitCapacity 8

/* Constructs and returns a new UNProgram by value */
UNProgram unProgramMake(){
    UNProgram toRet = {0};
    toRet.code = arrayListMake(uint8_t,
        codeInitCapacity
    );
    toRet.lineNumbers = arrayListMake(uint16_t,
        codeInitCapacity
    );
    toRet.literals = unLiteralsMake();
    return toRet;
}

/*
 * Pushes the specified byte to the back of the
 * specified program's code
 */
void unProgramPushBackCode(
    UNProgram *programPtr,
    uint8_t byte,
    uint16_t lineNumber
){
    arrayListPushBack(uint8_t,
        &(programPtr->code),
        byte
    );
    arrayListPushBack(uint16_t,
        &(programPtr->lineNumbers),
        lineNumber
    );
}

/*
 * Pushes the specified todo UNValue to the back of the
 * specified program's literals and returns the index
 * where it was written
 */
size_t unProgramPushBackLiteral(
    UNProgram *programPtr,
    UNValue literal
){
    return unLiteralsPushBack(
        &(programPtr->literals),
        literal
    );
}

/*
 * Prints out the disassembly of the specified program 
 */
void unProgramDisassemble(UNProgram *programPtr){
    for(size_t offset = 0;
        offset < programPtr->code.size;
    ){
        offset = unProgramDisassembleInstruction(
            programPtr,
            offset
        );
    }
}

/*
 * Prints out the disassembly of a simple instruction
 * that occupies a single byte and returns the new
 * offset
 */
size_t printSimpleInstruction(
    const char *name,
    size_t offset
){
    printf("%s\n", name);
    return offset + 1;
}

/*
 * Prints out the disassembly of a literal instruction
 * and returns the new offset
 */
size_t printLiteralInstruction(
    const char *name,
    UNProgram *programPtr,
    size_t offset
){
    uint8_t litIndex = arrayListGet(uint8_t,
        &(programPtr->code),
        offset + 1
    );
    UNValue literal = unLiteralsGet(
        &(programPtr->literals),
        litIndex
    );
    printf("%-8s %4d '", name, litIndex);
    unValuePrint(literal);
    printf("'\n");
    return offset + 2;
}

/*
 * Prints out the disassembly of a single instruction
 * in the specified program and returns the new offset
 */
size_t unProgramDisassembleInstruction(
    UNProgram *programPtr,
    size_t offset
){
    /* print the address of the instruction */
    printf("%04lu ", offset);

    /* print line info */
    if(offset > 0
        && arrayListGet(uint16_t,
            &(programPtr->lineNumbers),
            offset
        )
            == arrayListGet(uint16_t,
                &(programPtr->lineNumbers),
                offset - 1
            )
    ){
        printf("    | ");
    }
    else{
        printf("%5d ", arrayListGet(uint16_t,
            &(programPtr->lineNumbers),
            offset
        ));
    }

    uint8_t instruction = arrayListGet(uint8_t,
        &(programPtr->code),
        offset
    );
    switch(instruction){
        case UN_LITERAL:
            return printLiteralInstruction(
                "LIT",
                programPtr,
                offset
            );
        case UN_RETURN:
            return printSimpleInstruction(
                "RET",
                offset
            );
        default:
            printf(
                "unknown bad opcode %d\n",
                instruction
            );
            return offset + 1;
    }
}

/*
 * Frees the memory associated with the specified
 * program
 */
void unProgramFree(UNProgram *programPtr){
    arrayListFree(uint8_t, &(programPtr->code));
    arrayListFree(uint16_t,
        &(programPtr->lineNumbers)
    );
    unLiteralsFree(&(programPtr->literals));
    memset(programPtr, 0, sizeof(*programPtr));
}