#include "Unknown_Compiler.h"

#include <stdbool.h>
#include <stdio.h>

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

/*
 * Represents the precedence hiearchy of the grammar 
 * of Unknown from lowest to highest
 */
typedef enum UNPrecedence{
    un_precNone,
    un_precAssign,
    un_precOr,
    un_precAnd,
    un_precEquality,
    un_precCompare,
    un_precTerm,
    un_precFactor,
    un_precUnary,
    un_precCall,
    un_precPrimary,
} UNPrecedence;

/* A function which is associated with a parse rule */
typedef void (*ParseFunc)(UNCompiler*);

/*
 * Encapsulates the behavior of the compiler upon
 * encountering a token of a certain type
 */
typedef struct UNParseRule{
    ParseFunc prefixFunc;
    ParseFunc infixFunc;
    UNPrecedence precedence;
} UNParseRule;

/* table for all parse rules */
static const UNParseRule rules[] = {
    [un_tokenLeftParen]     = {grouping, call,   PREC_CALL},
    [un_tokenRightParen]    = {NULL,     NULL,   PREC_NONE},
    [un_tokenLeftBrace]     = {NULL,     NULL,   PREC_NONE},
    [un_tokenRightBrace]    = {NULL,     NULL,   PREC_NONE},
    [un_tokenComma]         = {NULL,     NULL,   PREC_NONE},
    [un_tokenDot]           = {NULL,     dot,    PREC_CALL},
    [un_tokenMinus]         = {unary,    binary, PREC_TERM},
    [un_tokenPlus]          = {NULL,     binary, PREC_TERM},
    [un_tokenSemicolon]     = {NULL,     NULL,   PREC_NONE},
    [un_tokenSlash]         = {NULL,     binary, PREC_FACTOR},
    [un_tokenStar]          = {NULL,     binary, PREC_FACTOR},
    [un_tokenBang]          = {unary,    NULL,   PREC_NONE},
    [un_tokenBangEqual]     = {NULL,     binary, PREC_EQUALITY},
    [un_tokenEqual]         = {NULL,     NULL,   PREC_NONE},
    [un_tokenDoubleEqual]   = {NULL,     binary, PREC_EQUALITY},
    [un_tokenGreater]       = {NULL,     binary, PREC_COMPARISON},
    [un_tokenGreaterEqual]  = {NULL,     binary, PREC_COMPARISON},
    [un_tokenLess]          = {NULL,     binary, PREC_COMPARISON},
    [un_tokenLessEqual]     = {NULL,     binary, PREC_COMPARISON},
    [un_tokenIdentifier]    = {variable, NULL,   PREC_NONE},
    [un_tokenString]        = {string,   NULL,   PREC_NONE},
    [un_tokenNumber]        = {number,   NULL,   PREC_NONE},
    [un_tokenAmpersand]     = {NULL,     and_,   PREC_AND},
    [un_tokenElse]          = {NULL,     NULL,   PREC_NONE},
    [un_tokenFalse]         = {literal,  NULL,   PREC_NONE},
    [un_tokenFor]           = {NULL,     NULL,   PREC_NONE},
    [un_tokenFunc]          = {NULL,     NULL,   PREC_NONE},
    [un_tokenIf]            = {NULL,     NULL,   PREC_NONE},
    [un_tokenVerticalBar]   = {NULL,     or_,    PREC_OR},
    [un_tokenPrint]         = {NULL,     NULL,   PREC_NONE},
    [un_tokenReturn]        = {NULL,     NULL,   PREC_NONE},
    [un_tokenTrue]          = {literal,  NULL,   PREC_NONE},
    [un_tokenLet]           = {NULL,     NULL,   PREC_NONE},
    [un_tokenWhile]         = {NULL,     NULL,   PREC_NONE},
    [un_tokenError]         = {NULL,     NULL,   PREC_NONE},
    [un_tokenEOF]           = {NULL,     NULL,   PREC_NONE},
};

/*
 * Constructs and returns a new UNCompiler for the
 * specified file by value
 */
static UNCompiler unCompilerMake(const char *fileName){
    UNCompiler toRet = {0};
    toRet.lexer = unLexerMake(fileName);
    toRet.compiledProgram = unProgramMake();
    toRet.hadError = false;
    toRet.inPanicMode = false;
    return toRet;
}

/*
 * Throws an error for the specified token with the
 * given message
 */
static void unCompilerError(
    UNCompiler *compilerPtr,
    UNToken *tokenPtr,
    const char *msg
){
    /* ignore errors if already in panic mode */
    if(compilerPtr->inPanicMode){
        return;
    }
    compilerPtr->inPanicMode = true;

    #define bufferSize 50
    static char buffer[bufferSize] = {0};
    snprintf(
        buffer,
        bufferSize - 1,
        "%lu: Error",
        tokenPtr->lineNumber
    );
    pgWarning(buffer);

    if(tokenPtr->type == un_tokenEOF){
        pgWarning("(at eof)");
    }
    else if(tokenPtr->type == un_tokenError){
        /* nothing */
    }
    else{
        memset(buffer, 0, bufferSize);
        snprintf(
            buffer,
            bufferSize - 1,
            "(at '%.*s')",
            tokenPtr->length,
            tokenPtr->startPtr
        );
        pgWarning(buffer);
    }

    pgWarning(msg);

    compilerPtr->hadError = true;

    #undef bufferSize
}

/* Throws an error for the current token */
#define unCompilerErrorCurrent(COMPILERPTR, MSG) \
    unCompilerError( \
        COMPILERPTR, \
        &((COMPILERPTR)->currentToken), \
        MSG \
    )

/* Throws an error for the previous token */
#define unCompilerErrorPrev(COMPILERPTR, MSG) \
    unCompilerError( \
        COMPILERPTR, \
        &((COMPILERPTR)->prevToken), \
        MSG \
    )

/* Advances the specified compiler to the next token */
static void unCompilerAdvance(UNCompiler *compilerPtr){
    compilerPtr->prevToken = compilerPtr->currentToken;
    while(true){
        /* get next token from lexer */
        compilerPtr->currentToken = unLexerNext(
            &(compilerPtr->lexer)
        );
        /* bail out if bad token*/
        if(compilerPtr->currentToken.type
            = un_tokenError
        ){
            break;
        }

        unCompilerErrorCurrent(compilerPtr, "idk");
    }
}

/*
 * Advances the specified compiler to the next token
 * and makes sure it matches the given type
 */
static void unCompilerMatch(
    UNCompiler *compilerPtr,
    UNTokenType type,
    const char *errorMsgIfNoMatch
){
    if(compilerPtr->currentToken.type == type){
        unCompilerAdvance(compilerPtr);
        return;
    }

    unCompilerErrorCurrent(
        compilerPtr,
        errorMsgIfNoMatch
    );
}

/*
 * Gets the current program from the specified compiler
 * //todo: update later
 */
static UNProgram *unCompilerGetCurrentProgram(
    UNCompiler *compilerPtr
){
    return &(compilerPtr->compiledProgram);
}

/*
 * Writes the specified byte to the current program
 * of the specified compiler
 */
#define unCompilerWriteByte(COMPILERPTR, BYTE) \
    unProgramPushBackCode( \
        unCompilerGetCurrentProgram(COMPILERPTR), \
        BYTE, \
        (COMPILERPTR)->prevToken.lineNumber \
    )

/*
 * Writes the specified two bytes to the current
 * program of the specified compiler
 */
#define unCompilerWriteBytes( \
    COMPILERPTR, \
    BYTE1, \
    BYTE2 \
) \
    do{ \
        unCompilerWriteByte(COMPILERPTR, BYTE1); \
        unCompilerWriteByte(COMPILERPTR, BYTE2); \
    } while(false)

/*
 * Adds a literal to the current program of the
 * specified compiler and returns its index if
 * successful
 */
static uint8_t unCompilerMakeLiteral(
    UNCompiler *compilerPtr,
    UNValue value
){
    size_t litIndex = unLiteralsPushBack(
        &(compilerPtr->compiledProgram.literals),
        value
    );
    /*
     * error if the lit index doesn't fit in a single
     * byte since it needs to fit in the instruction
     */
    if(value > UINT8_MAX){
        unCompilerErrorPrev(
            compilerPtr,
            "too many constants to fit index in byte"
        );
        return 0;
    }

    return (uint8_t)litIndex;
}

/*
 * Writes a literal to the current program of the
 * specified compiler
 */
#define unCompilerWriteLiteral(COMPILERPTR, VALUE) \
    unCompilerWriteBytes( \
        COMPILERPTR, \
        un_literal, \
        unLiteralsPushBack( \
            &((COMPILERPTR)->compiledProgram \
                .literals), \
            VALUE \
        ) \
    )

/* forward decl */
static void unCompilerExpression(UNCompiler*);

/*
 * Parses the next expression for the specified
 * compiler at or above the given precedence level
 */
static void unCompilerExpressionPrecedence(
    UNCompiler *compilerPtr,
    UNPrecedence precedence
){
    //todo expr precedence
}

/* Parses the next number for the specified compiler */
static void unCompilerNumber(
    UNCompiler *compilerPtr
){
    UNValue value = strtod(
        compilerPtr->prevToken.startPtr,
        NULL
    );
    unCompilerWriteLiteral(compilerPtr, value);
}
 
/*
 * Parses the next unary operator for the specified
 * compiler
 */
static void unCompilerUnary(UNCompiler *compilerPtr){
    /*
     * save the type (since we have to read the rest
     * of the expression)
     */
    UNTokenType operatorType
        = compilerPtr->prevToken.type;
    
    /* compile expression that comes after the unary */
    unCompilerExpressionPrecedence(
        compilerPtr,
        un_precUnary
    );

    switch(operatorType){
        case un_tokenMinus:
            unCompilerWriteByte(
                compilerPtr,
                un_negate
            );
            break;
        //todo: unary bang
    }
}

/*
 * Parses the next infix binary operator for the
 * specified compiler
 */
static void unCompilerBinary(UNCompiler *compilerPtr){
    /*
     * save the type (since we have to read the rest
     * of the expression)
     */
    UNTokenType operatorType
        = compilerPtr->prevToken.type;
    
    UNParseRule *rulePtr = getRule(operatorType);
    unCompilerExpressionPrecedence(
        compilerPtr,
        rulePtr->precedence + 1
    );
    switch(operatorType){
        case un_tokenPlus:
            unCompilerWriteByte(compilerPtr, un_add);
            break;
        case un_tokenMinus:
            unCompilerWriteByte(
                compilerPtr,
                un_subtract
            );
            break;
        case un_tokenStar:
            unCompilerWriteByte(
                compilerPtr,
                un_multiply
            );
            break;
        case un_tokenSlash:
            unCompilerWriteByte(
                compilerPtr,
                un_divide
            );
            break;
    }
}

/*
 * Parses the next expression for the specified
 * compiler
 */
static void unCompilerExpression(
    UNCompiler *compilerPtr
){
    unCompilerExpressionPrecedence(
        compilerPtr,
        un_precAssign
    );
}

/*
 * Parses an expression within a set of parenthesis
 * for the specified compiler - should be called after
 * the first parenthesis has been matched
 */
static void unCompilerGrouping(
    UNCompiler *compilerPtr
){
    unCompilerExpression(compilerPtr);
    unCompilerMatch(
        compilerPtr,
        un_tokenRightParen,
        "expect ')' after expression"
    );
}

/*
 * Performs actions at the end of the compilation
 * process for the specified compiler
 */
static void unCompilerEnd(UNCompiler *compilerPtr){
    //todo: temp emit return
    unCompilerWriteByte(compilerPtr, un_tokenReturn);
}

/*
 * Frees the memory associated with the specified
 * UNCompiler but not the generated program
 */
static void unCompilerFree(UNCompiler *compilerPtr){
    unLexerFree(&(compilerPtr->lexer));
    /* do not free generated program */
}

/*
 * compiles the specified Unknown source file and
 * returns the program; error on compiler error
 */
UNProgram unCompile(const char *fileName){
    UNCompiler compiler = unCompilerMake(fileName);

    unCompilerAdvance(&compiler);
    unCompilerExpression(&compiler);
    unCompilerMatch(
        un_tokenEOF,
        "expect end of expression"
    );
    unCompilerEnd(&compiler);

    bool hadError = compiler.hadError;

    /* doesn't free the program */
    unCompilerFree(&compiler);
    
    if(hadError){
        pgError(
            "halting due to Unknown compiler error(s)"
        );
    }

    return compiler.compiledProgram;
}