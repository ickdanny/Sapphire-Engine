#ifndef NECRO_TOKEN_H
#define NECRO_TOKEN_H

#include <stddef.h>

/* the types of tokens for the Necro language */
typedef enum NecroTokenType{
    /* single character tokens */
    necro_tokenLeftParen,
    necro_tokenRightParen,
    necro_tokenLeftBrace,
    necro_tokenRightBrace,
    necro_tokenComma,
    necro_tokenDot,
    necro_tokenMinus,
    necro_tokenPercent,
    necro_tokenPlus,
    necro_tokenSemicolon,
    necro_tokenSlash,
    necro_tokenBackSlash,
    necro_tokenStar,

    /* one or two character tokens */
    necro_tokenBang,
    necro_tokenBangEqual,
    necro_tokenDoubleEqual,
    necro_tokenGreater,
    necro_tokenDoubleGreater,
    necro_tokenGreaterEqual,
    necro_tokenLess,
    necro_tokenDoubleLess,
    necro_tokenLessEqual,
    necro_tokenDoubleLeftBracket,
    necro_tokenDoubleRightBracket,
    necro_tokenDoubleAmpersand,
    necro_tokenDoubleVerticalBar,
    necro_tokenColonEqual,
    necro_tokenPlusEqual,
    necro_tokenMinusEqual,
    necro_tokenStarEqual,
    necro_tokenSlashEqual,
    necro_tokenPercentEqual,
    necro_tokenMinusGreater,

    /* literals */
    necro_tokenIdentifier,
    necro_tokenInt,
    necro_tokenFloat,
    necro_tokenString,

    /* keywords */
    necro_tokenElse,
    necro_tokenFalse,
    necro_tokenFor,
    necro_tokenIf,
    necro_tokenInclude,
    necro_tokenLet,
    necro_tokenPrint, /* prints non-strings too */
    necro_tokenReturn,
    necro_tokenTrue,
    necro_tokenVar,
    necro_tokenWait,
    necro_tokenWhile,
    necro_tokenYield,    

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

#endif