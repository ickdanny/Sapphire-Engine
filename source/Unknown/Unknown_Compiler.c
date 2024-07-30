#include "Unknown_Compiler.h"

#include <stdio.h>

#define maxParams 255

/* define for verbose compiler output for debugging */
/* #define COMPILER_VERBOSE */

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
typedef void (*ParseFunc)(UNCompiler*, bool);

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
#define number unCompilerNumber
#define unary unCompilerUnary
#define binary unCompilerBinary
#define grouping unCompilerGrouping
#define call unCompilerCall
#define dot unCompilerDot
#define variable unCompilerVariable
#define string unCompilerString
#define and_ unCompilerAnd
#define or_ unCompilerOr
#define bool_ unCompilerBool
#define vector unCompilerVector
#define point unCompilerPoint
static const UNParseRule parseRules[] = {
    [un_tokenLeftParen]
        = {grouping, call,   un_precCall},
    [un_tokenRightParen]
        = {NULL,     NULL,   un_precNone},
    [un_tokenLeftBrace]
        = {NULL,     NULL,   un_precNone},
    [un_tokenRightBrace]
        = {NULL,     NULL,   un_precNone},
    [un_tokenLeftBracket]
        = {point,     NULL,  un_precNone},
    [un_tokenRightBracket]
        = {NULL,     NULL,   un_precNone},
    [un_tokenComma]
        = {NULL,     NULL,   un_precNone},
    [un_tokenDot]
        = {NULL,     dot,    un_precCall},
    [un_tokenMinus]
        = {unary,    binary, un_precTerm},
    [un_tokenPercent]
        = {NULL,     binary, un_precFactor},
    [un_tokenPlus]
        = {NULL,     binary, un_precTerm},
    [un_tokenSemicolon]
        = {NULL,     NULL,   un_precNone},
    [un_tokenSlash]
        = {NULL,     binary, un_precFactor},
    [un_tokenStar]
        = {NULL,     binary, un_precFactor},
    [un_tokenBang]
        = {unary,    NULL,   un_precNone},
    [un_tokenBangEqual]
        = {NULL,     binary, un_precEquality},
    [un_tokenEqual]
        = {NULL,     NULL,   un_precNone},
    [un_tokenDoubleEqual]
        = {NULL,     binary, un_precEquality},
    [un_tokenGreater]
        = {NULL,     binary, un_precCompare},
    [un_tokenDoubleGreater]
        = {NULL,     NULL,   un_precNone},
    [un_tokenGreaterEqual]
        = {NULL,     binary, un_precCompare},
    [un_tokenLess]
        = {NULL,     binary, un_precCompare},
    [un_tokenDoubleLess]
        = {vector,   NULL,   un_precNone},
    [un_tokenLessEqual]
        = {NULL,     binary, un_precCompare},
    [un_tokenIdentifier]
        = {variable, NULL,   un_precNone},
    [un_tokenString]
        = {string,   NULL,   un_precNone},
    [un_tokenInt]
        = {number,   NULL,   un_precNone},
    [un_tokenFloat]
        = {number,   NULL,   un_precNone},
    [un_tokenAmpersand]
        = {NULL,     and_,   un_precAnd},
    [un_tokenElse]
        = {NULL,     NULL,   un_precNone},
    [un_tokenFalse]
        = {bool_,    NULL,   un_precNone},
    [un_tokenFor]
        = {NULL,     NULL,   un_precNone},
    [un_tokenFunc]
        = {NULL,     NULL,   un_precNone},
    [un_tokenIf]
        = {NULL,     NULL,   un_precNone},
    [un_tokenVerticalBar]
        = {NULL,     or_,    un_precOr},
    [un_tokenPrint]
        = {NULL,     NULL,   un_precNone},
    [un_tokenReturn]
        = {NULL,     NULL,   un_precNone},
    [un_tokenTrue]
        = {bool_,    NULL,   un_precNone},
    [un_tokenLet]
        = {NULL,     NULL,   un_precNone},
    [un_tokenWhile]
        = {NULL,     NULL,   un_precNone},
    [un_tokenYield]
        = {NULL,     NULL,   un_precNone},
    [un_tokenWait]
        = {NULL,     NULL,   un_precNone},
    [un_tokenError]
        = {NULL,     NULL,   un_precNone},
    [un_tokenEOF]
        = {NULL,     NULL,   un_precNone},
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

/*
 * Returns a pointer to the parse rule for the
 * specified token type
 */
static const UNParseRule *getRule(UNTokenType type){
    return &(parseRules[type]);
}

/*
 * Reserves the first stack slot for the VM's internal
 * use; error if called when locals already exist
 */
static void _unFuncCompilerReserveFirstStackSlot(
    _UNFuncCompiler *funcCompilerPtr
){
    assertTrue(
        funcCompilerPtr->localCount == 0,
        "error: must be called when local count is 0; "
        SRC_LOCATION
    );
    UNLocal *reservedPtr= &(funcCompilerPtr->locals[
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
static void _unFuncCompilerInit(
    _UNFuncCompiler *toInitPtr,
    UNFuncType funcType,
    UNCompiler *compilerPtr
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
        toInitPtr->funcPtr = unObjectFuncMake(
            toInitPtr->enclosingPtr->funcPtr
        );
    }
    else{
        toInitPtr->funcPtr = unObjectFuncMake(NULL);
    }
    /* single string map for all functions in a file */
    if(funcType != un_scriptFuncType){
        toInitPtr->funcPtr->namePtr
            = unObjectStringCopy(
                compilerPtr->prevToken.startPtr,
                compilerPtr->prevToken.length,
                NULL,
                toInitPtr->funcPtr->program.literals
                    .stringMapPtr
            );
    }
}

/*
 * Constructs and returns a new UNCompiler for the
 * specified file by value
 */
UNCompiler unCompilerMake(){
    UNCompiler toRet = {0};
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
    /* loop to report error tokens */
    while(true){
        /* get next token from lexer */
        compilerPtr->currentToken = unLexerNext(
            &(compilerPtr->lexer)
        );
        /* bail out if good token*/
        if(compilerPtr->currentToken.type
            != un_tokenError
        ){
            break;
        }

        /* report error token */
        unCompilerErrorCurrent(
            compilerPtr,
            compilerPtr->currentToken.startPtr
        );
    }
}

/*
 * Returns true if the current token in the specified
 * compiler matches the given type, false otherwise
 */
static bool unCompilerCheckType(
    UNCompiler *compilerPtr,
    UNTokenType type
){
    return compilerPtr->currentToken.type == type;
}

/*
 * Advances the specified compiler to the next token
 * if matches the given type, returns true if
 * match found, false otherwise
 */
static bool unCompilerMatch(
    UNCompiler *compilerPtr,
    UNTokenType type
){
    if(unCompilerCheckType(compilerPtr, type)){
        unCompilerAdvance(compilerPtr);
        return true;
    }
    return false;
}

/*
 * Advances the specified compiler to the next token
 * if it matches the given type, error if it doesn't
 */
#define unCompilerConsume( \
    COMPILERPTR, \
    TYPE, \
    ERRORMSG \
) \
    do{ \
        if(!unCompilerMatch( \
            (COMPILERPTR), \
            (TYPE) \
        )){ \
            unCompilerErrorPrev( \
                COMPILERPTR, \
                ERRORMSG \
            ); \
        } \
    } while(false);

/*
 * Gets the current program from the specified compiler
 */
static UNProgram *unCompilerGetCurrentProgram(
    UNCompiler *compilerPtr
){
    return &(compilerPtr->currentFuncCompilerPtr
        ->funcPtr->program);
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

/* Used to have the compiler enter a new local scope */
#define unCompilerBeginScope(COMPILERPTR) \
    (++((COMPILERPTR)->currentFuncCompilerPtr \
        ->scopeDepth))

/* Used to have the compiler exit a local scope */
static void unCompilerEndScope(
    UNCompiler *compilerPtr
){
    _UNFuncCompiler *funcCompilerPtr
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
        unCompilerWriteByte(compilerPtr, un_pop);
        --(funcCompilerPtr->localCount);
    }
}

/*
 * Performs actions at the end of the compilation
 * process for the specified compiler and returns the
 * pointer to the UNObjectFunc holding the compiled
 * program (or function)
 */
static UNObjectFunc *unCompilerEnd(
    UNCompiler *compilerPtr
){
    /* emit return for functions but not for scripts */
    if(compilerPtr->currentFuncCompilerPtr->funcType
        != un_scriptFuncType
    ){
        unCompilerWriteByte(compilerPtr, un_return);
    }
    else{
        /* for scripts, emit an end instruction */
        unCompilerWriteByte(compilerPtr, un_end);
    }
    UNObjectFunc *toRet 
        = compilerPtr->currentFuncCompilerPtr->funcPtr;
    
    #ifdef COMPILER_VERBOSE
    if(!(compilerPtr->hadError)){
        if(toRet->namePtr != NULL){
            unObjectPrint(
                unObjectValue(toRet->namePtr)
            );
            printf(":\n");
        }
        else{
            printf("unnamed:\n");
        }
        unProgramDisassemble(
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
static void unCompilerSynchronize(
    UNCompiler *compilerPtr
){
    compilerPtr->inPanicMode = false;
    while(compilerPtr->currentToken.type
        != un_tokenEOF
    ){
        /* if passed a semicolon, reached sync point */
        if(compilerPtr->prevToken.type
            == un_tokenSemicolon
        ){
            return;
        }
        switch(compilerPtr->currentToken.type){
            case un_tokenFunc:
            case un_tokenLet:
            case un_tokenFor:
            case un_tokenIf:
            case un_tokenWhile:
            case un_tokenYield:
            case un_tokenWait:
            case un_tokenPrint:
            case un_tokenReturn:
                return;
            default:
                /* do nothing */
                break;
        }

        unCompilerAdvance(compilerPtr);
    }
}

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
        &(unCompilerGetCurrentProgram(compilerPtr)
            ->literals),
        value
    );
    /*
     * error if the lit index doesn't fit in a single
     * byte since it needs to fit in the instruction
     */
    if(litIndex > UINT8_MAX){
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
            &(unCompilerGetCurrentProgram( \
                COMPILERPTR \
            )->literals), \
            VALUE \
        ) \
    )

/*
 * Parses the next expression for the specified
 * compiler at or above the given precedence level
 */
static void unCompilerExpressionPrecedence(
    UNCompiler *compilerPtr,
    UNPrecedence precedence
){
    /* read next token, check for prefix rule */
    unCompilerAdvance(compilerPtr);
    ParseFunc prefixFunc = getRule(
        compilerPtr->prevToken.type
    )->prefixFunc;

    /* no prefix rule indicates syntax error */
    if(!prefixFunc){
        unCompilerErrorPrev(
            compilerPtr,
            "Expect expression"
        );
        return;
    }
    bool canAssign = precedence <= un_precAssign;
    prefixFunc(compilerPtr, canAssign);

    /*
     * next token might indicate that the prefix 
     * expression is an operand of an infix one; make
     * sure that its precedence is high enough
     */
    while(precedence <= getRule(
        compilerPtr->currentToken.type)->precedence
    ){
        unCompilerAdvance(compilerPtr);
        ParseFunc infixFunc = getRule(
            compilerPtr->prevToken.type
        )->infixFunc;
        infixFunc(compilerPtr, canAssign);
    }

    /* error if trailing '=' */
    if(canAssign
        && unCompilerMatch(compilerPtr, un_tokenEqual)
    ){
        unCompilerErrorPrev(
            compilerPtr,
            "Invalid assignment target"
        );
    }
}

/* Parses the next number for the specified compiler */
void unCompilerNumber(
    UNCompiler *compilerPtr,
    bool canAssign
){
    UNValue value = {0};
    switch(compilerPtr->prevToken.type){
        case un_tokenInt:
            value = unIntValue(atoi(
                compilerPtr->prevToken.startPtr
            ));
            break;
        case un_tokenFloat:
            value = unFloatValue(
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
    unCompilerWriteLiteral(compilerPtr, value);
}
 
/*
 * Parses the next unary operator for the specified
 * compiler
 */
void unCompilerUnary(
    UNCompiler *compilerPtr,
    bool canAssign
){
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
        case un_tokenBang:
            unCompilerWriteByte(
                compilerPtr,
                un_not
            );
            break;
        case un_tokenMinus:
            unCompilerWriteByte(
                compilerPtr,
                un_negate
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
void unCompilerBinary(
    UNCompiler *compilerPtr,
    bool canAssign
){
    /*
     * save the type (since we have to read the rest
     * of the expression)
     */
    UNTokenType operatorType
        = compilerPtr->prevToken.type;
    
    const UNParseRule *rulePtr = getRule(operatorType);
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
        case un_tokenPercent:
            unCompilerWriteByte(
                compilerPtr,
                un_modulo
            );
            break;
        case un_tokenBangEqual:
            unCompilerWriteBytes(
                compilerPtr,
                un_equal,
                un_not
            );
            break;
        case un_tokenDoubleEqual:
            unCompilerWriteByte(
                compilerPtr,
                un_equal
            );
            break;
        case un_tokenGreater:
            unCompilerWriteByte(
                compilerPtr,
                un_greater
            );
            break;
        case un_tokenGreaterEqual:
            unCompilerWriteBytes(
                compilerPtr,
                un_less,
                un_not
            );
            break;
        case un_tokenLess:
            unCompilerWriteByte(
                compilerPtr,
                un_less
            );
            break;
        case un_tokenLessEqual:
            unCompilerWriteBytes(
                compilerPtr,
                un_greater,
                un_not
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
void unCompilerExpression(
    UNCompiler *compilerPtr
){
    unCompilerExpressionPrecedence(
        compilerPtr,
        un_precAssign
    );
}

/*
 * Parses the next declaration for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void unCompilerDeclaration(UNCompiler *compilerPtr){
    if(unCompilerMatch(compilerPtr, un_tokenFunc)){
        unCompilerFunctionDeclaration(compilerPtr);
    }
    else if(unCompilerMatch(compilerPtr, un_tokenLet)){
        unCompilerVariableDeclaration(compilerPtr);
    }
    else{
        unCompilerStatement(compilerPtr);
    }

    /* if compile error, go to a sync point */
    if(compilerPtr->inPanicMode){
        unCompilerSynchronize(compilerPtr);
    }
}

/*
 * Creates a new string literal for the specified
 * identifier and returns its literal index
 */
static uint8_t unCompilerIdentifierLiteral(
    UNCompiler *compilerPtr,
    UNToken *tokenPtr
){
    return unCompilerMakeLiteral(
        compilerPtr,
        unObjectValue(
            unObjectStringCopy(
                tokenPtr->startPtr,
                tokenPtr->length,
                NULL,
                unCompilerGetCurrentProgram(
                    compilerPtr)->literals.stringMapPtr
            )
        )
    );
}

/*
 * Adds information about local variable having the
 * given token as its name for the specified compiler
 */
static void unCompilerAddLocal(
    UNCompiler *compilerPtr,
    UNToken name
){
    _UNFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;
    if(funcCompilerPtr->localCount == _uint8_t_count){
        unCompilerErrorPrev(
            compilerPtr,
            "Too many locals in func"
        );
        return;
    }
    UNLocal *localPtr= &(funcCompilerPtr->locals[
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
    UNToken *tokenPtr1,
    UNToken *tokenPtr2
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
static void unCompilerDeclareVariable(
    UNCompiler *compilerPtr
){
    _UNFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;

    /* bail if in global scope */
    if(funcCompilerPtr->scopeDepth == 0){
        return;
    }

    UNToken *namePtr = &(compilerPtr->prevToken);

    /*
     * error if two variables in same depth w/ same
     * name
     */
    for(int i = funcCompilerPtr->localCount - 1;
        i >= 0;
        --i
    ){
        UNLocal *localPtr
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
            unCompilerErrorPrev(
                compilerPtr,
                "Preexisting variable with this name "
                "in this scope"
            );
        }
    }

    unCompilerAddLocal(compilerPtr, *namePtr);
}

/*
 * Parses the name of the next variable for the given
 * compiler and returns the index of the string
 * constant for the name
 */
static uint8_t unCompilerParseVariable(
    UNCompiler *compilerPtr,
    const char *errorMsg
){
    unCompilerConsume(
        compilerPtr,
        un_tokenIdentifier,
        errorMsg
    );

    unCompilerDeclareVariable(compilerPtr);

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

    return unCompilerIdentifierLiteral(
        compilerPtr,
        &(compilerPtr->prevToken)
    );
}

/*
 * Marks the topmost local as finished initialized
 * for the specified compiler; does nothing if the
 * current variable being compiled is global
 */
static void unCompilerMarkLocalInitialized(
    UNCompiler *compilerPtr
){
    _UNFuncCompiler *funcCompilerPtr
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
static void unCompilerDefineVariable(
    UNCompiler *compilerPtr,
    uint8_t globalIndex
){
    /*
     * do nothing if local variable - already sits
     * on top of stack
     */
    if(compilerPtr->currentFuncCompilerPtr->scopeDepth
        > 0
    ){
        unCompilerMarkLocalInitialized(compilerPtr);
        return;
    }

    /* write global variables */
    unCompilerWriteBytes(
        compilerPtr,
        un_defineGlobal,
        globalIndex
    );
}

/*
 * Helps the specified compiler parse the header
 * of a function (consuming from the '(' of the
 * parameter list to the '{' prior to the body)
 */
static void unCompilerFunctionHeader(
    UNCompiler *compilerPtr
){
    unCompilerConsume(
        compilerPtr,
        un_tokenLeftParen,
        "Expect '(' after func name"
    );
    /* get parameters */
    if(!unCompilerCheckType(
        compilerPtr,
        un_tokenRightParen
    )){
        do{
            ++(compilerPtr->currentFuncCompilerPtr
                ->funcPtr->arity);
            if(compilerPtr->currentFuncCompilerPtr
                ->funcPtr->arity > maxParams
            ){
                unCompilerErrorCurrent(
                    compilerPtr,
                    "Parameters exceeding max"
                );
            }
            uint8_t paramNameIndex
                = unCompilerParseVariable(
                    compilerPtr,
                    "Expect parameter name"
                );
            unCompilerDefineVariable(
                compilerPtr,
                paramNameIndex
            );
        } while(unCompilerMatch(
            compilerPtr,
            un_tokenComma
        ));
    }
    unCompilerConsume(
        compilerPtr,
        un_tokenRightParen,
        "Expect ')' after parameters"
    );
    unCompilerConsume(
        compilerPtr,
        un_tokenLeftBrace,
        "Expect '{' before function body"
    );
}

/* Helps the specified compiler parse a function */
static void unCompilerFunction(
    UNCompiler *compilerPtr,
    UNFuncType funcType
){
    /* create new func compiler for the new function */
    _UNFuncCompiler newFuncCompiler = {0};
    _unFuncCompilerInit(
        &newFuncCompiler,
        funcType,
        compilerPtr
    );
    /* no corresponding end scope; unneeded */
    unCompilerBeginScope(compilerPtr);

    /* consume syntax of function declaration */
    unCompilerFunctionHeader(compilerPtr);

    /* compile function body */
    unCompilerBlock(compilerPtr);

    /*
     * retrieve code for the function and write to the
     * enclosing code as a literal
     */
    UNObjectFunc *funcPtr = unCompilerEnd(compilerPtr);
    unCompilerWriteBytes(
        compilerPtr,
        un_literal,
        unCompilerMakeLiteral(
            compilerPtr,
            unObjectValue(funcPtr)
        )
    );
}

/*
 * Parses the next function declaration for the
 * specified compiler
 */
void unCompilerFunctionDeclaration(
    UNCompiler *compilerPtr
){
    uint8_t globalIndex = unCompilerParseVariable(
        compilerPtr,
        "Expect function name"
    );
    /*
     * marks local functions as initialized but does
     * nothing for global functions
     */
    unCompilerMarkLocalInitialized(compilerPtr);
    unCompilerFunction(
        compilerPtr,
        un_functionFuncType
    );
    unCompilerDefineVariable(compilerPtr, globalIndex);
}

/*
 * Parses the next variable declaration for the
 * specified compiler; uninitialized variables get the
 * default value of FALSE
 */
void unCompilerVariableDeclaration(
    UNCompiler *compilerPtr
){
    uint8_t globalIndex = unCompilerParseVariable(
        compilerPtr,
        "Expect variable name"
    );

    if(unCompilerMatch(compilerPtr, un_tokenEqual)){
        unCompilerExpression(compilerPtr);
    }
    else{
        /*
         * if uninitialized, variables get set to
         * the value FALSE
         */
        unCompilerWriteByte(compilerPtr, un_false);
    }

    /* eat the semicolon */
    unCompilerConsume(
        compilerPtr,
        un_tokenSemicolon,
        "expect ';' after var declare; "
    );

    unCompilerDefineVariable(compilerPtr, globalIndex);
}

/*
 * Parses the next statement for the specified
 * compiler; a declaration includes declares whereas
 * a statement does not
 */
void unCompilerStatement(UNCompiler *compilerPtr){
    /* match the print statement */
    if(unCompilerMatch(compilerPtr, un_tokenPrint)){
        unCompilerPrintStatement(compilerPtr);
    }
    /* match an if statement */
    else if(unCompilerMatch(compilerPtr, un_tokenIf)){
        unCompilerIfStatement(compilerPtr);
    }
    /* match a while loop */
    else if(unCompilerMatch(
        compilerPtr,
        un_tokenWhile
    )){
        unCompilerWhileStatement(compilerPtr);
    }
    /* match a for loop */
    else if(unCompilerMatch(compilerPtr, un_tokenFor)){
        unCompilerForStatement(compilerPtr);
    }
    /* match a return */
    else if(unCompilerMatch(
        compilerPtr,
        un_tokenReturn
    )){
        unCompilerReturnStatement(compilerPtr);
    }
    /* match a yield */
    else if(unCompilerMatch(
        compilerPtr,
        un_tokenYield
    )){
        unCompilerYieldStatement(compilerPtr);
    }
    /* match a wait loop */
    else if(unCompilerMatch(
        compilerPtr,
        un_tokenWait
    )){
        unCompilerWaitStatement(compilerPtr);
    }
    /* match a block */
    else if(unCompilerMatch(
        compilerPtr,
        un_tokenLeftBrace)
    ){
        unCompilerBeginScope(compilerPtr);
        unCompilerBlock(compilerPtr);
        unCompilerEndScope(compilerPtr);
    }
    /* if none of the above, expression statement */
    else{
        unCompilerExpressionStatement(compilerPtr);
    }
}

/*
 * Parses the next print statement for the specified
 * compiler
 */
void unCompilerPrintStatement(UNCompiler *compilerPtr){
    /* compile the expression after "print" */
    unCompilerExpression(compilerPtr);
    /* eat the semicolon */
    unCompilerConsume(
        compilerPtr,
        un_tokenSemicolon,
        "expect ';' after print statement; "
    );
    /* print the result of the expression */
    unCompilerWriteByte(compilerPtr, un_print);
}

/*
 * Writes a jump instruction for the specified compiler
 * and returns the index in the code of the instruction
 */
static int unCompilerWriteJump(
    UNCompiler *compilerPtr,
    uint8_t instruction
){
    unCompilerWriteByte(compilerPtr, instruction);

    /* emit 2 bytes of placeholder values */
    unCompilerWriteByte(compilerPtr, 0xFF);
    unCompilerWriteByte(compilerPtr, 0xFF);

    /* return index of instruction */
    return unCompilerGetCurrentProgram(
        compilerPtr
    )->code.size - 2;
}

/*
 * Patches the jump distance into the jump instruction
 * at the given index for the specified compiler
 */
static void unCompilerPatchJump(
    UNCompiler *compilerPtr,
    int jumpInstructionIndex
){
    UNProgram *currentProgramPtr
        = unCompilerGetCurrentProgram(compilerPtr);
    /*
     * calculate how far the jump is; -2 to account
     * for the fact that the ditsance itself is going
     * to be encoded in the code
     */
    int jumpDist = currentProgramPtr->code.size
        - jumpInstructionIndex - 2;

    if(jumpDist > UINT16_MAX){
        unCompilerErrorPrev(
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
void unCompilerIfStatement(UNCompiler *compilerPtr){
    unCompilerConsume(
        compilerPtr,
        un_tokenLeftParen,
        "Expect '(' after \"if\""
    );
    unCompilerExpression(compilerPtr);
    unCompilerConsume(
        compilerPtr,
        un_tokenRightParen,
        "Expect ')' after if condition"
    );

    /*
     * save the index to write the address of the jump
     * instruction
     */
    int thenJumpInstructionIndex = unCompilerWriteJump(
        compilerPtr,
        un_jumpIfFalse
    );

    /* have the VM pop off the condition if true */
    unCompilerWriteByte(compilerPtr, un_pop);

    /* parse the then block */
    unCompilerStatement(compilerPtr);

    /* write unconditional jump past else block */
    int elseJumpInstructionIndex = unCompilerWriteJump(
        compilerPtr,
        un_jump
    );

    /* write the address after compiling statement */
    unCompilerPatchJump(
        compilerPtr,
        thenJumpInstructionIndex
    );

    /* have the VM pop off the condition if false */
    unCompilerWriteByte(compilerPtr, un_pop);

    /* check for else branch */
    if(unCompilerMatch(compilerPtr, un_tokenElse)){
        unCompilerStatement(compilerPtr);
    }

    /* patch the unconditional jump past the else */
    unCompilerPatchJump(
        compilerPtr,
        elseJumpInstructionIndex
    );
}

/*
 * Writes a loop instruction for the specified
 * compiler (a loop instruction is just a backwards
 * jump)
 */
static void unCompilerWriteLoop(
    UNCompiler *compilerPtr,
    int loopStartIndex
){
    unCompilerWriteByte(compilerPtr, un_loop);

    /* +2 to account for size of operands */
    int offset
        = unCompilerGetCurrentProgram(compilerPtr)
            ->code.size - loopStartIndex + 2;
    if(offset > UINT16_MAX){
        unCompilerErrorPrev(
            compilerPtr,
            "Loop body too large"
        );
    }

    unCompilerWriteByte(
        compilerPtr,
        (offset >> 8) & 0xFF
    );
    unCompilerWriteByte(compilerPtr, offset & 0xFF);
}

/*
 * Parses the next while statement for the specified
 * compiler
 */
void unCompilerWhileStatement(UNCompiler *compilerPtr){
    /* save index before the condition check */
    int loopStartIndex
        = unCompilerGetCurrentProgram(compilerPtr)
            ->code.size;

    unCompilerConsume(
        compilerPtr,
        un_tokenLeftParen,
        "Expect '(' after \"while\""
    );
    unCompilerExpression(compilerPtr);
    unCompilerConsume(
        compilerPtr,
        un_tokenRightParen,
        "Expect ')' after while condition"
    );

    int exitJumpInstructionIndex = unCompilerWriteJump(
        compilerPtr,
        un_jumpIfFalse
    );
    unCompilerWriteByte(compilerPtr, un_pop);
    unCompilerStatement(compilerPtr);
    unCompilerWriteLoop(compilerPtr, loopStartIndex);

    unCompilerPatchJump(
        compilerPtr,
        exitJumpInstructionIndex
    );
    /* need to pop false off the stack */
    unCompilerWriteByte(compilerPtr, un_pop);
}

/*
 * Parses the next for statement for the specified
 * compiler
 */
void unCompilerForStatement(UNCompiler *compilerPtr){
    /*
     * a for loop has its own scope in case the
     * initializer is a var declare
     */
    unCompilerBeginScope(compilerPtr);

    unCompilerConsume(
        compilerPtr,
        un_tokenLeftParen,
        "Expect '(' after \"for\""
    );

    /* parse initializer */
    if(unCompilerMatch(
        compilerPtr,
        un_tokenSemicolon
    )){
        /* no initializer */
    }
    else if(unCompilerMatch(compilerPtr, un_tokenLet)){
        /* variable declaration initializer */
        unCompilerVariableDeclaration(compilerPtr);
    }
    else{
        /* expression statement initializer */
        unCompilerExpressionStatement(compilerPtr);
    }

    /* save index before the condition check */
    int loopStartIndex
        = unCompilerGetCurrentProgram(compilerPtr)
            ->code.size;

    /* parse condition */
    int exitJumpInstructionIndex = -1;
    if(!unCompilerMatch(
        compilerPtr,
        un_tokenSemicolon)
    ){
        /* expr not stmt since don't want to pop */
        unCompilerExpression(compilerPtr);
        unCompilerConsume(
            compilerPtr,
            un_tokenSemicolon,
            "Expect ';'"
        );

        /* jump out if condition false */
        exitJumpInstructionIndex = unCompilerWriteJump(
            compilerPtr,
            un_jumpIfFalse
        );
        unCompilerWriteByte(compilerPtr, un_pop);
    }

    /* parse increment */
    if(!unCompilerMatch(
        compilerPtr,
        un_tokenRightParen)
    ){
        /*
         * jump over increment, run body, then jump
         * back to the increment
         */
        int bodyJumpInstructionIndex
            = unCompilerWriteJump(
                compilerPtr,
                un_jump
            );
        int incrementStartIndex
            = unCompilerGetCurrentProgram(compilerPtr)
                ->code.size;
        /* expr since no semicolon at end */
        unCompilerExpression(compilerPtr);
        unCompilerWriteByte(compilerPtr, un_pop);

        unCompilerConsume(
            compilerPtr,
            un_tokenRightParen,
            "Expect ')' after for clauses"
        );

        /* jump to condition check */
        unCompilerWriteLoop(
            compilerPtr,
            loopStartIndex
        );
        /* make it so the body jumps to increment */
        loopStartIndex = incrementStartIndex;
        /* jump past the increment at first */
        unCompilerPatchJump(
            compilerPtr,
            bodyJumpInstructionIndex
        );
    }

    unCompilerStatement(compilerPtr);
    /*
     * loops to either the condition or the increment
     * if it is present
     */
    unCompilerWriteLoop(compilerPtr, loopStartIndex);

    /* patch the jump from the condition */
    if(exitJumpInstructionIndex != -1){
        unCompilerPatchJump(
            compilerPtr,
            exitJumpInstructionIndex
        );
        unCompilerWriteByte(compilerPtr, un_pop);
    }

    unCompilerEndScope(compilerPtr);
}

/*
 * Parses the next yield statement for the specified
 * compiler
 */
void unCompilerYieldStatement(UNCompiler *compilerPtr){
    /* eat the semicolon */
    unCompilerConsume(
        compilerPtr,
        un_tokenSemicolon,
        "expect ';' after yield statement"
    );
    /* write a yield instruction */
    unCompilerWriteByte(compilerPtr, un_yield);
}

/*
 * Parses the next wait statement for the specified
 * compiler
 */
void unCompilerWaitStatement(UNCompiler *compilerPtr){
    /* save index before the condition check */
    int loopStartIndex
        = unCompilerGetCurrentProgram(compilerPtr)
            ->code.size;

    unCompilerExpression(compilerPtr);
    unCompilerConsume(
        compilerPtr,
        un_tokenSemicolon,
        "Expect ';' after wait statement"
    );

    /* jump to end if true */
    unCompilerWriteByte(compilerPtr, un_not);
    int exitJumpInstructionIndex = unCompilerWriteJump(
        compilerPtr,
        un_jumpIfFalse
    );
    unCompilerWriteByte(compilerPtr, un_pop);
    unCompilerWriteByte(compilerPtr, un_yield);
    unCompilerWriteLoop(compilerPtr, loopStartIndex);

    unCompilerPatchJump(
        compilerPtr,
        exitJumpInstructionIndex
    );
}

/*
 * writes a sequence of instructions representing a
 * null return; it will return false
 */
#define unCompilerWriteNullReturn(COMPILERPTR) \
    do{ \
        unCompilerWriteByte(compilerPtr, un_false); \
        unCompilerWriteByte(compilerPtr, un_return); \
    } while(false)

/*
 * Parses the next return statement for the specified
 * compiler
 */
void unCompilerReturnStatement(
    UNCompiler *compilerPtr
){
    /*
     * error if trying to return in the top level
     * script
     */
    if(compilerPtr->currentFuncCompilerPtr->funcType
        == un_scriptFuncType
    ){
        unCompilerErrorPrev(
            compilerPtr,
            "Cannot return from top level script"
        );
    }
    /*
     * if the next token is a semicolon, this is a
     * null return
     */
    if(unCompilerMatch(
        compilerPtr,
        un_tokenSemicolon
    )){
        unCompilerWriteNullReturn(compilerPtr);
    }
    /* otherwise, parse the expression to return */
    else{
        unCompilerExpression(compilerPtr);
        unCompilerConsume(
            compilerPtr,
            un_tokenSemicolon,
            "Expect ';' after return value"
        );
        unCompilerWriteByte(compilerPtr, un_return);
    }
}

/* Parses the next block for the specified compiler */
void unCompilerBlock(UNCompiler *compilerPtr){
    while(!unCompilerCheckType(
            compilerPtr,
            un_tokenRightBrace
        ) && !unCompilerCheckType(
            compilerPtr,
            un_tokenEOF
        )
    ){
        unCompilerDeclaration(compilerPtr);
    }

    unCompilerConsume(
        compilerPtr,
        un_tokenRightBrace,
        "Expect '}' after block"
    );
}

/*
 * Parses the next expression statement for the
 * specified compiler
 */
void unCompilerExpressionStatement(
    UNCompiler *compilerPtr
){
    unCompilerExpression(compilerPtr);
    /* eat the semicolon */
    unCompilerConsume(
        compilerPtr,
        un_tokenSemicolon,
        "expect ';' after expr statement; "
    );
    /* discard the result of the expression */
    unCompilerWriteByte(compilerPtr, un_pop);
}

/*
 * Parses an expression within a set of parenthesis
 * for the specified compiler - should be called after
 * the first parenthesis has been matched
 */
void unCompilerGrouping(
    UNCompiler *compilerPtr,
    bool canAssign
){
    unCompilerExpression(compilerPtr);
    unCompilerConsume(
        compilerPtr,
        un_tokenRightParen,
        "expect ')' after expression; "
    );
}

/*
 * Parses the argument list of a function call for the
 * specified compiler and returns the number of
 * arguments passed
 */
static uint8_t unCompilerArgumentList(
    UNCompiler *compilerPtr
){
    uint8_t numArgs = 0;
    /*
     * compile each expression separated by commas
     * until the closing paren
     */
    if(!unCompilerCheckType(
        compilerPtr,
        un_tokenRightParen
    )){
        do{
            unCompilerExpression(compilerPtr);
            if(numArgs == maxParams){
                unCompilerErrorPrev(
                    compilerPtr,
                    "too many arguments"
                );
            }
            ++numArgs;
        } while(unCompilerMatch(
            compilerPtr,
            un_tokenComma
        ));
    }
    unCompilerConsume(
        compilerPtr,
        un_tokenRightParen,
        "Expect ')' after args"
    );
    return numArgs;
}

/* Parses a function call for the specified compiler */
void unCompilerCall(
    UNCompiler *compilerPtr,
    bool canAssign
){
    uint8_t numArgs = unCompilerArgumentList(
        compilerPtr
    );
    unCompilerWriteBytes(
        compilerPtr,
        un_call,
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
static int unCompilerResolveLocal(
    UNCompiler *compilerPtr,
    UNToken *name
){
    _UNFuncCompiler *funcCompilerPtr
        = compilerPtr->currentFuncCompilerPtr;
    for(int i = funcCompilerPtr->localCount - 1;
        i >= 0;
        -- i
    ){
        UNLocal *localPtr
            = &(funcCompilerPtr->locals[i]);
        if(identifiersEqual(name, &(localPtr->name))){
            /*
             * handle case of uninitialized local
             * trying to read itself by checking for
             * the sentinel value -1
             */
            if(localPtr->depth == -1){
                unCompilerErrorPrev(
                    compilerPtr,
                    "Cannot read local var in its own "
                    "initializer"
                );
            }
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
static void unCompilerNamedVariable(
    UNCompiler *compilerPtr,
    UNToken varName,
    bool canAssign
){
    uint8_t getOp = 0;
    uint8_t setOp = 0;

    /*
     * eventually becomes the second byte following
     * the instruction code; the stack slot for locals
     * and the index of the name literal for globals
     */
    int arg = unCompilerResolveLocal(
        compilerPtr,
        &varName
    );

    bool isLocal = (arg != -1);

    /* if variable was resolved to a local */
    if(isLocal){
        getOp = un_getLocal;
        setOp = un_setLocal;
    }
    else{
        arg = unCompilerIdentifierLiteral(
            compilerPtr,
            &varName
        );
        getOp = un_getGlobal;
        setOp = un_setGlobal;
    }

    /*
     * if following token is dot, either get/set on
     * vector or point
     */
    if(unCompilerMatch(compilerPtr, un_tokenDot)){
        /* expect current token to be an identifier */
        unCompilerConsume(
            compilerPtr,
            un_tokenIdentifier,
            "Expect 'r', 't', 'x', or 'y' following "
            "a '.'"
        )
        if(compilerPtr->prevToken.length != 1){
            unCompilerErrorPrev(
                compilerPtr,
                "Expect only 'r', 't', 'x', or 'y'"
            );
        }
        /*
         * VN needs 2 instructions to get the value
         * of a member; one to load the whole composite
         * type, another to get specifically the member
         */
        UNInstruction memberGetOp = 0;
        switch(*(compilerPtr->prevToken.startPtr)){
            case 'r':
                memberGetOp = un_getR;
                setOp = isLocal 
                    ? un_setRLocal : un_setRGlobal;
                break;
            case 't':
                memberGetOp = un_getTheta;
                setOp = isLocal
                    ? un_setThetaLocal
                    : un_setThetaGlobal;
                break;
            case 'x':
                memberGetOp = un_getX;
                setOp = isLocal 
                    ? un_setXLocal : un_setXGlobal;
                break;
            case 'y':
                memberGetOp = un_getY;
                setOp = isLocal 
                    ? un_setYLocal : un_setYGlobal;
                break;
            default:
                unCompilerErrorPrev(
                    compilerPtr,
                    "Expect only 'r', 't', 'x', or 'y'"
                );
                break;
        }
        /*
         * if following token is equal, member
         * assignment
         */
        if(canAssign
            && unCompilerMatch(compilerPtr, un_tokenEqual)
        ){
            unCompilerExpression(compilerPtr);
            unCompilerWriteBytes(
                compilerPtr,
                setOp,
                (uint8_t)arg
            );
        }
        /* otherwise member get */
        else{
            unCompilerWriteBytes(
                compilerPtr,
                getOp,
                (uint8_t)arg
            );
            unCompilerWriteByte(
                compilerPtr,
                memberGetOp
            );
        }
    }
    /* if following token is equal, assignment */
    else if(canAssign
        && unCompilerMatch(compilerPtr, un_tokenEqual)
    ){
        unCompilerExpression(compilerPtr);
        unCompilerWriteBytes(
            compilerPtr,
            setOp,
            (uint8_t)arg
        );
    }
    /* otherwise just a get */
    else{
        unCompilerWriteBytes(
            compilerPtr,
            getOp,
            (uint8_t)arg
        );
    }
}

/*
 * Parses a variable for the specified compiler
 */
void unCompilerVariable(
    UNCompiler *compilerPtr,
    bool canAssign
){
    unCompilerNamedVariable(
        compilerPtr,
        compilerPtr->prevToken,
        canAssign
    );
}

/*
 * Parses a string for the specified compiler
 */
void unCompilerString(
    UNCompiler *compilerPtr,
    bool canAssign
){
    /*
     * The object is not referenced from the VM's
     * object list as it is always accessible from
     * the literals
     */
    unCompilerWriteLiteral(
        compilerPtr,
        unObjectValue(
            unObjectStringCopy(
                compilerPtr->prevToken.startPtr + 1,
                compilerPtr->prevToken.length - 2,
                NULL,
                unCompilerGetCurrentProgram(
                    compilerPtr
                )->literals.stringMapPtr
            )
        )
    );
}

/*
 * Parses a boolean AND for the specified compiler
 */
void unCompilerAnd(
    UNCompiler *compilerPtr,
    bool canAssign
){
    /*
     * if stack top is false, the left hand side of
     * the AND is false thus we short circuit and
     * skip computing the right hand side; otherwise,
     * discard the left hand side and evalute the
     * right hand side as the entire expression
     */
    int endJumpInstructionIndex = unCompilerWriteJump(
        compilerPtr,
        un_jumpIfFalse
    );
    unCompilerWriteByte(compilerPtr, un_pop);
    unCompilerExpressionPrecedence(
        compilerPtr,
        un_precAnd
    );
    unCompilerPatchJump(
        compilerPtr,
        endJumpInstructionIndex
    );
}

/*
 * Parses a boolean OR for the specified compiler
 */
void unCompilerOr(
    UNCompiler *compilerPtr,
    bool canAssign
){
    /*
     * if left hand is false, jump over the next jump
     * to the end which would short circuit
     */
    int elseJumpInstructionIndex = unCompilerWriteJump(
        compilerPtr,
        un_jumpIfFalse
    );
    int endJumpInstructionIndex = unCompilerWriteJump(
        compilerPtr,
        un_jump
    );
    unCompilerPatchJump(
        compilerPtr,
        elseJumpInstructionIndex
    );
    unCompilerWriteByte(compilerPtr, un_pop);
    unCompilerExpressionPrecedence(
        compilerPtr,
        un_precAnd
    );
    unCompilerPatchJump(
        compilerPtr,
        endJumpInstructionIndex
    );
}

/*
 * Parses a bool for the specified compiler
 */
void unCompilerBool(
    UNCompiler *compilerPtr,
    bool canAssign
){
    switch(compilerPtr->prevToken.type){
        case un_tokenFalse:
            unCompilerWriteByte(
                compilerPtr,
                un_false
            );
            break;
        case un_tokenTrue:
            unCompilerWriteByte(
                compilerPtr,
                un_true
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
void unCompilerVector(
    UNCompiler *compilerPtr,
    bool canAssign
){
    /* parse the R expression */
    unCompilerExpression(compilerPtr);

    /* eat the comma */
    unCompilerConsume(
        compilerPtr,
        un_tokenComma,
        "Expect ',' after R initializer"
    );

    /* parse the theta expression */
    unCompilerExpression(compilerPtr);

    /* eat the closing '>>' */
    unCompilerConsume(
        compilerPtr,
        un_tokenDoubleGreater,
        "Expect \">>\" after theta initializer"
    );

    /* emit the instruction for creating a vector */
    unCompilerWriteByte(compilerPtr, un_makeVector);
}

/* Parses a point for the specified compiler */
void unCompilerPoint(
    UNCompiler *compilerPtr,
    bool canAssign
){
    /* parse the X expression */
    unCompilerExpression(compilerPtr);

    /* eat the comma */
    unCompilerConsume(
        compilerPtr,
        un_tokenComma,
        "Expect ',' after X initializer"
    );

    /* parse the Y expression */
    unCompilerExpression(compilerPtr);

    /* eat the closing ']' */
    unCompilerConsume(
        compilerPtr,
        un_tokenRightBracket,
        "Expect ']' after Y initializer"
    );

    /* emit the instruction for creating a vector */
    unCompilerWriteByte(compilerPtr, un_makePoint);
}

/*
 * Parses a dot for the specified compiler
 */
void unCompilerDot(
    UNCompiler *compilerPtr,
    bool canAssign
){
    /*
     * should not be reached when the operand is a
     * variable name; special case for variables to
     * handle that
     */
    /* expect current token to be an identifier */
    unCompilerConsume(
        compilerPtr,
        un_tokenIdentifier,
        "Expect 'r', 't', 'x', or 'y' following a '.'"
    )
    if(compilerPtr->prevToken.length != 1){
        unCompilerErrorPrev(
            compilerPtr,
            "Expect only 'r', 't', 'x', or 'y'"
        );
    }
    /* only get for non-variable names */
    UNInstruction instruction = 0;
    switch(*(compilerPtr->prevToken.startPtr)){
        case 'r':
            instruction = un_getR;
            break;
        case 't':
            instruction = un_getTheta;
            break;
        case 'x':
            instruction = un_getX;
            break;
        case 'y':
            instruction = un_getY;
            break;
        default:
            unCompilerErrorPrev(
                compilerPtr,
                "Expect only 'r', 't', 'x', or 'y'"
            );
            break;
    }
    unCompilerWriteByte(compilerPtr, instruction);
}

/* Resets the state of the specified compiler */
void unCompilerReset(UNCompiler *compilerPtr){
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
 * UNCompiler but not the generated program
 */
void unCompilerFree(UNCompiler *compilerPtr){
    unLexerFree(&(compilerPtr->lexer));
    /* do not free generated program */
}

/*
 * compiles the specified Unknown source file and
 * returns the program as a pointer to a newly
 * allocated UNObjectFunc; error on compiler error
 */
UNObjectFunc *unCompilerCompileScript(
    UNCompiler *compilerPtr,
    const char *fileName
){
    /* reset the compiler; nulls the funcPtr also */
    unCompilerReset(compilerPtr);
    compilerPtr->lexer = unLexerMake(fileName);

    /* create the outermost func compiler */
    _UNFuncCompiler scriptFuncCompiler = {0};
    _unFuncCompilerInit(
        &scriptFuncCompiler,
        un_scriptFuncType,
        compilerPtr
    );

    unCompilerAdvance(compilerPtr);

    while(!unCompilerMatch(
        compilerPtr,
        un_tokenEOF
    )){
        unCompilerDeclaration(compilerPtr);
    }

    UNObjectFunc *toRet = unCompilerEnd(compilerPtr);

    bool hadError = compilerPtr->hadError;

    /*
     * doesn't free the program, just the resources
     * needed during compilation
     */
    unCompilerFree(compilerPtr);
    
    if(hadError){
        unObjectFree((UNObject*)toRet);
        toRet = NULL;
        pgError(
            "halting due to Unknown compiler error(s)"
        );
    }

    unCompilerReset(compilerPtr);
    
    return toRet;
}

/*
 * compiles the specified Unknown function file and
 * returns the program as a pointer to a newly
 * allocated UNObjectFunc; error on compiler error
 */
UNObjectFunc *unCompilerCompileFuncFile(
    UNCompiler *compilerPtr,
    const char *fileName
){
    /* reset the compiler; nulls the funcPtr also */
    unCompilerReset(compilerPtr);
    compilerPtr->lexer = unLexerMake(fileName);

    /* create the outermost func compiler */
    _UNFuncCompiler scriptFuncCompiler = {0};
    _unFuncCompilerInit(
        &scriptFuncCompiler,
        un_scriptFuncType,
        compilerPtr
    );
    /*
     * change the type from script to file; can't just
     * call _unFunccompilerInit because it makes
     * assumptions
     */
    scriptFuncCompiler.funcType = un_functionFuncType;

    unCompilerAdvance(compilerPtr);

    /* no corresponding end scope; unneeded */
    unCompilerBeginScope(compilerPtr);

    /* consume syntax of function declaration */
    unCompilerFunctionHeader(compilerPtr);

    /* compile function body */
    unCompilerBlock(compilerPtr);

    UNObjectFunc *toRet = unCompilerEnd(compilerPtr);

    bool hadError = compilerPtr->hadError;

    /*
     * doesn't free the program, just the resources
     * needed during compilation
     */
    unCompilerFree(compilerPtr);
    
    if(hadError){
        unObjectFree((UNObject*)toRet);
        toRet = NULL;
        pgError(
            "halting due to Unknown compiler error(s)"
        );
    }

    unCompilerReset(compilerPtr);
    
    return toRet;
}