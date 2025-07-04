#include "Necro_Lexer.h"

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
NecroLexer necroLexerMake(const char *fileName){
    NecroLexer toRet = {0};

    /* load source code into memory */
    toRet.sourcePtr = loadSourceFile(fileName);
    assertNotNull(
        toRet.sourcePtr,
        "error: failed to load Necro source code; "
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
#define necroLexerMakeToken(lexerPtr, tokenType) \
    (NecroToken){ \
        (tokenType), \
        (lexerPtr)->startPtr, \
        ((size_t)((lexerPtr)->currentPtr \
            - (lexerPtr)->startPtr)), \
        (lexerPtr)->lineNumber \
    }

/* Creates a new error token */
#define necroLexerMakeErrorToken(lexerPtr, errorMsg) \
    (NecroToken){ \
        necro_tokenError, \
        errorMsg, \
        ((size_t)strlen(errorMsg)), \
        (lexerPtr)->lineNumber \
    }

/*
 * Returns true if the specified lexer is at eof,
 * false otherwise
 */
#define necroLexerIsAtEnd(lexerPtr) \
    (*((lexerPtr)->currentPtr) == '\0')

/*
 * Advances the specified lexer to the next 
 * character and returns the it
 */
#define necroLexerAdvance(lexerPtr) \
    (*((lexerPtr)->currentPtr++))

/*
 * Peeks at the next character in the specified lexer
 * but does not advance it
 */
#define necroLexerPeek(lexerPtr) \
    (*((lexerPtr)->currentPtr))

/*
 * Peeks at the second next character in the specified
 * lexer but does not advance it
 */
#define necroLexerPeekNext(lexerPtr) \
    necroLexerIsAtEnd(lexerPtr) \
        ? '\0' \
        : (((lexerPtr)->currentPtr[1]))

/*
 * Advances the specified lexer if and only if the
 * next character matches the given character; returns
 * true if match found, false otherwise
 */
static bool necroLexerMatch(
    NecroLexer *lexerPtr,
    char expected
){
    if(necroLexerIsAtEnd(lexerPtr)){
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
static void necroLexerSkipWhitespace(NecroLexer *lexerPtr){
    while(true){
        char c = necroLexerPeek(lexerPtr);
        switch(c){
            /* skip whitespace characters */
            case ' ':
            case '\r':
            case '\t':
                necroLexerAdvance(lexerPtr);
                break;
            /* for new lines, increment line number */
            case '\n':
                ++(lexerPtr->lineNumber);
                necroLexerAdvance(lexerPtr);
                break;
            /* skip entire lines of comments */
            case '#':
                /*
                 * go to end of line or file, whichever
                 * comes first
                 */
                while(necroLexerPeek(lexerPtr) != '\n'
                    && !necroLexerIsAtEnd(lexerPtr)
                ){
                    necroLexerAdvance(lexerPtr);
                }
                break;
            /* skip multiline comments */
            case '~': {
                necroLexerAdvance(lexerPtr);
                bool inMultilineComment = true;
                /*
                 * go to next ~ or file, whichever
                 * comes first; eof is not an error
                 */
                while(inMultilineComment
                    && !necroLexerIsAtEnd(lexerPtr)
                ){
                    switch(necroLexerPeek(lexerPtr)){
                        /* handle newline in comment */
                        case '\n':
                            ++(lexerPtr->lineNumber);
                            necroLexerAdvance(
                                lexerPtr
                            );
                            break;
                        case '~':
                            necroLexerAdvance(
                                lexerPtr
                            );
                        case '\0':
                            inMultilineComment = false;
                            break;
                        /* skip all other chars */
                        default:
                            necroLexerAdvance(
                                lexerPtr
                            );
                    }
                }
                break;
            }
            default:
                return;
        }
    }
}

/* Reads a string from the specified lexer */
NecroToken necroLexerMakeString(NecroLexer *lexerPtr){
    while(necroLexerPeek(lexerPtr) != '"'
        && !necroLexerIsAtEnd(lexerPtr)
    ){
        if(necroLexerPeek(lexerPtr) == '\n'){
            ++(lexerPtr->lineNumber);
        }
        necroLexerAdvance(lexerPtr);
    }
    if(necroLexerIsAtEnd(lexerPtr)){
        return necroLexerMakeErrorToken(
            lexerPtr,
            "unterminated string"
        );
    }

    /* advance past the closing quote */
    necroLexerAdvance(lexerPtr);
    return necroLexerMakeToken(
        lexerPtr,
        necro_tokenString
    );
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
NecroToken necroLexerMakeNumber(NecroLexer *lexerPtr){
    bool dotPresent = false;

    /* read whole part */
    while(isDigit(necroLexerPeek(lexerPtr))){
        necroLexerAdvance(lexerPtr);
    }

    /* check for fractional part */
    if(necroLexerPeek(lexerPtr) == '.'
        && isDigit(necroLexerPeekNext(lexerPtr))
    ){
        dotPresent = true;
        necroLexerAdvance(lexerPtr);
        while(isDigit(necroLexerPeek(lexerPtr))){
            necroLexerAdvance(lexerPtr);
        }
    }

    return necroLexerMakeToken(
        lexerPtr,
        dotPresent ? necro_tokenFloat : necro_tokenInt
    );
}

/*
 * Tests the rest of the token the specified lexer is
 * currently looking at if it matches the given string;
 * returns true if it does, false otherwise
 */
bool necroLexerStringMatch(
    NecroLexer *lexerPtr,
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
NecroTokenType necroLexerDeduceIdentifierType(
    NecroLexer *lexerPtr
){
    /* use a DFA/trie to match keywords */
    switch(*(lexerPtr->startPtr)){
        case 'e': return necroLexerStringMatch(
            lexerPtr,
            1,
            3,
            "lse"
        ) ? necro_tokenElse : necro_tokenIdentifier;
        case 'f':
            if(lexerPtr->currentPtr
                - lexerPtr->startPtr > 1
            ){
                switch((lexerPtr->startPtr)[1]){
                    case 'a':
                        return necroLexerStringMatch(
                            lexerPtr,
                            2,
                            3,
                            "lse"
                        )
                            ? necro_tokenFalse
                            : necro_tokenIdentifier;
                    case 'o':
                        return necroLexerStringMatch(
                            lexerPtr,
                            2,
                            1,
                            "r"
                        )
                            ? necro_tokenFor
                            : necro_tokenIdentifier;
                } /* end 'f' branch */
            }
            break;
        case 'i':
            if(lexerPtr->currentPtr
                - lexerPtr->startPtr > 1
            ){
                switch((lexerPtr->startPtr)[1]){
                    case 'f':
                        return necroLexerStringMatch(
                            lexerPtr,
                            2,
                            0,
                            ""
                        )
                            ? necro_tokenIf
                            : necro_tokenIdentifier;
                    case 'n':
                        return necroLexerStringMatch(
                            lexerPtr,
                            2,
                            5,
                            "clude"
                        )
                            ? necro_tokenInclude
                            : necro_tokenIdentifier;
                } /* end of 'i' branch */
            }
            break;
        case 'l': return necroLexerStringMatch(
            lexerPtr,
            1,
            2,
            "et"
        ) ? necro_tokenLet : necro_tokenIdentifier;
        case 'p': return necroLexerStringMatch(
            lexerPtr,
            1,
            4,
            "rint"
        ) ? necro_tokenPrint : necro_tokenIdentifier;
        case 'r': return necroLexerStringMatch(
            lexerPtr,
            1,
            5,
            "eturn"
        ) ? necro_tokenReturn : necro_tokenIdentifier;
        case 't': return necroLexerStringMatch(
            lexerPtr,
            1,
            3,
            "rue"
        ) ? necro_tokenTrue : necro_tokenIdentifier;
        case 'v': return necroLexerStringMatch(
            lexerPtr,
            1,
            2,
            "ar"
        ) ? necro_tokenVar : necro_tokenIdentifier;
        case 'w':
            if(lexerPtr->currentPtr
                - lexerPtr->startPtr > 1
            ){
                switch((lexerPtr->startPtr)[1]){
                    case 'a':
                        return necroLexerStringMatch(
                            lexerPtr,
                            2,
                            2,
                            "it"
                        )
                            ? necro_tokenWait
                            : necro_tokenIdentifier;
                    case 'h':
                        return necroLexerStringMatch(
                            lexerPtr,
                            2,
                            3,
                            "ile"
                        )
                            ? necro_tokenWhile
                            : necro_tokenIdentifier;
                } /* end 'w' branch */
            }
            break;
        case 'y': return necroLexerStringMatch(
            lexerPtr,
            1,
            4,
            "ield"
        ) ? necro_tokenYield : necro_tokenIdentifier;
    }
    /* otherwise, token is just a normal identifier */
    return necro_tokenIdentifier;
}

/* Reads an identifier from the specified lexer */
NecroToken necroLexerMakeIdentifier(NecroLexer *lexerPtr){
    while(isAlpha(necroLexerPeek(lexerPtr))
        || isDigit(necroLexerPeek(lexerPtr))
    ){
        necroLexerAdvance(lexerPtr);
    }
    return necroLexerMakeToken(
        lexerPtr,
        necroLexerDeduceIdentifierType(lexerPtr)
    );
}

/* Gets the next token from the specified lexer */
NecroToken necroLexerNext(NecroLexer *lexerPtr){
    necroLexerSkipWhitespace(lexerPtr);
    lexerPtr->startPtr = lexerPtr->currentPtr;

    /* if at end, return eof token */
    if(necroLexerIsAtEnd(lexerPtr)){
        return necroLexerMakeToken(lexerPtr, necro_tokenEOF);
    }

    char c = necroLexerAdvance(lexerPtr);
    if(isAlpha(c)){
        return necroLexerMakeIdentifier(lexerPtr);
    }
    if(isDigit(c)){
        return necroLexerMakeNumber(lexerPtr);
    }
    switch(c){
        case '(': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenLeftParen
        );
        case ')': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenRightParen
        );
        case '{': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenLeftBrace
        );
        case '}': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenRightBrace
        );
        case ';': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenSemicolon
        );
        case ',': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenComma
        );
        case '.': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenDot
        );
        case '-': {
            NecroTokenType type = necro_tokenMinus;
            if(necroLexerMatch(lexerPtr, '=')){
                type = necro_tokenMinusEqual;
            }
            if(necroLexerMatch(lexerPtr, '>')){
                type = necro_tokenMinusGreater;
            }
            return necroLexerMakeToken(lexerPtr, type);
        }
        case '%': {
            NecroTokenType type = necro_tokenPercent;
            if(necroLexerMatch(lexerPtr, '=')){
                type = necro_tokenPercentEqual;
            }
            return necroLexerMakeToken(lexerPtr, type);
        }
        case '+': {
            NecroTokenType type = necro_tokenPlus;
            if(necroLexerMatch(lexerPtr, '=')){
                type = necro_tokenPlusEqual;
            }
            return necroLexerMakeToken(lexerPtr, type);
        }
        case '/': {
            NecroTokenType type = necro_tokenSlash;
            if(necroLexerMatch(lexerPtr, '=')){
                type = necro_tokenSlashEqual;
            }
            return necroLexerMakeToken(lexerPtr, type);
        }
        case '*': {
            NecroTokenType type = necro_tokenStar;
            if(necroLexerMatch(lexerPtr, '=')){
                type = necro_tokenStarEqual;
            }
            return necroLexerMakeToken(lexerPtr, type);
        }
        case '\\': return necroLexerMakeToken(
            lexerPtr,
            necro_tokenBackSlash
        );
        case '!': return necroLexerMakeToken(
            lexerPtr,
            necroLexerMatch(lexerPtr, '=')
                ? necro_tokenBangEqual
                : necro_tokenBang
        );
        case '=': 
            /* double equal for equality */
            if(necroLexerMatch(lexerPtr, '=')){
                return necroLexerMakeToken(
                    lexerPtr,
                    necro_tokenDoubleEqual
                );
            }
            /* single equal is lexical error */
            break;
        case '<': {
            NecroTokenType type = necro_tokenLess;
            if(necroLexerMatch(lexerPtr, '=')){
                type = necro_tokenLessEqual;
            }
            else if(necroLexerMatch(lexerPtr, '<')){
                type = necro_tokenDoubleLess;
            }
            return necroLexerMakeToken(lexerPtr, type);
        }
        case '>': {
            NecroTokenType type = necro_tokenGreater;
            if(necroLexerMatch(lexerPtr, '=')){
                type = necro_tokenGreaterEqual;
            }
            else if(necroLexerMatch(lexerPtr, '>')){
                type = necro_tokenDoubleGreater;
            }
            return necroLexerMakeToken(lexerPtr, type);
        }
        case '[': 
            /* double left bracket for points */
            if(necroLexerMatch(lexerPtr, '[')){
                return necroLexerMakeToken(
                    lexerPtr,
                    necro_tokenDoubleLeftBracket
                );
            }
            /* single left bracket is lexical error */
            break;
        case ']': 
            /* double right bracket for points */
            if(necroLexerMatch(lexerPtr, ']')){
                return necroLexerMakeToken(
                    lexerPtr,
                    necro_tokenDoubleRightBracket
                );
            }
            /* single right bracket is lexical error */
            break;
        case '&': 
            /* double ampersand for logical and */
            if(necroLexerMatch(lexerPtr, '&')){
                return necroLexerMakeToken(
                    lexerPtr,
                    necro_tokenDoubleAmpersand
                );
            }
            /* single ampersand is lexical error */
            break;
        case '|': 
            /* double vertical bar for logical or */
            if(necroLexerMatch(lexerPtr, '|')){
                return necroLexerMakeToken(
                    lexerPtr,
                    necro_tokenDoubleVerticalBar
                );
            }
            /* single vertical bar is lexical error */
            break;
        case ':': 
            /* colon equal for assignment */
            if(necroLexerMatch(lexerPtr, '=')){
                return necroLexerMakeToken(
                    lexerPtr,
                    necro_tokenColonEqual
                );
            }
            /* single colon is lexical error */
            break;
        case '"': return necroLexerMakeString(lexerPtr);
    }

    return necroLexerMakeErrorToken(lexerPtr, "bad char");
}

/*
 * Frees the memory associated with the specified
 * lexer
 */
void necroLexerFree(NecroLexer *lexerPtr){
    if(lexerPtr && lexerPtr->sourcePtr){
        if(lexerPtr->sourcePtr){
            pgFree(lexerPtr->sourcePtr);
        }
        memset(lexerPtr, 0, sizeof(*lexerPtr));
    }
}