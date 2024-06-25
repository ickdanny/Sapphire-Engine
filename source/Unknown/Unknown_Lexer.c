#include "Unknown_Lexer.h"

#include <stdio.h>

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

/* Gets the next token from the specified lexer */
UNToken unLexerNext(UNLexer *lexerPtr){
    //todo
    lexerPtr->startPtr = lexerPtr->currentPtr;

    //if at end, return eof token
    if(*(lexerPtr->currentPtr) == '\0'){
        return eofToken;
    }

    return errorToken;
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