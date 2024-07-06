#include "Unknown_Lexer.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h> /* for isdigit */

#include "PGUtil.h"

/*
 * Loads the string contents of the specified file 
 * to the heap and returns a pointer to it; returns
 * NULL on error
 */
static char* loadSourceFile(const char *fileName){
    FILE *filePtr = fopen(fileName, "rb");

    /* bail if failed to open file */
    if(!filePtr){
        pgWarning("failed to open file: ");
        pgWarning(fileName);
        return NULL;
    }

    /* get the size of the file */
    fseek(filePtr, 0, SEEK_END);
    size_t fileSize = ftell(filePtr);
    rewind(filePtr);

    char *buffer = pgAlloc(fileSize + 1, 1);
    size_t bytesRead = fread(
        buffer,
        1,
        fileSize,
        filePtr
    );
    if(bytesRead < fileSize){
        pgFree(buffer);
        pgError("failed to read full bytes in lexer");
        return NULL;
    }
    buffer[bytesRead] = 0;

    fclose(filePtr);
    return buffer;
}

/*
 * Constructs and returns a lexer for the specified
 * file
 */
UNLexer unLexerMake(const char *fileName){
    UNLexer toRet = {0};

    /* load source code into memory */
    toRet.sourcePtr = loadSourceFile(fileName);
    assertNotNull(
        toRet.sourcePtr,
        "error: failed to load Unknown source code; "
        SRC_LOCATION
    );

    toRet.startPtr = toRet.sourcePtr;
    toRet.currentPtr = toRet.sourcePtr;
    toRet.lineNumber = 1;

    return toRet;
}

/*
 * Creates a new token of the specified type based on
 * the current state of the given lexer
 */
#define unLexerMakeToken(LEXERPTR, TYPE) \
    (UNToken){ \
        (TYPE), \
        (LEXERPTR)->startPtr, \
        ((size_t)((LEXERPTR)->currentPtr \
            - (LEXERPTR)->startPtr)), \
        (LEXERPTR)->lineNumber \
    }

/* Creates a new error token */
#define unLexerMakeErrorToken(LEXERPTR, MSG) \
    (UNToken){ \
        un_tokenError, \
        MSG, \
        ((size_t)strlen(MSG)), \
        (LEXERPTR)->lineNumber \
    }

/*
 * Returns true if the specified lexer is at eof,
 * false otherwise
 */
#define unLexerIsAtEnd(LEXERPTR) \
    (*(LEXERPTR->currentPtr) == '\0')

/*
 * Advances the specified lexer to the next token and
 * returns the it
 */
#define unLexerAdvance(LEXERPTR) \
    (*((LEXERPTR)->currentPtr++))

/*
 * Peeks at the next character in the specified lexer
 * but does not advance it
 */
#define unLexerPeek(LEXERPTR) \
    (*((LEXERPTR)->currentPtr))

/*
 * Peeks at the second next character in the specified
 * lexer but does not advance it
 */
#define unLexerPeekNext(LEXERPTR) \
    unLexerIsAtEnd(LEXERPTR) \
        ? '\0' \
        : (((LEXERPTR)->currentPtr[1]))

/*
 * Advances the specified lexer if and only if the
 * next character matches the given character; returns
 * true if match found, false otherwise
 */
static bool unLexerMatch(
    UNLexer *lexerPtr,
    char expected
){
    if(unLexerIsAtEnd(lexerPtr)){
        return false;
    }
    if(*(lexerPtr->currentPtr) != expected){
        return false;
    }
    ++(lexerPtr->currentPtr);
    return true;
}

/*
 * Skips whitespace to the next character for the
 * given lexer
 */
static void unLexerSkipWhitespace(UNLexer *lexerPtr){
    while(true){
        char c = unLexerPeek(lexerPtr);
        switch(c){
            /* skip whitespace characters */
            case ' ':
            case '\r':
            case '\t':
                unLexerAdvance(lexerPtr);
                break;
            /* for new lines, increment line number */
            case '\n':
                ++(lexerPtr->lineNumber);
                unLexerAdvance(lexerPtr);
                break;
            /* skip entire lines of comments */
            case '#':
                /*
                 * go to end of line or file, whichever
                 * comes first
                 */
                while(unLexerPeek(lexerPtr) != '\n'
                    && !unLexerIsAtEnd(lexerPtr)
                ){
                    unLexerAdvance(lexerPtr);
                }
                break;
            default:
                return;
        }
    }
}

/* Reads a string from the specified lexer */
UNToken unLexerMakeString(UNLexer *lexerPtr){
    while(unLexerPeek(lexerPtr) != '"'
        && !unLexerIsAtEnd(lexerPtr)
    ){
        if(unLexerPeek(lexerPtr) == '\n'){
            ++(lexerPtr->lineNumber);
        }
        unLexerAdvance(lexerPtr);
    }
    if(unLexerIsAtEnd(lexerPtr)){
        return unLexerMakeErrorToken(
            lexerPtr,
            "unterminated string"
        );
    }

    /* advance past the closing quote */
    unLexerAdvance(lexerPtr);
    return unLexerMakeToken(lexerPtr, un_tokenString);
}

/*
 * returns true if the given character is a digit 0-9,
 * false otherwise
 */
#define isDigit(CHAR) isdigit(CHAR)

/*
 * returns true if the given character is part of the
 * latin alphabet or is an underscore, false otherwise
 */
#define isAlpha(CHAR) \
    ( \
        (CHAR >= 'a' && CHAR <= 'z') \
        || (CHAR >= 'A' && CHAR <= 'Z') \
        || (CHAR == '_') \
    )


/* Reads a number from the specified lexer */
UNToken unLexerMakeNumber(UNLexer *lexerPtr){
    bool dotPresent = false;

    /* read whole part */
    while(isDigit(unLexerPeek(lexerPtr))){
        unLexerAdvance(lexerPtr);
    }

    /* check for fractional part */
    if(unLexerPeek(lexerPtr) == '.'
        && isDigit(unLexerPeekNext(lexerPtr))
    ){
        dotPresent = true;
        unLexerAdvance(lexerPtr);
        while(isDigit(unLexerPeek(lexerPtr))){
            unLexerAdvance(lexerPtr);
        }
    }

    return unLexerMakeToken(
        lexerPtr,
        dotPresent ? un_tokenFloat : un_tokenInt
    );
}

/*
 * Tests the rest of the token the specified lexer is
 * currently looking at if it matches the given string;
 * returns true if it does, false otherwise
 */
bool unLexerStringMatch(
    UNLexer *lexerPtr,
    size_t start, /* starting character in token */
    size_t length,
    const char *stringToMatch
){
    return (lexerPtr->currentPtr - lexerPtr->startPtr
            == start + length
        && memcmp(
            lexerPtr->startPtr + start,
            stringToMatch,
            length
        ) == 0
    );
}

/*
 * Deduces the type of identifier the specified lexer
 * is currently looking at, i.e. figures out if it
 * is looking at a keyword or not
 */
UNTokenType unLexerDeduceIdentifierType(
    UNLexer *lexerPtr
){
    /* use a DFA/trie to match keywords */
    switch(*(lexerPtr->startPtr)){
        case 'e': return unLexerStringMatch(
            lexerPtr,
            1,
            3,
            "lse"
        ) ? un_tokenElse : un_tokenIdentifier;
        case 'f':
            if(lexerPtr->currentPtr
                - lexerPtr->startPtr > 1
            ){
                switch((lexerPtr->startPtr)[1]){
                    case 'a':
                        return unLexerStringMatch(
                            lexerPtr,
                            2,
                            3,
                            "lse"
                        )
                            ? un_tokenFalse
                            : un_tokenIdentifier;
                    case 'o':
                        return unLexerStringMatch(
                            lexerPtr,
                            2,
                            1,
                            "r"
                        )
                            ? un_tokenFor
                            : un_tokenIdentifier;
                    case 'u':
                        return unLexerStringMatch(
                            lexerPtr,
                            2,
                            2,
                            "nc"
                        )
                            ? un_tokenFunc
                            : un_tokenIdentifier;
                } /* end 'f' branch */
            }
            break;
        case 'i': return unLexerStringMatch(
            lexerPtr,
            1,
            1,
            "f"
        ) ? un_tokenIf : un_tokenIdentifier;
        case 'l': return unLexerStringMatch(
            lexerPtr,
            1,
            2,
            "et"
        ) ? un_tokenLet : un_tokenIdentifier;
        case 'p': return unLexerStringMatch(
            lexerPtr,
            1,
            4,
            "rint"
        ) ? un_tokenPrint : un_tokenIdentifier;
        case 'r': return unLexerStringMatch(
            lexerPtr,
            1,
            5,
            "eturn"
        ) ? un_tokenReturn : un_tokenIdentifier;
        case 't': return unLexerStringMatch(
            lexerPtr,
            1,
            3,
            "rue"
        ) ? un_tokenTrue : un_tokenIdentifier;
        case 'w': return unLexerStringMatch(
            lexerPtr,
            1,
            4,
            "hile"
        ) ? un_tokenWhile : un_tokenIdentifier;
    }
    /* otherwise, token is just a normal identifier */
    return un_tokenIdentifier;
}

/* Reads an identifier from the specified lexer */
UNToken unLexerMakeIdentifier(UNLexer *lexerPtr){
    while(isAlpha(unLexerPeek(lexerPtr))
        || isDigit(unLexerPeek(lexerPtr))
    ){
        unLexerAdvance(lexerPtr);
    }
    return unLexerMakeToken(
        lexerPtr,
        unLexerDeduceIdentifierType(lexerPtr)
    );
}

/* Gets the next token from the specified lexer */
UNToken unLexerNext(UNLexer *lexerPtr){
    unLexerSkipWhitespace(lexerPtr);
    lexerPtr->startPtr = lexerPtr->currentPtr;

    /* if at end, return eof token */
    if(unLexerIsAtEnd(lexerPtr)){
        return unLexerMakeToken(lexerPtr, un_tokenEOF);
    }

    char c = unLexerAdvance(lexerPtr);
    if(isAlpha(c)){
        return unLexerMakeIdentifier(lexerPtr);
    }
    if(isDigit(c)){
        return unLexerMakeNumber(lexerPtr);
    }
    switch(c){
        case '(': return unLexerMakeToken(
            lexerPtr,
            un_tokenLeftParen
        );
        case ')': return unLexerMakeToken(
            lexerPtr,
            un_tokenRightParen
        );
        case '{': return unLexerMakeToken(
            lexerPtr,
            un_tokenLeftBrace
        );
        case '}': return unLexerMakeToken(
            lexerPtr,
            un_tokenRightBrace
        );
        case ';': return unLexerMakeToken(
            lexerPtr,
            un_tokenSemicolon
        );
        case ',': return unLexerMakeToken(
            lexerPtr,
            un_tokenComma
        );
        case '.': return unLexerMakeToken(
            lexerPtr,
            un_tokenDot
        );
        case '-': return unLexerMakeToken(
            lexerPtr,
            un_tokenMinus
        );
        case '%': return unLexerMakeToken(
            lexerPtr,
            un_tokenPercent
        );
        case '+': return unLexerMakeToken(
            lexerPtr,
            un_tokenPlus
        );
        case '/': return unLexerMakeToken(
            lexerPtr,
            un_tokenSlash
        );
        case '*': return unLexerMakeToken(
            lexerPtr,
            un_tokenStar
        );
        case '&': return unLexerMakeToken(
            lexerPtr,
            un_tokenAmpersand
        );
        case '|': return unLexerMakeToken(
            lexerPtr,
            un_tokenVerticalBar
        );
        case '!': return unLexerMakeToken(
            lexerPtr,
            unLexerMatch(lexerPtr, '=')
                ? un_tokenBangEqual
                : un_tokenBang
        );
        case '=': return unLexerMakeToken(
            lexerPtr,
            unLexerMatch(lexerPtr, '=')
                ? un_tokenDoubleEqual
                : un_tokenEqual
        );
        case '<': return unLexerMakeToken(
            lexerPtr,
            unLexerMatch(lexerPtr, '=')
                ? un_tokenLessEqual
                : un_tokenLess
        );
        case '>': return unLexerMakeToken(
            lexerPtr,
            unLexerMatch(lexerPtr, '=')
                ? un_tokenGreaterEqual
                : un_tokenGreater
        );
        case '"': return unLexerMakeString(lexerPtr);
    }

    return unLexerMakeErrorToken(lexerPtr, "bad char");
}

/*
 * Frees the memory associated with the specified
 * lexer
 */
void unLexerFree(UNLexer *lexerPtr){
    if(lexerPtr && lexerPtr->sourcePtr){
        if(lexerPtr->sourcePtr){
            pgFree(lexerPtr->sourcePtr);
        }
        memset(lexerPtr, 0, sizeof(*lexerPtr));
    }
}