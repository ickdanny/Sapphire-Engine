#include "Necro_Program.h"

#include "Necro_Instruction.h"

#define codeInitCapacity 8

/*
 * Constructs and returns a new NecroProgram by value;
 * the enclosing program pointer is nullable
 */
NecroProgram necroProgramMake(
    NecroProgram *enclosingPtr
){
    NecroProgram toRet = {0};
    toRet.code = arrayListMake(uint8_t,
        codeInitCapacity
    );
    toRet.lineNumbers = arrayListMake(uint16_t,
        codeInitCapacity
    );
    if(enclosingPtr){
        toRet.literals = necroLiteralsMake(
            enclosingPtr->literals.stringMapPtr
        );
    }
    else{
        toRet.literals = necroLiteralsMake(NULL);
    }
    return toRet;
}

/*
 * Returns a pointer to the first instruction in the
 * specified NecroProgram; returns NULL if not possible
 */
uint8_t *necroProgramGetEntryPoint(
    NecroProgram *programPtr
){
    if(!programPtr){
        return NULL;
    }
    if(programPtr->code.size == 0){
        return NULL;
    }
    return arrayListFrontPtr(uint8_t,
        &(programPtr->code)
    );
}

/*
 * Pushes the specified byte to the back of the
 * specified program's code
 */
void necroProgramPushBackCode(
    NecroProgram *programPtr,
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
 * Pushes the specified NecroValue to the back of the
 * specified program's literals and returns the index
 * where it was written
 */
size_t necroProgramPushBackLiteral(
    NecroProgram *programPtr,
    NecroValue literal
){
    return necroLiteralsPushBack(
        &(programPtr->literals),
        literal
    );
}

/*
 * Prints out the disassembly of the specified program 
 */
void necroProgramDisassemble(NecroProgram *programPtr){
    for(size_t offset = 0;
        offset < programPtr->code.size;
    ){
        offset = necroProgramDisassembleInstruction(
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
static size_t printSimpleInstruction(
    const char *name,
    size_t offset
){
    printf("%s\n", name);
    return offset + 1;
}

/*
 * Prints out the disassembly of a 2-byte instruction
 * with the second byte printed out as an integer
 */
static size_t printByteInstruction(
    const char *name,
    NecroProgram *programPtr,
    int offset
){
    /* get the slot from the next byte in the code */
    uint8_t slot = arrayListGet(uint8_t,
        &(programPtr->code),
        offset + 1
    );

    printf("%-8s %4d\n", name, slot);
    return offset + 2;
}

/* Prints out the disassembly of a jump instruction */
static size_t printJumpInstruction(
    const char *name,
    int sign,
    NecroProgram *programPtr,
    int offset
){
    uint16_t jump = (uint16_t)arrayListGet(uint8_t,
        &(programPtr->code),
        offset + 1
    );
    jump <<= 8;
    jump |= arrayListGet(uint8_t,
        &(programPtr->code),
        offset + 2
    );

    printf(
        "%-8s %4d -> %d\n",
        name,
        offset,
        offset + 3 + sign * jump
    );
    return offset + 3;
}

/*
 * Prints out the disassembly of a literal instruction
 * and returns the new offset
 */
static size_t printLiteralInstruction(
    const char *name,
    NecroProgram *programPtr,
    size_t offset
){
    uint8_t litIndex = arrayListGet(uint8_t,
        &(programPtr->code),
        offset + 1
    );
    NecroValue literal = necroLiteralsGet(
        &(programPtr->literals),
        litIndex
    );
    printf("%-8s %4d '", name, litIndex);
    necroValuePrint(literal);
    printf("'\n");
    return offset + 2;
}

/*
 * Prints out the disassembly of a single instruction
 * in the specified program and returns the new offset
 */
size_t necroProgramDisassembleInstruction(
    NecroProgram *programPtr,
    size_t offset
){
    /* print the address of the instruction */
    printf("%04lu ", (unsigned long)offset);

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
        case necro_literal:
            return printLiteralInstruction(
                "LIT",
                programPtr,
                offset
            );
        case necro_pop:
            return printSimpleInstruction(
                "POP",
                offset
            );
        case necro_defineGlobal:
            return printLiteralInstruction(
                "DEFGLOB",
                programPtr,
                offset
            );
        case necro_getGlobal:
            return printLiteralInstruction(
                "GETGLOB",
                programPtr,
                offset
            );
        case necro_setGlobal:
            return printLiteralInstruction(
                "SETGLOB",
                programPtr,
                offset
            );
        case necro_getLocal:
            return printByteInstruction(
                "GETLOCAL",
                programPtr,
                offset
            );
        case necro_setLocal:
            return printByteInstruction(
                "SETLOCAL",
                programPtr,
                offset
            );
        case necro_true:
            return printSimpleInstruction(
                "TRUE",
                offset
            );
        case necro_false:
            return printSimpleInstruction(
                "FALSE",
                offset
            );
        case necro_add:
            return printSimpleInstruction(
                "ADD",
                offset
            );
        case necro_subtract:
            return printSimpleInstruction(
                "SUB",
                offset
            );
        case necro_multiply:
            return printSimpleInstruction(
                "MUL",
                offset
            );
        case necro_divide:
            return printSimpleInstruction(
                "DIV",
                offset
            );
        case necro_modulo:
            return printSimpleInstruction(
                "MOD",
                offset
            );
        case necro_negate:
            return printSimpleInstruction(
                "NEG",
                offset
            );
        case necro_equal:
            return printSimpleInstruction(
                "EQUAL",
                offset
            );
        case necro_greater:
            return printSimpleInstruction(
                "GREATER",
                offset
            );
        case necro_less:
            return printSimpleInstruction(
                "LESS",
                offset
            );
        case necro_not:
            return printSimpleInstruction(
                "NOT",
                offset
            );
        case necro_makeVector:
            return printSimpleInstruction(
                "VECTOR",
                offset
            );
        case necro_makePoint:
            return printSimpleInstruction(
                "POINT",
                offset
            );
        case necro_getR:
            return printSimpleInstruction(
                "GETR",
                offset
            );
        case necro_getTheta:
            return printSimpleInstruction(
                "GETT",
                offset
            );
        case necro_getX:
            return printSimpleInstruction(
                "GETX",
                offset
            );
        case necro_getY:
            return printSimpleInstruction(
                "GETY",
                offset
            );
        case necro_setRGlobal:
            return printSimpleInstruction(
                "SETRG",
                offset
            );
        case necro_setThetaGlobal:
            return printSimpleInstruction(
                "SETTG",
                offset
            );
        case necro_setXGlobal:
            return printSimpleInstruction(
                "SETXG",
                offset
            );
        case necro_setYGlobal:
            return printSimpleInstruction(
                "SETYG",
                offset
            );
        case necro_setRLocal:
            return printSimpleInstruction(
                "SETRL",
                offset
            );
        case necro_setThetaLocal:
            return printSimpleInstruction(
                "SETTL",
                offset
            );
        case necro_setXLocal:
            return printSimpleInstruction(
                "SETXL",
                offset
            );
        case necro_setYLocal:
            return printSimpleInstruction(
                "SETYL",
                offset
            );
        case necro_print:
            return printSimpleInstruction(
                "PRINT",
                offset
            );
        case necro_jump:
            return printJumpInstruction(
                "JMP",
                1,
                programPtr,
                offset
            );
        case necro_jumpIfFalse:
            return printJumpInstruction(
                "JFALSE",
                1,
                programPtr,
                offset
            );
        case necro_loop:
            return printJumpInstruction(
                "LOOP",
                -1,
                programPtr,
                offset
            );
        case necro_call:
            return printByteInstruction(
                "CALL",
                programPtr,
                offset
            );
        case necro_return:
            return printSimpleInstruction(
                "RET",
                offset
            );
        case necro_yield:
            return printSimpleInstruction(
                "YIELD",
                offset
            );
        case necro_end:
            return printSimpleInstruction(
                "END",
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
void necroProgramFree(NecroProgram *programPtr){
    arrayListFree(uint8_t, &(programPtr->code));
    arrayListFree(uint16_t,
        &(programPtr->lineNumbers)
    );
    necroLiteralsFree(&(programPtr->literals));
    memset(programPtr, 0, sizeof(*programPtr));
}