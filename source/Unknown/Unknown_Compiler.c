#include "Unknown_Compiler.h"

#include <stdio.h>

#include "Unknown_Object.h"

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
static const UNParseRule parseRules[] = {
    [un_tokenLeftParen]
        = {grouping, call,   un_precCall},
    [un_tokenRightParen]
        = {NULL,     NULL,   un_precNone},
    [un_tokenLeftBrace]
        = {NULL,     NULL,   un_precNone},
    [un_tokenRightBrace]
        = {NULL,     NULL,   un_precNone},
    [un_tokenComma]
        = {NULL,     NULL,   un_precNone},
    [un_tokenDot]
        = {NULL,     dot,    un_precCall},
    [un_tokenMinus]
        = {unary,    binary, un_precTerm},
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
    [un_tokenGreaterEqual]
        = {NULL,     binary, un_precCompare},
    [un_tokenLess]
        = {NULL,     binary, un_precCompare},
    [un_tokenLessEqual]
        = {NULL,     binary, un_precCompare},
    [un_tokenIdentifier]
        = {variable, NULL,   un_precNone},
    [un_tokenString]
        = {string,   NULL,   un_precNone},
    [un_tokenNumber]
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

/*
 * Returns a pointer to the parse rule for the
 * specified token type
 */
static const UNParseRule *getRule(UNTokenType type){
    return &(parseRules[type]);
}

/* Resets the state of the specified local info */
void unLocalInfoReset(UNLocalInfo *localInfoPtr){
    memset(localInfoPtr, 0, sizeof(*localInfoPtr));
}

/*
 * Constructs and returns a new UNCompiler for the
 * specified file by value
 */
UNCompiler unCompilerMake(){
    UNCompiler toRet = {0};
    unLocalInfoReset(&(toRet.localInfo));
    toRet.hadError = false;
    toRet.inPanicMode = false;
    return toRet;
}

/* Resets the state of the specified compiler */
void unCompilerReset(UNCompiler *compilerPtr){
    memset(compilerPtr, 0, sizeof(*compilerPtr));
    unLocalInfoReset(&(compilerPtr->localInfo));
    compilerPtr->hadError = false;
    compilerPtr->inPanicMode = false;
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
    assertTrue( \
        unCompilerMatch( \
            (COMPILERPTR), \
            (TYPE) \
        ), \
        ERRORMSG \
    )

/*
 * Gets the current program from the specified compiler
 * //todo: update getCurrentProgram later
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
            //todo: possible other types of declares
            case un_tokenFunc:
            case un_tokenLet:
            case un_tokenFor:
            case un_tokenIf:
            case un_tokenWhile:
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
        &(compilerPtr->compiledProgram.literals),
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
            &((COMPILERPTR)->compiledProgram \
                .literals), \
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
    UNValue value = unNumberValue(strtod(
        compilerPtr->prevToken.startPtr,
        NULL
    ));
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
    if(unCompilerMatch(compilerPtr, un_tokenLet)){
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
                &(compilerPtr->compiledProgram
                    .literals.stringMap)
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
    UNLocalInfo *localInfoPtr
        = &(compilerPtr->localInfo);
    if(localInfoPtr->localCount == _uint8_t_count){
        unCompilerErrorPrev(
            compilerPtr,
            "Too many locals in func"
        );
        return;
    }
    UNLocal *localPtr= &(localInfoPtr->locals[
        localInfoPtr->localCount++
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
    UNLocalInfo *localInfoPtr
        = &(compilerPtr->localInfo);

    /* bail if in global scope */
    if(localInfoPtr->scopeDepth == 0){
        return;
    }

    UNToken *namePtr = &(compilerPtr->prevToken);

    /*
     * error if two variables in same depth w/ same
     * name
     */
    for(int i = localInfoPtr->localCount - 1;
        i >= 0;
        --i
    ){
        UNLocal *localPtr = &(localInfoPtr->locals[i]);

        /* break out if find local in higher depth */
        if(localPtr->depth != -1
            && localPtr->depth
                < localInfoPtr->scopeDepth
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
    if(compilerPtr->localInfo.scopeDepth > 0){
        return 0;
    }

    return unCompilerIdentifierLiteral(
        compilerPtr,
        &(compilerPtr->prevToken)
    );
}

/*
 * Marks the topmost local as finished initialized
 * for the specified compiler
 */
static void unCompilerMarkLocalInitialized(
    UNCompiler *compilerPtr
){
    UNLocalInfo *localInfoPtr
        = &(compilerPtr->localInfo);
    localInfoPtr->locals[
        localInfoPtr->localCount - 1
    ].depth = localInfoPtr->scopeDepth;
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
    if(compilerPtr->localInfo.scopeDepth > 0){
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
 * Parses the next variable declaration for the
 * specified compiler; uninitialized variables get the
 * default value of FALSE
 */
void unCompilerVariableDeclaration(
    UNCompiler *compilerPtr
){
    uint8_t globalIndex = unCompilerParseVariable(
        compilerPtr,
        "expect variable name"
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

/* Used to have the compiler enter a new local scope */
#define unCompilerBeginScope(COMPILERPTR) \
    (++((COMPILERPTR)->localInfo.scopeDepth))

/* Used to have the compiler exit a local scope */
static void unCompilerEndScope(
    UNCompiler *compilerPtr
){
    UNLocalInfo *localInfoPtr
        = &(compilerPtr->localInfo);
    --(localInfoPtr->scopeDepth);

    /*
     * for every local in the scope, pop it off
     * the stack
     */
    while(localInfoPtr->localCount > 0
        && localInfoPtr->locals[
            localInfoPtr->localCount - 1
        ].depth > localInfoPtr->scopeDepth
    ){
        unCompilerWriteByte(compilerPtr, un_pop);
        --(localInfoPtr->localCount);
    }
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
 * Parses a function call for the specified compiler
 * //todo
 */
void unCompilerCall(
    UNCompiler *compilerPtr,
    bool canAssign
){
    //todo call body
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
    UNLocalInfo *localInfoPtr
        = &(compilerPtr->localInfo);
    for(int i = localInfoPtr->localCount - 1;
        i >= 0;
        -- i
    ){
        UNLocal *localPtr = &(localInfoPtr->locals[i]);
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
            return i;
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

    /* if variable was resolved to a local */
    if(arg != -1){
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

    /* if following token is equal, assignment */
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
                &(compilerPtr->compiledProgram.
                    literals.stringMap)
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
    //todo and body
}

/*
 * Parses a boolean OR for the specified compiler
 */
void unCompilerOr(
    UNCompiler *compilerPtr,
    bool canAssign
){
    //todo or body
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

/*
 * Parses a dot for the specified compiler
 */
void unCompilerDot(
    UNCompiler *compilerPtr,
    bool canAssign
){
    //todo dot body
}

/*
 * Performs actions at the end of the compilation
 * process for the specified compiler
 */
static void unCompilerEnd(UNCompiler *compilerPtr){
    //todo: temp emit return
    unCompilerWriteByte(compilerPtr, un_return);
    #ifdef _DEBUG
    if(!(compilerPtr->hadError)){
        unProgramDisassemble(
            &(compilerPtr->compiledProgram)
        );
    }
    #endif
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
 * returns the program; error on compiler error
 */
UNProgram unCompilerCompile(
    UNCompiler *compilerPtr,
    const char *fileName
){
    unCompilerReset(compilerPtr);
    compilerPtr->lexer = unLexerMake(fileName);
    compilerPtr->compiledProgram = unProgramMake();

    unCompilerAdvance(compilerPtr);

    while(!unCompilerMatch(
        compilerPtr,
        un_tokenEOF
    )){
        unCompilerDeclaration(compilerPtr);
    }

    unCompilerEnd(compilerPtr);

    bool hadError = compilerPtr->hadError;

    /*
     * doesn't free the program, just the resources
     * needed during compilation
     */
    unCompilerFree(compilerPtr);
    
    if(hadError){
        pgError(
            "halting due to Unknown compiler error(s)"
        );
    }

    UNProgram toRet = compilerPtr->compiledProgram;
    unCompilerReset(compilerPtr);
    return toRet;
}