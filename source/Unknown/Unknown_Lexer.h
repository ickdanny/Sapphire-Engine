#ifndef UNKNOWN_LEXER_H
#define UNKNOWN_LEXER_H

#include <stddef.h>

/* the types of tokens for the Unknown language */
typedef enum UNTokenType{
    /* single character tokens */
    un_tokenLeftParen,
    un_tokenRightParen,
    un_tokenLeftBrace,
    un_tokenRightBrace,
    un_tokenLeftBracket,
    un_tokenRightBracket,
    un_tokenComma,
    un_tokenDot,
    un_tokenMinus,
    un_tokenPercent,
    un_tokenPlus,
    un_tokenSemicolon,
    un_tokenSlash,
    un_tokenStar,
    un_tokenAmpersand,
    un_tokenVerticalBar,

    /* one or two character tokens */
    un_tokenBang,
    un_tokenBangEqual,
    un_tokenEqual,
    un_tokenDoubleEqual,
    un_tokenGreater,
    un_tokenDoubleGreater,
    un_tokenGreaterEqual,
    un_tokenLess,
    un_tokenDoubleLess,
    un_tokenLessEqual,

    /* literals */
    un_tokenIdentifier,
    un_tokenInt,
    un_tokenFloat,
    un_tokenString,

    /* keywords */
    un_tokenElse,
    un_tokenFalse,
    un_tokenFor,
    un_tokenFunc,
    un_tokenIf,
    un_tokenLet,
    un_tokenReturn,
    un_tokenTrue,
    un_tokenWait,
    un_tokenWhile,
    un_tokenYield,

    /* useful for printing not just strings */
    un_tokenPrint,

    /* meta */
    un_tokenError,
    un_tokenEOF
} UNTokenType;

/* represents a single Unknown token */
typedef struct UNToken{
    UNTokenType type;
    /*
     * pointer to the start of the token in the source
     * code
     */
    const char *startPtr;
    size_t length;
    size_t lineNumber;
} UNToken;

/* used to generate tokens from a source file */
typedef struct UNLexer{
    /*
     * pointer to the whole heap block containing
     * the source code; owned by the lexer
     */
    char *sourcePtr;
    /* points to start of current token */
    char *startPtr;
    /* points to current character being scanned */
    char *currentPtr;
    /* line number of current token */
    size_t lineNumber;
} UNLexer;

/*
 * Constructs and returns a lexer for the specified
 * file
 */
UNLexer unLexerMake(const char *fileName);

/* Gets the next token from the specified lexer */
UNToken unLexerNext(UNLexer *lexerPtr);

/*
 * Frees the memory associated with the specified
 * lexer
 */
void unLexerFree(UNLexer *lexerPtr);

#endif