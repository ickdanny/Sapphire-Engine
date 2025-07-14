#ifndef NECRO_COMPILER_H
#define NECRO_COMPILER_H

#include <stdbool.h>

#include "Necro_Lexer.h"
#include "Necro_Program.h"
#include "Necro_Instruction.h"
#include "Necro_Object.h"

#define _uint8_t_count (UINT8_MAX + 1)

/*
 * Stores metadata about a single local variable for
 * a compiler
 */
typedef struct NecroLocal{
    NecroToken name;
    int depth;
    //todo: needs to store whether const or not
} NecroLocal;

/*
 * Indicates what type of function is currently being
 * compiled
 */
typedef enum NecroFuncType{
    necro_invalidFuncType,
    necro_scriptFuncType, /* for top level */
    necro_functionFuncType /* for lambdas */
} NecroFuncType;

/*
 * Stores data related to the compilation process
 * of a single function (should be allocated on the
 * stack by functions)
 */
typedef struct _NecroFuncCompiler{
    struct _NecroFuncCompiler *enclosingPtr;
    NecroObjectFunc *funcPtr;
    NecroFuncType funcType;
    NecroLocal locals[_uint8_t_count];
    int localCount;
    int scopeDepth;
} _NecroFuncCompiler;

/*
 * Stores data related to the compilation process of
 * a single file
 */
typedef struct NecroCompiler{
    NecroToken currentToken;
    NecroToken prevToken;
    NecroLexer lexer;
    /*
     * does not own; func compilers live on the stack
     * and are created within functions
     */
    _NecroFuncCompiler *currentFuncCompilerPtr;
    bool hadError;
    bool inPanicMode;
} NecroCompiler;

/*
 * Constructs and returns a new NecroCompiler by value
 */
NecroCompiler necroCompilerMake();

/* Parses the next number for the specified compiler */
void necroCompilerNumber(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses the next unary operator for the specified
 * compiler
 */
void necroCompilerUnary(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses the next infix binary operator for the
 * specified compiler
 */
void necroCompilerBinary(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses the next expression for the specified
 * compiler
 */
void necroCompilerExpression(
    NecroCompiler *compilerPtr
);

/*
 * Parses the next declaration for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void necroCompilerDeclaration(
    NecroCompiler *compilerPtr
);

/*
 * Parses the next include declaration for the
 * specified compiler
 */
void necroCompilerIncludeDeclaration(
    NecroCompiler *compilerPtr
);

/*
 * Parses the next variable declaration for the
 * specified compiler; uninitialized variables get the
 * default value of FALSE
 */
void necroCompilerVariableDeclaration(
    NecroCompiler *compilerPtr,
    bool mutable
);

/*
 * Parses the next statement for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void necroCompilerStatement(NecroCompiler *compilerPtr);

/*
 * Parses the next print statement for the specified
 * compiler
 */
void necroCompilerPrintStatement(NecroCompiler *compilerPtr);

/*
 * Parses the next if statement for the specified
 * compiler
 */
void necroCompilerIfStatement(NecroCompiler *compilerPtr);

/*
 * Parses the next while statement for the specified
 * compiler
 */
void necroCompilerWhileStatement(NecroCompiler *compilerPtr);

/*
 * Parses the next for statement for the specified
 * compiler
 */
void necroCompilerForStatement(NecroCompiler *compilerPtr);

/*
 * Parses the next yield statement for the specified
 * compiler
 */
void necroCompilerYieldStatement(NecroCompiler *compilerPtr);

/*
 * Parses the next wait statement for the specified
 * compiler
 */
void necroCompilerWaitStatement(NecroCompiler *compilerPtr);

/*
 * Parses the next return statement for the specified
 * compiler
 */
void necroCompilerReturnStatement(
    NecroCompiler *compilerPtr
);

/* Parses the next block for the specified compiler */
void necroCompilerBlock(NecroCompiler *compilerPtr);

/*
 * Parses the next expression statement for the
 * specified compiler
 */
void necroCompilerExpressionStatement(
    NecroCompiler *compilerPtr
);

/*
 * Parses an expression within a set of parenthesis
 * for the specified compiler - should be called after
 * the first parenthesis has been matched
 */
void necroCompilerGrouping(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a function call for the specified compiler
 */
void necroCompilerCall(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a variable for the specified compiler
 */
void necroCompilerVariable(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a lambda for the specified compiler
 */
void necroCompilerLambda(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a string for the specified compiler
 */
void necroCompilerString(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a boolean AND for the specified compiler
 */
void necroCompilerAnd(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a boolean OR for the specified compiler
 */
void necroCompilerOr(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a bool for the specified compiler
 */
void necroCompilerBool(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/* Parses a vector for the specified compiler */
void necroCompilerVector(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/* Parses a point for the specified compiler */
void necroCompilerPoint(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a dot for the specified compiler; only
 * handles get operations
 */
void necroCompilerDot(
    NecroCompiler *compilerPtr,
    bool canAssign
);

/*
 * Frees the memory associated with the specified
 * NecroCompiler but not the generated program
 */
void necroCompilerFree(NecroCompiler *compilerPtr);

/*
 * compiles the specified Necro source file and
 * returns the program as a pointer to a newly
 * allocated NecroObjectFunc; error on compiler error
 */
NecroObjectFunc *necroCompilerCompileScript(
    NecroCompiler *compilerPtr,
    const char *fileName
);

#endif