#ifndef UNKNOWN_COMPILER_H
#define UNKNOWN_COMPILER_H

#include <stdbool.h>

#include "Unknown_Lexer.h"
#include "Unknown_Program.h"
#include "Unknown_Instructions.h"

/*
 * Stores data related to the compilation process of
 * a single file
 */
typedef struct UNCompiler{
    UNToken currentToken;
    UNToken prevToken;
    UNLexer lexer;
    UNProgram compiledProgram;
    bool hadError;
    bool inPanicMode;
} UNCompiler;

/* Constructs and returns a new UNCompiler by value */
UNCompiler unCompilerMake();

/* Parses the next number for the specified compiler */
void unCompilerNumber(UNCompiler *compilerPtr);

/*
 * Parses the next unary operator for the specified
 * compiler
 */
void unCompilerUnary(UNCompiler *compilerPtr);

/*
 * Parses the next infix binary operator for the
 * specified compiler
 */
void unCompilerBinary(UNCompiler *compilerPtr);

/*
 * Parses the next expression for the specified
 * compiler
 */
void unCompilerExpression(UNCompiler *compilerPtr);

/*
 * Parses an expression within a set of parenthesis
 * for the specified compiler - should be called after
 * the first parenthesis has been matched
 */
void unCompilerGrouping(UNCompiler *compilerPtr);

/*
 * Parses a function call for the specified compiler
 * //todo
 */
void unCompilerCall(UNCompiler *compilerPtr);

/*
 * Parses a variable for the specified compiler
 * //todo
 */
void unCompilerVariable(UNCompiler *compilerPtr);

/*
 * Parses a string for the specified compiler
 * //todo
 */
void unCompilerString(UNCompiler *compilerPtr);

/*
 * Parses a boolean AND for the specified compiler
 * //todo
 */
void unCompilerAnd(UNCompiler *compilerPtr);

/*
 * Parses a boolean OR for the specified compiler
 * //todo
 */
void unCompilerOr(UNCompiler *compilerPtr);

/*
 * Parses a bool for the specified compiler
 * //todo
 */
void unCompilerBool(UNCompiler *compilerPtr);

/*
 * Parses a dot for the specified compiler
 * //todo
 */
void unCompilerDot(UNCompiler *compilerPtr);

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