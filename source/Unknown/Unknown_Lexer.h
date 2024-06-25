#ifndef UNKNOWN_LEXER_H
#define UNKNOWN_LEXER_H

#include <stddef.h>

/* the types of tokens for the Unknown language */
typedef enum UNTokenType{
    /* single character tokens */
    un_leftParen,
    un_rightParen,
    un_leftBrace,
    un_rightBrace,
    un_comma,
    un_dot,
    un_minus,
    un_plus,
    un_semicolon,
    un_slash,
    un_star,
    un_ampersand,
    un_verticalBar,

    /* one or two character tokens */
    un_bang,
    un_bangEqual,
    un_equal,
    un_doubleEqual,
    un_greater,
    un_greaterEqual,
    un_less,
    un_lessEqual,

    /* literals */
    un_identifier,
    un_string,
    un_number,

    /* keywords */
    un_else,
    un_false,
    un_for,
    un_func,
    un_if,
    un_let,
    un_return,
    un_true,
    un_while,

    un_print, //todo: temp print token for debugging

    /* meta */
    un_tokenError,
    un_eof
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