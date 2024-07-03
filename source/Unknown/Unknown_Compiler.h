#ifndef UNKNOWN_COMPILER_H
#define UNKNOWN_COMPILER_H

#include <stdbool.h>

#include "Unknown_Lexer.h"
#include "Unknown_Program.h"
#include "Unknown_Instructions.h"

#define _uint8_t_count (UINT8_MAX + 1)

/*
 * Stores metadata about a single local variable for
 * a compiler
 */
typedef struct UNLocal{
    UNToken name;
    int depth;
} UNLocal;

/*
 * Stores metadata related to the process of compiling
 * local variables for a compiler
 */
typedef struct UNLocalInfo{
    UNLocal locals[_uint8_t_count];
    int localCount;
    int scopeDepth;
} UNLocalInfo;

/*
 * Stores data related to the compilation process of
 * a single file
 */
typedef struct UNCompiler{
    UNToken currentToken;
    UNToken prevToken;
    UNLexer lexer;
    UNLocalInfo localInfo;
    UNProgram compiledProgram;
    bool hadError;
    bool inPanicMode;
} UNCompiler;

/* Constructs and returns a new UNCompiler by value */
UNCompiler unCompilerMake();

/* Parses the next number for the specified compiler */
void unCompilerNumber(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses the next unary operator for the specified
 * compiler
 */
void unCompilerUnary(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses the next infix binary operator for the
 * specified compiler
 */
void unCompilerBinary(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses the next expression for the specified
 * compiler
 */
void unCompilerExpression(
    UNCompiler *compilerPtr
);

/*
 * Parses the next declaration for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void unCompilerDeclaration(UNCompiler *compilerPtr);

/*
 * Parses the next variable declaration for the
 * specified compiler; uninitialized variables get the
 * default value of FALSE
 */
void unCompilerVariableDeclaration(
    UNCompiler *compilerPtr
);

/*
 * Parses the next statement for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void unCompilerStatement(UNCompiler *compilerPtr);

/*
 * Parses the next print statement for the specified
 * compiler
 */
void unCompilerPrintStatement(UNCompiler *compilerPtr);

/*
 * Parses the next if statement for the specified
 * compiler
 */
void unCompilerIfStatement(UNCompiler *compilerPtr);

/*
 * Parses the next while statement for the specified
 * compiler
 */
void unCompilerWhileStatement(UNCompiler *compilerPtr);

/*
 * Parses the next for statement for the specified
 * compiler
 */
void unCompilerForStatement(UNCompiler *compilerPtr);

/* Parses the next block for the specified compiler */
void unCompilerBlock(UNCompiler *compilerPtr);

/*
 * Parses the next expression statement for the
 * specified compiler
 */
void unCompilerExpressionStatement(
    UNCompiler *compilerPtr
);

/*
 * Parses an expression within a set of parenthesis
 * for the specified compiler - should be called after
 * the first parenthesis has been matched
 */
void unCompilerGrouping(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a function call for the specified compiler
 */
void unCompilerCall(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a variable for the specified compiler
 */
void unCompilerVariable(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a string for the specified compiler
 */
void unCompilerString(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a boolean AND for the specified compiler
 */
void unCompilerAnd(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a boolean OR for the specified compiler
 */
void unCompilerOr(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a bool for the specified compiler
 */
void unCompilerBool(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Parses a dot for the specified compiler
 */
void unCompilerDot(
    UNCompiler *compilerPtr,
    bool canAssign
);

/*
 * Frees the memory associated with the specified
 * UNCompiler but not the generated program
 */
void unCompilerFree(UNCompiler *compilerPtr);

/*
 * compiles the specified Unknown source file and
 * returns the program; error on compiler error
 */
UNProgram unCompilerCompile(
    UNCompiler *compilerPtr,
    const char *fileName
);

#endif