#ifndef NECRO_LEXER_H
#define NECRO_LEXER_H

#include <stddef.h>

//todo: update tokens for necro
/* the types of tokens for the Necro language */
typedef enum NecroTokenType{
    /* single character tokens */
    necro_tokenLeftParen,
    necro_tokenRightParen,
    necro_tokenLeftBrace,
    necro_tokenRightBrace,
    necro_tokenLeftBracket,
    necro_tokenRightBracket,
    necro_tokenComma,
    necro_tokenDot,
    necro_tokenMinus,
    necro_tokenPercent,
    necro_tokenPlus,
    necro_tokenSemicolon,
    necro_tokenSlash,
    necro_tokenStar,
    necro_tokenAmpersand,
    necro_tokenVerticalBar,

    /* one or two character tokens */
    necro_tokenBang,
    necro_tokenBangEqual,
    necro_tokenEqual,
    necro_tokenDoubleEqual,
    necro_tokenGreater,
    necro_tokenDoubleGreater,
    necro_tokenGreaterEqual,
    necro_tokenLess,
    necro_tokenDoubleLess,
    necro_tokenLessEqual,

    /* literals */
    necro_tokenIdentifier,
    necro_tokenInt,
    necro_tokenFloat,
    necro_tokenString,

    /* keywords */
    necro_tokenElse,
    necro_tokenFalse,
    necro_tokenFor,
    necro_tokenFunc,
    necro_tokenIf,
    necro_tokenLet,
    necro_tokenReturn,
    necro_tokenTrue,
    necro_tokenWait,
    necro_tokenWhile,
    necro_tokenYield,

    /* useful for printing not just strings */
    necro_tokenPrint,

    /* meta */
    necro_tokenError,
    necro_tokenEOF
} NecroTokenType;

/* represents a single Necro token */
typedef struct NecroToken{
    NecroTokenType type;
    /*
     * pointer to the start of the token in the source
     * code
     */
    const char *startPtr;
    size_t length;
    size_t lineNumber;
} NecroToken;

/* used to generate tokens from a source file */
typedef struct NecroLexer{
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
} NecroLexer;

/*
 * Constructs and returns a lexer for the specified
 * file
 */
NecroLexer necroLexerMake(const char *fileName);

/* Gets the next token from the specified lexer */
NecroLexer necroLexerNext(NecroLexer *lexerPtr);

/*
 * Frees the memory associated with the specified
 * lexer
 */
void necroLexerFree(NecroLexer *lexerPtr);

#endif