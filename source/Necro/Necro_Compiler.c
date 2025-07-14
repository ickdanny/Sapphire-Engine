#include "Necro_Compiler.h"

#include <stdio.h>

#define maxParams 255

/* define for verbose compiler output for debugging */
/* #define COMPILER_VERBOSE */

/*
 * Represents the precedence hiearchy of the grammar 
 * of Necro from lowest to highest
 */
typedef enum NecroPrecedence{
    necro_precNone,
    necro_precAssign,
    necro_precOr,
    necro_precAnd,
    necro_precEquality,
    necro_precCompare,
    necro_precTerm,
    necro_precFactor,
    necro_precUnary,
    necro_precCall,
    necro_precPrimary,
} NecroPrecedence;

/* A function which is associated with a parse rule */
typedef void (*ParseFunc)(NecroCompiler*, bool);

/*
 * Encapsulates the behavior of the compiler upon
 * encountering a token of a certain type
 */
typedef struct NecroParseRule{
    ParseFunc prefixFunc;
    ParseFunc infixFunc;
    NecroPrecedence precedence;
} NecroParseRule;

/* table for all parse rules */
#define number necroCompilerNumber
#define unary necroCompilerUnary
#define binary necroCompilerBinary
#define grouping necroCompilerGrouping
#define call necroCompilerCall
#define dot necroCompilerDot
#define variable necroCompilerVariable
#define string necroCompilerString
#define and_ necroCompilerAnd
#define or_ necroCompilerOr
#define bool_ necroCompilerBool
#define vector necroCompilerVector
#define point necroCompilerPoint
#define lambda necroCompilerLambda
static const NecroParseRule parseRules[] = {
    [necro_tokenLeftParen]
        = {grouping, call,   necro_precCall},
    [necro_tokenRightParen]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenLeftBrace]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenRightBrace]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenComma]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenDot]
        = {NULL,     dot,    necro_precCall},
    [necro_tokenMinus]
        = {unary,    binary, necro_precTerm},
    [necro_tokenPercent]
        = {NULL,     binary, necro_precFactor},
    [necro_tokenPlus]
        = {NULL,     binary, necro_precTerm},
    [necro_tokenSemicolon]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenSlash]
        = {NULL,     binary, necro_precFactor},
    [necro_tokenBackSlash]
        = {lambda,   NULL,   necro_precNone},
    [necro_tokenStar]
        = {NULL,     binary, necro_precFactor},
    [necro_tokenBang]
        = {unary,    NULL,   necro_precNone},
    [necro_tokenBangEqual]
        = {NULL,     binary, necro_precEquality},
    [necro_tokenDoubleEqual]
        = {NULL,     binary, necro_precEquality},
    [necro_tokenGreater]
        = {NULL,     binary, necro_precCompare},
    [necro_tokenDoubleGreater]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenGreaterEqual]
        = {NULL,     binary, necro_precCompare},
    [necro_tokenLess]
        = {NULL,     binary, necro_precCompare},
    [necro_tokenDoubleLess]
        = {vector,   NULL,   necro_precNone},
    [necro_tokenLessEqual]
        = {NULL,     binary, necro_precCompare},
    [necro_tokenDoubleLeftBracket]
        = {point,     NULL,  necro_precNone},
    [necro_tokenDoubleRightBracket]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenDoubleAmpersand]
        = {NULL,     and_,   necro_precAnd},
    [necro_tokenDoubleVerticalBar]
        = {NULL,     or_,    necro_precOr},
    [necro_tokenColonEqual]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenPlusEqual]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenMinusEqual]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenStarEqual]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenSlashEqual]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenPercentEqual]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenMinusGreater]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenIdentifier]
        = {variable, NULL,   necro_precNone},
    [necro_tokenInt]
        = {number,   NULL,   necro_precNone},
    [necro_tokenFloat]
        = {number,   NULL,   necro_precNone},
    [necro_tokenString]
        = {string,   NULL,   necro_precNone},
    [necro_tokenElse]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenFalse]
        = {bool_,    NULL,   necro_precNone},
    [necro_tokenFor]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenIf]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenInclude]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenLet]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenPrint]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenReturn]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenTrue]
        = {bool_,    NULL,   necro_precNone},
    [necro_tokenVar]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenWait]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenWhile]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenYield]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenError]
        = {NULL,     NULL,   necro_precNone},
    [necro_tokenEOF]
        = {NULL,     NULL,   necro_precNone},
};
#undef number
#undef unary
#undef binary
#undef grouping
#undef call
#undef dot
#undef variable
#undef string
#undef and_
#undef or_
#undef bool_
#undef vector
#undef point
#undef lambda

/*
 * Returns a pointer to the parse rule for the
 * specified token type
 */
static const NecroParseRule *getRule(
    NecroTokenType type
){
    return &(parseRules[type]);
}

/*
 * Reserves the first stack slot for the VM's internal
 * use; error if called when locals already exist
 */
static void _necroFuncCompilerReserveFirstStackSlot(
    _NecroFuncCompiler *funcCompilerPtr
){
    assertTrue(
        funcCompilerPtr->localCount == 0,
        "error: must be called when local count is 0; "
        SRC_LOCATION
    );
    NecroLocal *reservedPtr= &(funcCompilerPtr->locals[
        funcCompilerPtr->localCount++
    ]);
    reservedPtr->depth = 0;
    reservedPtr->name.startPtr = "";
    reservedPtr->name.length = 0;
}

/*
 * Initializes the func compiler passed by pointer
 * (it is a big struct)
 */
static void _necroFuncCompilerInit(
    _NecroFuncCompiler *toInitPtr,
    NecroFuncType funcType,
    NecroCompiler *compilerPtr
){
    toInitPtr->enclosingPtr
        = compilerPtr->currentFuncCompilerPtr;
    compilerPtr->currentFuncCompilerPtr
        = toInitPtr;
    toInitPtr->funcPtr = NULL;
    toInitPtr->funcType = funcType;
    memset(
        &(toInitPtr->locals),
        0,
        sizeof(toInitPtr->locals)
    );
    toInitPtr->localCount = 0;
    toInitPtr->scopeDepth = 0;
    if(toInitPtr->enclosingPtr){
        toInitPtr->funcPtr = necroObjectFuncMake(
            toInitPtr->enclosingPtr->funcPtr
        );
    }
    else{
        toInitPtr->funcPtr = necroObjectFuncMake(NULL);
    }
    /* single string map for all functions in a file */
    if(funcType != necro_scriptFuncType){
        toInitPtr->funcPtr->namePtr
            = necroObjectStringCopy(
                compilerPtr->prevToken.startPtr,
                compilerPtr->prevToken.length,
                NULL,
                toInitPtr->funcPtr->program.literals
                    .stringMapPtr
            );
    }
}

/*
 * Constructs and returns a new NecroCompiler for the
 * specified file by value
 */
NecroCompiler necroCompilerMake(){
    NecroCompiler toRet = {0};
    toRet.hadError = false;
    toRet.inPanicMode = false;
    return toRet;
}

/*
 * Throws an error for the specified token with the
 * given message
 */
static void necroCompilerError(
    NecroCompiler *compilerPtr,
    NecroToken *tokenPtr,
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
        (unsigned long)tokenPtr->lineNumber
    );
    pgWarning(buffer);

    if(tokenPtr->type == necro_tokenEOF){
        pgWarning("(at eof)");
    }
    else if(tokenPtr->type == necro_tokenError){
        /* nothing */
    }
    else{
        memset(buffer, 0, bufferSize);
        snprintf(
            buffer,
            bufferSize - 1,
            "(at '%.*s')",
            (int)tokenPtr->length,
            tokenPtr->startPtr
        );
        pgWarning(buffer);
    }

    pgWarning(msg);

    compilerPtr->hadError = true;

    #undef bufferSize
}

/* Throws an error for the current token */
#define necroCompilerErrorCurrent(COMPILERPTR, MSG) \
    necroCompilerError( \
        COMPILERPTR, \
        &((COMPILERPTR)->currentToken), \
        MSG \
    )

/* Throws an error for the previous token */
#define necroCompilerErrorPrev(COMPILERPTR, MSG) \
    necroCompilerError( \
        COMPILERPTR, \
        &((COMPILERPTR)->prevToken), \
        MSG \
    )

/* Advances the specified compiler to the next token */
static void necroCompilerAdvance(
    NecroCompiler *compilerPtr
){
    compilerPtr->prevToken = compilerPtr->currentToken;
    /* loop to report error tokens */
    while(true){
        /* get next token from lexer */
        compilerPtr->currentToken = necroLexerNext(
            &(compilerPtr->lexer)
        );
        /* bail out if good token*/
        if(compilerPtr->currentToken.type
            != necro_tokenError
        ){
            break;
        }

        /* report error token */
        necroCompilerErrorCurrent(
            compilerPtr,
            compilerPtr->currentToken.startPtr
        );
    }
}

/*
 * Returns true if the current token in the specified
 * compiler matches the given type, false otherwise
 */
static bool necroCompilerCheckType(
    NecroCompiler *compilerPtr,
    NecroTokenType type
){
    return compilerPtr->currentToken.type == type;
}

/*
 * Advances the specified compiler to the next token
 * if matches the given type, returns true if
 * match found, false otherwise
 */
static bool necroCompilerMatch(
    NecroCompiler *compilerPtr,
    NecroTokenType type
){
    if(necroCompilerCheckType(compilerPtr, type)){
        necroCompilerAdvance(compilerPtr);
        return true;
    }
    return false;
}

/*
 * Advances the specified compiler to the next token
 * if it matches the given type, error if it doesn't
 */
#define necroCompilerConsume( \
    COMPILERPTR, \
    TYPE, \
    ERRORMSG \
) \
    do{ \
        if(!necroCompilerMatch( \
            (COMPILERPTR), \
            (TYPE) \
        )){ \
            necroCompilerErrorPrev( \
                COMPILERPTR, \
                ERRORMSG \
            ); \
        } \
    } while(false);

/*
 * Gets the current program from the specified compiler
 */
static NecroProgram *necroCompilerGetCurrentProgram(
    NecroCompiler *compilerPtr
){
    return &(compilerPtr->currentFuncCompilerPtr
        ->funcPtr->program);
}

/*
 * Writes the specified byte to the current program
 * of the specified compiler
 */
#define necroCompilerWriteByte(COMPILERPTR, BYTE) \
    necroProgramPushBackCode( \
        necroCompilerGetCurrentProgram(COMPILERPTR), \
        BYTE, \
        (COMPILERPTR)->prevToken.lineNumber \
    )

/*
 * Writes the specified two bytes to the current
 * program of the specified compiler
 */
#define necroCompilerWriteBytes( \
    COMPILERPTR, \
    BYTE1, \
    BYTE2 \
) \
    do{ \
        necroCompilerWriteByte(COMPILERPTR, BYTE1); \
        necroCompilerWriteByte(COMPILERPTR, BYTE2); \
    } while(false)

/* Used to have the compiler enter a new local scope */
#define necroCompilerBeginScope(COMPILERPTR) \
    (++((COMPILERPTR)->currentFuncCompilerPtr \
        ->scopeDepth))

/* Used to have the compiler exit a local scope */
static void necroCompilerEndScope(
    NecroCompiler *compilerPtr
){
    _NecroFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;
    --(funcCompilerPtr->scopeDepth);

    /*
     * for every local in the scope, pop it off
     * the stack
     */
    while(funcCompilerPtr->localCount > 0
        && funcCompilerPtr->locals[
            funcCompilerPtr->localCount - 1
        ].depth > funcCompilerPtr->scopeDepth
    ){
        necroCompilerWriteByte(compilerPtr, necro_pop);
        --(funcCompilerPtr->localCount);
    }
}

/*
 * Performs actions at the end of the compilation
 * process for the specified compiler and returns the
 * pointer to the NecroObjectFunc holding the compiled
 * program (or function)
 */
static NecroObjectFunc *necroCompilerEnd(
    NecroCompiler *compilerPtr
){
    /* emit return for functions but not for scripts */
    if(compilerPtr->currentFuncCompilerPtr->funcType
        != necro_scriptFuncType
    ){
        necroCompilerWriteByte(
            compilerPtr,
            necro_return
        );
    }
    else{
        /* for scripts, emit an end instruction */
        necroCompilerWriteByte(compilerPtr, necro_end);
    }
    NecroObjectFunc *toRet 
        = compilerPtr->currentFuncCompilerPtr->funcPtr;
    
    #ifdef COMPILER_VERBOSE
    if(!(compilerPtr->hadError)){
        if(toRet->namePtr != NULL){
            necroObjectPrint(
                necroObjectValue(toRet->namePtr)
            );
            printf(":\n");
        }
        else{
            printf("unnamed:\n");
        }
        necroProgramDisassemble(
            &(toRet->program)
        );
    }
    #endif

    /* pop off the current func compiler */
    compilerPtr->currentFuncCompilerPtr
        = compilerPtr->currentFuncCompilerPtr
            ->enclosingPtr;
    
    return toRet;
}

/*
 * After a compiler error, the compiler will try to
 * move to the next sync point i.e. the next statement
 * boundary
 */
static void necroCompilerSynchronize(
    NecroCompiler *compilerPtr
){
    compilerPtr->inPanicMode = false;
    while(compilerPtr->currentToken.type
        != necro_tokenEOF
    ){
        /* if passed a semicolon, reached sync point */
        if(compilerPtr->prevToken.type
            == necro_tokenSemicolon
        ){
            return;
        }
        switch(compilerPtr->currentToken.type){
            case necro_tokenInclude:
            case necro_tokenLet:
            case necro_tokenVar:
            case necro_tokenFor:
            case necro_tokenIf:
            case necro_tokenWhile:
            case necro_tokenYield:
            case necro_tokenWait:
            case necro_tokenPrint:
            case necro_tokenReturn:
                return;
            default:
                /* do nothing */
                break;
        }

        necroCompilerAdvance(compilerPtr);
    }
}

/*
 * Adds a literal to the current program of the
 * specified compiler and returns its index if
 * successful
 */
static uint8_t necroCompilerMakeLiteral(
    NecroCompiler *compilerPtr,
    NecroValue value
){
    size_t litIndex = necroLiteralsPushBack(
        &(necroCompilerGetCurrentProgram(compilerPtr)
            ->literals),
        value
    );
    /*
     * error if the lit index doesn't fit in a single
     * byte since it needs to fit in the instruction
     */
    if(litIndex > UINT8_MAX){
        necroCompilerErrorPrev(
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
#define necroCompilerWriteLiteral(COMPILERPTR, VALUE) \
    necroCompilerWriteBytes( \
        COMPILERPTR, \
        necro_literal, \
        necroLiteralsPushBack( \
            &(necroCompilerGetCurrentProgram( \
                COMPILERPTR \
            )->literals), \
            VALUE \
        ) \
    )

/*
 * Parses the next expression for the specified
 * compiler at or above the given precedence level
 */
static void necroCompilerExpressionPrecedence(
    NecroCompiler *compilerPtr,
    NecroPrecedence precedence
){
    /* read next token, check for prefix rule */
    necroCompilerAdvance(compilerPtr);
    ParseFunc prefixFunc = getRule(
        compilerPtr->prevToken.type
    )->prefixFunc;

    /* no prefix rule indicates syntax error */
    if(!prefixFunc){
        necroCompilerErrorPrev(
            compilerPtr,
            "Expect expression"
        );
        return;
    }
    bool canAssign = precedence <= necro_precAssign;
    prefixFunc(compilerPtr, canAssign);

    /*
     * next token might indicate that the prefix 
     * expression is an operand of an infix one; make
     * sure that its precedence is high enough
     */
    while(precedence <= getRule(
        compilerPtr->currentToken.type)->precedence
    ){
        necroCompilerAdvance(compilerPtr);
        ParseFunc infixFunc = getRule(
            compilerPtr->prevToken.type
        )->infixFunc;
        infixFunc(compilerPtr, canAssign);
    }

    /* error if trailing := */
    if(canAssign && necroCompilerMatch(
        compilerPtr,
        necro_tokenColonEqual
    )){
        necroCompilerErrorPrev(
            compilerPtr,
            "Invalid assignment target"
        );
    }
}

/* Parses the next number for the specified compiler */
void necroCompilerNumber(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    NecroValue value = {0};
    switch(compilerPtr->prevToken.type){
        case necro_tokenInt:
            value = necroIntValue(atoi(
                compilerPtr->prevToken.startPtr
            ));
            break;
        case necro_tokenFloat:
            value = necroFloatValue(
                strtof(
                    compilerPtr->prevToken.startPtr,
                    NULL
                )
            );
            break;
        default:
            pgError(
                "unexpected default; "
                SRC_LOCATION
            );
            break;
    }
    necroCompilerWriteLiteral(compilerPtr, value);
}
 
/*
 * Parses the next unary operator for the specified
 * compiler
 */
void necroCompilerUnary(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /*
     * save the type (since we have to read the rest
     * of the expression)
     */
    NecroTokenType operatorType
        = compilerPtr->prevToken.type;
    
    /* compile expression that comes after the unary */
    necroCompilerExpressionPrecedence(
        compilerPtr,
        necro_precUnary
    );

    switch(operatorType){
        case necro_tokenBang:
            necroCompilerWriteByte(
                compilerPtr,
                necro_not
            );
            break;
        case necro_tokenMinus:
            necroCompilerWriteByte(
                compilerPtr,
                necro_negate
            );
            break;
        default:
            /* do nothing */
            break;
    }
}

/*
 * Parses the next infix binary operator for the
 * specified compiler
 */
void necroCompilerBinary(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /*
     * save the type (since we have to read the rest
     * of the expression)
     */
    NecroTokenType operatorType
        = compilerPtr->prevToken.type;
    
    const NecroParseRule *rulePtr
        = getRule(operatorType);
    necroCompilerExpressionPrecedence(
        compilerPtr,
        rulePtr->precedence + 1
    );
    switch(operatorType){
        case necro_tokenPlus:
            necroCompilerWriteByte(
                compilerPtr,
                necro_add
            );
            break;
        case necro_tokenMinus:
            necroCompilerWriteByte(
                compilerPtr,
                necro_subtract
            );
            break;
        case necro_tokenStar:
            necroCompilerWriteByte(
                compilerPtr,
                necro_multiply
            );
            break;
        case necro_tokenSlash:
            necroCompilerWriteByte(
                compilerPtr,
                necro_divide
            );
            break;
        case necro_tokenPercent:
            necroCompilerWriteByte(
                compilerPtr,
                necro_modulo
            );
            break;
        case necro_tokenBangEqual:
            necroCompilerWriteBytes(
                compilerPtr,
                necro_equal,
                necro_not
            );
            break;
        case necro_tokenDoubleEqual:
            necroCompilerWriteByte(
                compilerPtr,
                necro_equal
            );
            break;
        case necro_tokenGreater:
            necroCompilerWriteByte(
                compilerPtr,
                necro_greater
            );
            break;
        case necro_tokenGreaterEqual:
            necroCompilerWriteBytes(
                compilerPtr,
                necro_less,
                necro_not
            );
            break;
        case necro_tokenLess:
            necroCompilerWriteByte(
                compilerPtr,
                necro_less
            );
            break;
        case necro_tokenLessEqual:
            necroCompilerWriteBytes(
                compilerPtr,
                necro_greater,
                necro_not
            );
            break;
        default:
            pgError(
                "unexpected default in binary; "
                SRC_LOCATION
            );
    }
}

/*
 * Parses the next expression for the specified
 * compiler
 */
void necroCompilerExpression(
    NecroCompiler *compilerPtr
){
    necroCompilerExpressionPrecedence(
        compilerPtr,
        necro_precAssign
    );
}

/*
 * Parses the next declaration for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void necroCompilerDeclaration(NecroCompiler *compilerPtr){
    if(necroCompilerMatch(
        compilerPtr,
        necro_tokenInclude
    )){
        necroCompilerIncludeDeclaration(compilerPtr);
    }
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenLet)
    ){
        necroCompilerVariableDeclaration(
            compilerPtr,
            false /* const variable */
        );
    }
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenVar)
    ){
        necroCompilerVariableDeclaration(
            compilerPtr,
            true /* mutable variable */
        );
    }
    else{
        necroCompilerStatement(compilerPtr);
    }

    /* if compile error, go to a sync point */
    if(compilerPtr->inPanicMode){
        necroCompilerSynchronize(compilerPtr);
    }
}

/*
 * Parses the next include declaration for the
 * specified compiler
 */
void necroCompilerIncludeDeclaration(
    NecroCompiler *compilerPtr
){
    //todo: need a new lexer for the include file,
    //also a guard for recursive includes
}

/*
 * Creates a new string literal for the specified
 * identifier and returns its literal index
 */
static uint8_t necroCompilerIdentifierLiteral(
    NecroCompiler *compilerPtr,
    NecroToken *tokenPtr
){
    return necroCompilerMakeLiteral(
        compilerPtr,
        necroObjectValue(
            necroObjectStringCopy(
                tokenPtr->startPtr,
                tokenPtr->length,
                NULL,
                necroCompilerGetCurrentProgram(
                    compilerPtr)->literals.stringMapPtr
            )
        )
    );
}

/*
 * Adds information about local variable having the
 * given token as its name for the specified compiler
 */
static void necroCompilerAddLocal(
    NecroCompiler *compilerPtr,
    NecroToken name
){
    _NecroFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;
    if(funcCompilerPtr->localCount >= _uint8_t_count){
        necroCompilerErrorPrev(
            compilerPtr,
            "Too many locals in func"
        );
        return;
    }
    NecroLocal *localPtr= &(funcCompilerPtr->locals[
        funcCompilerPtr->localCount++
    ]);
    localPtr->name = name;

    /* use -1 as sentinel value for uninitialized */
    localPtr->depth = -1;
}

/*
 * Returns true if the two given tokens represent the
 * same identifier, false otherwise
 */
static bool identifiersEqual(
    NecroToken *tokenPtr1,
    NecroToken *tokenPtr2
){
    if(tokenPtr1->length != tokenPtr2->length){
        return false;
    }
    return memcmp(
        tokenPtr1->startPtr,
        tokenPtr2->startPtr,
        tokenPtr1->length
    ) == 0;
}

/*
 * Registers information about a local variable for the
 * specified compiler
 */
static void necroCompilerDeclareVariable(
    NecroCompiler *compilerPtr
){
    _NecroFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;

    /* bail if in global scope */
    if(funcCompilerPtr->scopeDepth == 0){
        return;
    }

    NecroToken *namePtr = &(compilerPtr->prevToken);

    /*
     * error if two variables in same depth w/ same
     * name
     */
    for(int i = funcCompilerPtr->localCount - 1;
        i >= 0;
        --i
    ){
        NecroLocal *localPtr
            = &(funcCompilerPtr->locals[i]);

        /* break out if find local in higher depth */
        if(localPtr->depth != -1
            && localPtr->depth
                < funcCompilerPtr->scopeDepth
        ){
            break;
        }

        if(identifiersEqual(
            namePtr,
            &(localPtr->name)
        )){
            necroCompilerErrorPrev(
                compilerPtr,
                "Preexisting variable with this name "
                "in this scope"
            );
        }
    }

    necroCompilerAddLocal(compilerPtr, *namePtr);
}

/*
 * Parses the name of the next variable for the given
 * compiler and returns the index of the string
 * constant for the name
 */
static uint8_t necroCompilerParseVariable(
    NecroCompiler *compilerPtr,
    const char *errorMsg
){
    necroCompilerConsume(
        compilerPtr,
        necro_tokenIdentifier,
        errorMsg
    );

    necroCompilerDeclareVariable(compilerPtr);

    /*
     * if parser within a block, do not register a
     * string for its name (only globals looked up by
     * name at runtime)
     */
    if(compilerPtr->currentFuncCompilerPtr->scopeDepth
        > 0
    ){
        return 0;
    }

    return necroCompilerIdentifierLiteral(
        compilerPtr,
        &(compilerPtr->prevToken)
    );
}

/*
 * Marks the topmost local as finished initialized
 * for the specified compiler; does nothing if the
 * current variable being compiled is global
 */
static void necroCompilerMarkLocalInitialized(
    NecroCompiler *compilerPtr
){
    _NecroFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;
    /* special case for ignoring global functions */
    if(funcCompilerPtr->scopeDepth == 0){
        return;
    }
    funcCompilerPtr->locals[
        funcCompilerPtr->localCount - 1
    ].depth = funcCompilerPtr->scopeDepth;
}

/*
 * Emits the bytecode for a variable declaration
 * for the specified compiler
 */
static void necroCompilerDefineVariable(
    NecroCompiler *compilerPtr,
    uint8_t globalIndex
){
    /*
     * do nothing if local variable - already sits
     * on top of stack
     */
    if(compilerPtr->currentFuncCompilerPtr->scopeDepth
        > 0
    ){
        necroCompilerMarkLocalInitialized(compilerPtr);
        return;
    }

    /* write global variables */
    necroCompilerWriteBytes(
        compilerPtr,
        necro_defineGlobal,
        globalIndex
    );
}

/*
 * Parses the next variable declaration for the
 * specified compiler; uninitialized variables get the
 * default value of FALSE
 */
void necroCompilerVariableDeclaration(
    NecroCompiler *compilerPtr,
    bool mutable
){
    //todo: handle const and mutable variables
    uint8_t globalIndex = necroCompilerParseVariable(
        compilerPtr,
        "Expect variable name"
    );

    if(necroCompilerMatch(
        compilerPtr,
        necro_tokenColonEqual)
    ){
        necroCompilerExpression(compilerPtr);
    }
    else{
        /*
         * if uninitialized, variables get set to
         * the value FALSE
         */
        necroCompilerWriteByte(compilerPtr, necro_false);
    }

    /* eat the semicolon */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenSemicolon,
        "expect ';' after var declare; "
    );

    necroCompilerDefineVariable(
        compilerPtr,
        globalIndex
    );
}

/*
 * Parses the next statement for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void necroCompilerStatement(
    NecroCompiler *compilerPtr
){
    /* match the print statement */
    if(necroCompilerMatch(
        compilerPtr,
        necro_tokenPrint)
    ){
        necroCompilerPrintStatement(compilerPtr);
    }
    /* match an if statement */
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenIf)
    ){
        necroCompilerIfStatement(compilerPtr);
    }
    /* match a while loop */
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenWhile
    )){
        necroCompilerWhileStatement(compilerPtr);
    }
    /* match a for loop */
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenFor)
    ){
        necroCompilerForStatement(compilerPtr);
    }
    /* match a return */
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenReturn
    )){
        necroCompilerReturnStatement(compilerPtr);
    }
    /* match a yield */
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenYield
    )){
        necroCompilerYieldStatement(compilerPtr);
    }
    /* match a wait loop */
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenWait
    )){
        necroCompilerWaitStatement(compilerPtr);
    }
    /* match a block */
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenLeftBrace)
    ){
        necroCompilerBeginScope(compilerPtr);
        necroCompilerBlock(compilerPtr);
        necroCompilerEndScope(compilerPtr);
    }
    /* if none of the above, expression statement */
    else{
        necroCompilerExpressionStatement(compilerPtr);
    }
}

/*
 * Parses the next print statement for the specified
 * compiler
 */
void necroCompilerPrintStatement(
    NecroCompiler *compilerPtr
){
    /* compile the expression after "print" */
    necroCompilerExpression(compilerPtr);
    /* eat the semicolon */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenSemicolon,
        "expect ';' after print statement; "
    );
    /* print the result of the expression */
    necroCompilerWriteByte(compilerPtr, necro_print);
}

/*
 * Writes a jump instruction for the specified compiler
 * and returns the index in the code of the instruction
 */
static int necroCompilerWriteJump(
    NecroCompiler *compilerPtr,
    uint8_t instruction
){
    necroCompilerWriteByte(compilerPtr, instruction);

    /* emit 2 bytes of placeholder values */
    necroCompilerWriteByte(compilerPtr, 0xFF);
    necroCompilerWriteByte(compilerPtr, 0xFF);

    /* return index of instruction */
    return necroCompilerGetCurrentProgram(
        compilerPtr
    )->code.size - 2;
}

/*
 * Patches the jump distance into the jump instruction
 * at the given index for the specified compiler
 */
static void necroCompilerPatchJump(
    NecroCompiler *compilerPtr,
    int jumpInstructionIndex
){
    NecroProgram *currentProgramPtr
        = necroCompilerGetCurrentProgram(compilerPtr);
    /*
     * calculate how far the jump is; -2 to account
     * for the fact that the ditsance itself is going
     * to be encoded in the code
     */
    int jumpDist = currentProgramPtr->code.size
        - jumpInstructionIndex - 2;

    if(jumpDist > UINT16_MAX){
        necroCompilerErrorPrev(
            compilerPtr,
            "Too much code to jump over"
        );
    }

    /* encode the jump distance */
    arrayListSet(uint8_t,
        &(currentProgramPtr->code),
        jumpInstructionIndex,
        (jumpDist >> 8) & 0xFF
    );
    arrayListSet(uint8_t,
        &(currentProgramPtr->code),
        jumpInstructionIndex + 1,
        jumpDist & 0xFF
    );
}

/*
 * Parses the next if statement for the specified
 * compiler
 */
void necroCompilerIfStatement(
    NecroCompiler *compilerPtr
){
    necroCompilerConsume(
        compilerPtr,
        necro_tokenLeftParen,
        "Expect '(' after \"if\""
    );
    necroCompilerExpression(compilerPtr);
    necroCompilerConsume(
        compilerPtr,
        necro_tokenRightParen,
        "Expect ')' after if condition"
    );

    /*
     * save the index to write the address of the jump
     * instruction
     */
    int thenJumpInstructionIndex
        = necroCompilerWriteJump(
            compilerPtr,
            necro_jumpIfFalse
        );

    /* have the VM pop off the condition if true */
    necroCompilerWriteByte(compilerPtr, necro_pop);

    /* parse the then block */
    necroCompilerStatement(compilerPtr);

    /* write unconditional jump past else block */
    int elseJumpInstructionIndex
        = necroCompilerWriteJump(
            compilerPtr,
            necro_jump
        );

    /* write the address after compiling statement */
    necroCompilerPatchJump(
        compilerPtr,
        thenJumpInstructionIndex
    );

    /* have the VM pop off the condition if false */
    necroCompilerWriteByte(compilerPtr, necro_pop);

    /* check for else branch */
    if(necroCompilerMatch(
        compilerPtr,
        necro_tokenElse)
    ){
        necroCompilerStatement(compilerPtr);
    }

    /* patch the unconditional jump past the else */
    necroCompilerPatchJump(
        compilerPtr,
        elseJumpInstructionIndex
    );
}

/*
 * Writes a loop instruction for the specified
 * compiler (a loop instruction is just a backwards
 * jump)
 */
static void necroCompilerWriteLoop(
    NecroCompiler *compilerPtr,
    int loopStartIndex
){
    necroCompilerWriteByte(compilerPtr, necro_loop);

    /* +2 to account for size of operands */
    int offset
        = necroCompilerGetCurrentProgram(compilerPtr)
            ->code.size - loopStartIndex + 2;
    if(offset > UINT16_MAX){
        necroCompilerErrorPrev(
            compilerPtr,
            "Loop body too large"
        );
    }

    necroCompilerWriteByte(
        compilerPtr,
        (offset >> 8) & 0xFF
    );
    necroCompilerWriteByte(compilerPtr, offset & 0xFF);
}

/*
 * Parses the next while statement for the specified
 * compiler
 */
void necroCompilerWhileStatement(
    NecroCompiler *compilerPtr
){
    /* save index before the condition check */
    int loopStartIndex
        = necroCompilerGetCurrentProgram(compilerPtr)
            ->code.size;

    necroCompilerConsume(
        compilerPtr,
        necro_tokenLeftParen,
        "Expect '(' after \"while\""
    );
    necroCompilerExpression(compilerPtr);
    necroCompilerConsume(
        compilerPtr,
        necro_tokenRightParen,
        "Expect ')' after while condition"
    );

    int exitJumpInstructionIndex
        = necroCompilerWriteJump(
            compilerPtr,
            necro_jumpIfFalse
        );
    necroCompilerWriteByte(compilerPtr, necro_pop);
    necroCompilerStatement(compilerPtr);
    necroCompilerWriteLoop(
        compilerPtr,
        loopStartIndex
    );

    necroCompilerPatchJump(
        compilerPtr,
        exitJumpInstructionIndex
    );
    /* need to pop false off the stack */
    necroCompilerWriteByte(compilerPtr, necro_pop);
}

/*
 * Parses the next for statement for the specified
 * compiler
 */
void necroCompilerForStatement(
    NecroCompiler *compilerPtr
){
    /*
     * a for loop has its own scope in case the
     * initializer is a var declare
     */
    necroCompilerBeginScope(compilerPtr);

    necroCompilerConsume(
        compilerPtr,
        necro_tokenLeftParen,
        "Expect '(' after \"for\""
    );

    /* parse initializer */
    if(necroCompilerMatch(
        compilerPtr,
        necro_tokenSemicolon
    )){
        /* no initializer */
    }
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenLet
    )){
        /* const var declaration initializer */
        necroCompilerVariableDeclaration(
            compilerPtr,
            false
        );
    }
    else if(necroCompilerMatch(
        compilerPtr,
        necro_tokenVar
    )){
        /* mutable var declaration initializer */
        necroCompilerVariableDeclaration(
            compilerPtr,
            true
        );
    }
    else{
        /* expression statement initializer */
        necroCompilerExpressionStatement(compilerPtr);
    }

    /* save index before the condition check */
    int loopStartIndex
        = necroCompilerGetCurrentProgram(compilerPtr)
            ->code.size;

    /* parse condition */
    int exitJumpInstructionIndex = -1;
    if(!necroCompilerMatch(
        compilerPtr,
        necro_tokenSemicolon)
    ){
        /* expr not stmt since don't want to pop */
        necroCompilerExpression(compilerPtr);
        necroCompilerConsume(
            compilerPtr,
            necro_tokenSemicolon,
            "Expect ';'"
        );

        /* jump out if condition false */
        exitJumpInstructionIndex
            = necroCompilerWriteJump(
                compilerPtr,
                necro_jumpIfFalse
            );
        necroCompilerWriteByte(compilerPtr, necro_pop);
    }

    /* parse increment */
    if(!necroCompilerMatch(
        compilerPtr,
        necro_tokenRightParen)
    ){
        /*
         * jump over increment, run body, then jump
         * back to the increment
         */
        int bodyJumpInstructionIndex
            = necroCompilerWriteJump(
                compilerPtr,
                necro_jump
            );
        int incrementStartIndex
            = necroCompilerGetCurrentProgram(
                compilerPtr
            )->code.size;
        /* expr since no semicolon at end */
        necroCompilerExpression(compilerPtr);
        necroCompilerWriteByte(compilerPtr, necro_pop);

        necroCompilerConsume(
            compilerPtr,
            necro_tokenRightParen,
            "Expect ')' after for clauses"
        );

        /* jump to condition check */
        necroCompilerWriteLoop(
            compilerPtr,
            loopStartIndex
        );
        /* make it so the body jumps to increment */
        loopStartIndex = incrementStartIndex;
        /* jump past the increment at first */
        necroCompilerPatchJump(
            compilerPtr,
            bodyJumpInstructionIndex
        );
    }

    necroCompilerStatement(compilerPtr);
    /*
     * loops to either the condition or the increment
     * if it is present
     */
    necroCompilerWriteLoop(
        compilerPtr,
        loopStartIndex
    );

    /* patch the jump from the condition */
    if(exitJumpInstructionIndex != -1){
        necroCompilerPatchJump(
            compilerPtr,
            exitJumpInstructionIndex
        );
        necroCompilerWriteByte(compilerPtr, necro_pop);
    }

    necroCompilerEndScope(compilerPtr);
}

/*
 * Parses the next yield statement for the specified
 * compiler
 */
void necroCompilerYieldStatement(
    NecroCompiler *compilerPtr
){
    /* eat the semicolon */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenSemicolon,
        "expect ';' after yield statement"
    );
    /* write a yield instruction */
    necroCompilerWriteByte(compilerPtr, necro_yield);
}

/*
 * Parses the next wait statement for the specified
 * compiler
 */
void necroCompilerWaitStatement(
    NecroCompiler *compilerPtr
){
    /* save index before the condition check */
    int loopStartIndex
        = necroCompilerGetCurrentProgram(compilerPtr)
            ->code.size;

    necroCompilerExpression(compilerPtr);
    necroCompilerConsume(
        compilerPtr,
        necro_tokenSemicolon,
        "Expect ';' after wait statement"
    );

    /* jump to end if true */
    necroCompilerWriteByte(compilerPtr, necro_not);
    int exitJumpInstructionIndex
        = necroCompilerWriteJump(
            compilerPtr,
            necro_jumpIfFalse
        );
    necroCompilerWriteByte(compilerPtr, necro_pop);
    necroCompilerWriteByte(compilerPtr, necro_yield);
    necroCompilerWriteLoop(
        compilerPtr,
        loopStartIndex
    );

    necroCompilerPatchJump(
        compilerPtr,
        exitJumpInstructionIndex
    );
    /* need to pop false off the stack */
    necroCompilerWriteByte(compilerPtr, necro_pop);
}

/*
 * writes a sequence of instructions representing a
 * null return; it will return false
 */
#define necroCompilerWriteNullReturn(COMPILERPTR) \
    do{ \
        necroCompilerWriteByte(\
            compilerPtr, \
            necro_false \
        ); \
        necroCompilerWriteByte(\
            compilerPtr, \
            necro_return \
        ); \
    } while(false)

/*
 * Parses the next return statement for the specified
 * compiler
 */
void necroCompilerReturnStatement(
    NecroCompiler *compilerPtr
){
    /*
     * error if trying to return in the top level
     * script
     */
    if(compilerPtr->currentFuncCompilerPtr->funcType
        == necro_scriptFuncType
    ){
        necroCompilerErrorPrev(
            compilerPtr,
            "Cannot return from top level script"
        );
    }
    /*
     * if the next token is a semicolon, this is a
     * null return
     */
    if(necroCompilerMatch(
        compilerPtr,
        necro_tokenSemicolon
    )){
        necroCompilerWriteNullReturn(compilerPtr);
    }
    /* otherwise, parse the expression to return */
    else{
        necroCompilerExpression(compilerPtr);
        necroCompilerConsume(
            compilerPtr,
            necro_tokenSemicolon,
            "Expect ';' after return value"
        );
        necroCompilerWriteByte(
            compilerPtr,
            necro_return
        );
    }
}

/* Parses the next block for the specified compiler */
void necroCompilerBlock(NecroCompiler *compilerPtr){
    while(!necroCompilerCheckType(
            compilerPtr,
            necro_tokenRightBrace
        ) && !necroCompilerCheckType(
            compilerPtr,
            necro_tokenEOF
        )
    ){
        necroCompilerDeclaration(compilerPtr);
    }

    necroCompilerConsume(
        compilerPtr,
        necro_tokenRightBrace,
        "Expect '}' after block"
    );
}

/*
 * Parses the next expression statement for the
 * specified compiler
 */
void necroCompilerExpressionStatement(
    NecroCompiler *compilerPtr
){
    necroCompilerExpression(compilerPtr);
    /* eat the semicolon */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenSemicolon,
        "expect ';' after expr statement; "
    );
    /* discard the result of the expression */
    necroCompilerWriteByte(compilerPtr, necro_pop);
}

/*
 * Parses an expression within a set of parenthesis
 * for the specified compiler - should be called after
 * the first parenthesis has been matched
 */
void necroCompilerGrouping(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    necroCompilerExpression(compilerPtr);
    necroCompilerConsume(
        compilerPtr,
        necro_tokenRightParen,
        "expect ')' after expression; "
    );
}

/*
 * Parses the (actual) argument list of a function call
 * for the specified compiler and returns the number of
 * arguments passed
 */
static uint8_t necroCompilerArgumentList(
    NecroCompiler *compilerPtr
){
    uint8_t numArgs = 0;
    /*
     * compile each expression separated by commas
     * until the closing paren
     */
    if(!necroCompilerCheckType(
        compilerPtr,
        necro_tokenRightParen
    )){
        do{
            necroCompilerExpression(compilerPtr);
            if(numArgs == maxParams){
                necroCompilerErrorPrev(
                    compilerPtr,
                    "too many arguments"
                );
            }
            ++numArgs;
        } while(necroCompilerMatch(
            compilerPtr,
            necro_tokenComma
        ));
    }
    necroCompilerConsume(
        compilerPtr,
        necro_tokenRightParen,
        "Expect ')' after args"
    );
    return numArgs;
}

/* Parses a function call for the specified compiler */
void necroCompilerCall(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    uint8_t numArgs = necroCompilerArgumentList(
        compilerPtr
    );
    necroCompilerWriteBytes(
        compilerPtr,
        necro_call,
        numArgs
    );
}

/*
 * Returns the index of the local with the same name
 * as the given token, or -1 if no such local is found;
 * The index returned is the same as the index of the
 * local on the stack during runtime since the layout
 * of the local info is the same as the layout of the
 * stack
 */
static int necroCompilerResolveLocal(
    NecroCompiler *compilerPtr,
    NecroToken *name
){
    _NecroFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;
    for(int i = funcCompilerPtr->localCount - 1;
        i >= 0;
        -- i
    ){
        NecroLocal *localPtr
            = &(funcCompilerPtr->locals[i]);
        if(identifiersEqual(name, &(localPtr->name))){
            /*
             * handle case of uninitialized local
             * trying to read itself by checking for
             * the sentinel value -1
             */
            if(localPtr->depth == -1){
                necroCompilerErrorPrev(
                    compilerPtr,
                    "Cannot read local var in its own "
                    "initializer"
                );
            }
            //todo: why is the first slot the function?
            /*
             * add 1 since the first slot is always 
             * the function itself
             */
            return i + 1;
        }
    }
    return -1;
}

/*
 * Emits instructions for the specified global or local
 * variable passed as a token for the given compiler
 */
static void necroCompilerNamedVariable(
    NecroCompiler *compilerPtr,
    NecroToken varName,
    bool canAssign
){
    uint8_t getOp = 0;
    uint8_t setOp = 0;

    /*
     * eventually becomes the second byte following
     * the instruction code; the stack slot for locals
     * and the index of the name literal for globals
     */
    int arg = necroCompilerResolveLocal(
        compilerPtr,
        &varName
    );

    //todo: handle const variables

    bool isLocal = (arg != -1);

    /* if variable was resolved to a local */
    if(isLocal){
        getOp = necro_getLocal;
        setOp = necro_setLocal;
    }
    else{
        arg = necroCompilerIdentifierLiteral(
            compilerPtr,
            &varName
        );
        getOp = necro_getGlobal;
        setOp = necro_setGlobal;
    }

    /*
     * if following token is dot, either get/set on
     * vector or point
     */
    if(necroCompilerMatch(
        compilerPtr,
        necro_tokenDot
    )){
        /* expect current token to be an identifier */
        necroCompilerConsume(
            compilerPtr,
            necro_tokenIdentifier,
            "Expect 'r', 't', 'x', or 'y' following "
            "a '.'"
        )
        if(compilerPtr->prevToken.length != 1){
            necroCompilerErrorPrev(
                compilerPtr,
                "Expect only 'r', 't', 'x', or 'y'"
            );
        }
        /*
         * VN needs 2 instructions to get the value
         * of a member; one to load the whole composite
         * type, another to get specifically the member
         */
        NecroInstruction memberGetOp = 0;
        switch(*(compilerPtr->prevToken.startPtr)){
            case 'r':
                memberGetOp = necro_getR;
                setOp = isLocal 
                    ? necro_setRLocal
                    : necro_setRGlobal;
                break;
            case 't':
                memberGetOp = necro_getTheta;
                setOp = isLocal
                    ? necro_setThetaLocal
                    : necro_setThetaGlobal;
                break;
            case 'x':
                memberGetOp = necro_getX;
                setOp = isLocal 
                    ? necro_setXLocal
                    : necro_setXGlobal;
                break;
            case 'y':
                memberGetOp = necro_getY;
                setOp = isLocal 
                    ? necro_setYLocal
                    : necro_setYGlobal;
                break;
            default:
                necroCompilerErrorPrev(
                    compilerPtr,
                    "Expect only 'r', 't', 'x', or 'y'"
                );
                break;
        }
        /*
         * if following token is :=, member
         * assignment
         */
        if(canAssign && necroCompilerMatch(
            compilerPtr,
            necro_tokenColonEqual
        )){
            necroCompilerExpression(compilerPtr);
            necroCompilerWriteBytes(
                compilerPtr,
                setOp,
                (uint8_t)arg
            );
        }
        /* otherwise member get */
        else{
            necroCompilerWriteBytes(
                compilerPtr,
                getOp,
                (uint8_t)arg
            );
            necroCompilerWriteByte(
                compilerPtr,
                memberGetOp
            );
        }
    }
    /* if following token is :=, assignment */
    else if(canAssign && necroCompilerMatch(
        compilerPtr,
        necro_tokenColonEqual
    )){
        necroCompilerExpression(compilerPtr);
        necroCompilerWriteBytes(
            compilerPtr,
            setOp,
            (uint8_t)arg
        );
    }
    /* otherwise just a get */
    else{
        necroCompilerWriteBytes(
            compilerPtr,
            getOp,
            (uint8_t)arg
        );
    }
}

/*
 * Parses a variable for the specified compiler
 */
void necroCompilerVariable(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    necroCompilerNamedVariable(
        compilerPtr,
        compilerPtr->prevToken,
        canAssign
    );
}

/*
 * Helps the specified compiler parse the header
 * of a function (consuming from the '(' of the
 * parameter list to the '{' prior to the body)
 */
static void necroCompilerFunctionHeader(
    NecroCompiler *compilerPtr
){
    /* will be after lambda symbol */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenLeftParen,
        "Expect '(' after func name"
    );
    /* get parameters */
    if(!necroCompilerCheckType(
        compilerPtr,
        necro_tokenRightParen
    )){
        do{
            ++(compilerPtr->currentFuncCompilerPtr
                ->funcPtr->arity);
            if(compilerPtr->currentFuncCompilerPtr
                ->funcPtr->arity > maxParams
            ){
                necroCompilerErrorCurrent(
                    compilerPtr,
                    "Parameters exceeding max"
                );
            }
            uint8_t paramNameIndex
                = necroCompilerParseVariable(
                    compilerPtr,
                    "Expect parameter name"
                );
            //todo: are params globals?
            necroCompilerDefineVariable(
                compilerPtr,
                paramNameIndex
            );
        } while(necroCompilerMatch(
            compilerPtr,
            necro_tokenComma
        ));
    }
    necroCompilerConsume(
        compilerPtr,
        necro_tokenRightParen,
        "Expect ')' after parameters"
    );
    necroCompilerConsume(
        compilerPtr,
        necro_tokenMinusGreater,
        "Expect '->' before function body"
    );
    necroCompilerConsume(
        compilerPtr,
        necro_tokenLeftBrace,
        "Expect '{' before function body"
    );
}

/*
 * Parses a lambda for the specified compiler
 */
void necroCompilerLambda(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /* create new func compiler for the new function */
    _NecroFuncCompiler newFuncCompiler = {0};
    _necroFuncCompilerInit(
        &newFuncCompiler,
        necro_functionFuncType,
        compilerPtr
    );
    /* no corresponding end scope; unneeded */
    necroCompilerBeginScope(compilerPtr);

    /* consume syntax of function declaration */
    necroCompilerFunctionHeader(compilerPtr);

    /* compile function body */
    necroCompilerBlock(compilerPtr);

    /*
     * retrieve code for the function and write to the
     * enclosing code as a literal
     */
    NecroObjectFunc *funcPtr = necroCompilerEnd(
        compilerPtr
    );
    necroCompilerWriteBytes(
        compilerPtr,
        necro_literal,
        necroCompilerMakeLiteral(
            compilerPtr,
            necroObjectValue(funcPtr)
        )
    );
}

/*
 * Parses a string for the specified compiler
 */
void necroCompilerString(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /*
     * The object is not referenced from the VM's
     * object list as it is always accessible from
     * the literals
     */
    necroCompilerWriteLiteral(
        compilerPtr,
        necroObjectValue(
            necroObjectStringCopy(
                compilerPtr->prevToken.startPtr + 1,
                compilerPtr->prevToken.length - 2,
                NULL,
                necroCompilerGetCurrentProgram(
                    compilerPtr
                )->literals.stringMapPtr
            )
        )
    );
}

/*
 * Parses a boolean AND for the specified compiler
 */
void necroCompilerAnd(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /*
     * if stack top is false, the left hand side of
     * the AND is false thus we short circuit and
     * skip computing the right hand side; otherwise,
     * discard the left hand side and evalute the
     * right hand side as the entire expression
     */
    int endJumpInstructionIndex
        = necroCompilerWriteJump(
            compilerPtr,
            necro_jumpIfFalse
        );
    necroCompilerWriteByte(compilerPtr, necro_pop);
    necroCompilerExpressionPrecedence(
        compilerPtr,
        necro_precAnd
    );
    necroCompilerPatchJump(
        compilerPtr,
        endJumpInstructionIndex
    );
}

/*
 * Parses a boolean OR for the specified compiler
 */
void necroCompilerOr(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /*
     * if left hand is false, jump over the next jump
     * to the end which would short circuit
     */
    int elseJumpInstructionIndex
        = necroCompilerWriteJump(
            compilerPtr,
            necro_jumpIfFalse
        );
    int endJumpInstructionIndex
        = necroCompilerWriteJump(
            compilerPtr,
            necro_jump
        );
    necroCompilerPatchJump(
        compilerPtr,
        elseJumpInstructionIndex
    );
    necroCompilerWriteByte(compilerPtr, necro_pop);
    necroCompilerExpressionPrecedence(
        compilerPtr,
        necro_precAnd
    );
    necroCompilerPatchJump(
        compilerPtr,
        endJumpInstructionIndex
    );
}

/*
 * Parses a bool for the specified compiler
 */
void necroCompilerBool(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    switch(compilerPtr->prevToken.type){
        case necro_tokenFalse:
            necroCompilerWriteByte(
                compilerPtr,
                necro_false
            );
            break;
        case necro_tokenTrue:
            necroCompilerWriteByte(
                compilerPtr,
                necro_true
            );
            break;
        default:
            pgError(
                "expect default to be unreachable; "
                SRC_LOCATION
            );
            return;
    }
}

/* Parses a vector for the specified compiler */
void necroCompilerVector(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /* parse the R expression */
    necroCompilerExpression(compilerPtr);

    /* eat the comma */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenComma,
        "Expect ',' after R initializer"
    );

    /* parse the theta expression */
    necroCompilerExpression(compilerPtr);

    /* eat the closing '>>' */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenDoubleGreater,
        "Expect \">>\" after theta initializer"
    );

    /* emit the instruction for creating a vector */
    necroCompilerWriteByte(
        compilerPtr,
        necro_makeVector
    );
}

/* Parses a point for the specified compiler */
void necroCompilerPoint(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /* parse the X expression */
    necroCompilerExpression(compilerPtr);

    /* eat the comma */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenComma,
        "Expect ',' after X initializer"
    );

    /* parse the Y expression */
    necroCompilerExpression(compilerPtr);

    /* eat the closing ]] */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenDoubleRightBracket,
        "Expect ']]' after Y initializer"
    );

    /* emit the instruction for creating a vector */
    necroCompilerWriteByte(
        compilerPtr,
        necro_makePoint
    );
}

/*
 * Parses a dot for the specified compiler; only
 * handles get operations
 */
void necroCompilerDot(
    NecroCompiler *compilerPtr,
    bool canAssign
){
    /*
     * should not be reached when the operand is a
     * variable name; special case for variables to
     * handle that
     */
    /* expect current token to be an identifier */
    necroCompilerConsume(
        compilerPtr,
        necro_tokenIdentifier,
        "Expect 'r', 't', 'x', or 'y' following a '.'"
    )
    if(compilerPtr->prevToken.length != 1){
        necroCompilerErrorPrev(
            compilerPtr,
            "Expect only 'r', 't', 'x', or 'y'"
        );
    }
    /* only get for non-variable names */
    NecroInstruction instruction = 0;
    switch(*(compilerPtr->prevToken.startPtr)){
        case 'r':
            instruction = necro_getR;
            break;
        case 't':
            instruction = necro_getTheta;
            break;
        case 'x':
            instruction = necro_getX;
            break;
        case 'y':
            instruction = necro_getY;
            break;
        default:
            necroCompilerErrorPrev(
                compilerPtr,
                "Expect only 'r', 't', 'x', or 'y'"
            );
            break;
    }
    necroCompilerWriteByte(compilerPtr, instruction);

    /* for set operations, see NamedVariable */
}

/* Resets the state of the specified compiler */
void necroCompilerReset(NecroCompiler *compilerPtr){
    memset(compilerPtr, 0, sizeof(*compilerPtr));
    compilerPtr->hadError = false;
    compilerPtr->inPanicMode = false;
    /*
     * no need to take care of the func compiler stack
     * since it lives on the C call stack as local
     * function variables
     */
}

/*
 * Frees the memory associated with the specified
 * NecroCompiler but not the generated program
 */
void necroCompilerFree(NecroCompiler *compilerPtr){
    necroLexerFree(&(compilerPtr->lexer));
    /* do not free generated program */
}

/*
 * compiles the specified Necro source file and
 * returns the program as a pointer to a newly
 * allocated NecroObjectFunc; error on compiler error
 */
NecroObjectFunc *necroCompilerCompileScript(
    NecroCompiler *compilerPtr,
    const char *fileName
){
    /* reset the compiler; nulls the funcPtr also */
    necroCompilerReset(compilerPtr);
    compilerPtr->lexer = necroLexerMake(fileName);

    /*
     * create the outermost func compiler with the
     * script type
     */
    _NecroFuncCompiler scriptFuncCompiler = {0};
    _necroFuncCompilerInit(
        &scriptFuncCompiler,
        necro_scriptFuncType,
        compilerPtr
    );

    necroCompilerAdvance(compilerPtr);

    while(!necroCompilerMatch(
        compilerPtr,
        necro_tokenEOF
    )){
        necroCompilerDeclaration(compilerPtr);
    }

    NecroObjectFunc *toRet = necroCompilerEnd(
        compilerPtr
    );

    bool hadError = compilerPtr->hadError;

    /*
     * doesn't free the program, just the resources
     * needed during compilation
     */
    necroCompilerFree(compilerPtr);
    
    if(hadError){
        necroObjectFree((NecroObject*)toRet);
        toRet = NULL;
        pgError(
            "halting due to Necro compiler error(s)"
        );
    }

    necroCompilerReset(compilerPtr);
    
    return toRet;
}