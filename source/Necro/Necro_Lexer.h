#ifndef NECRO_LEXER_H
#define NECRO_LEXER_H

#include "Necro_Token.h"

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
NecroToken necroLexerNext(NecroLexer *lexerPtr);

/*
 * Frees the memory associated with the specified
 * lexer
 */
void necroLexerFree(NecroLexer *lexerPtr);

#endif