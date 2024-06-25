#include "Unknown_Compiler.h"

#include "Unknown_Lexer.h"

#include <stdbool.h>

/* compiles the specified Unknown source file */
void unCompile(const char *fileName){
    UNLexer lexer = unLexerMake(fileName);

    size_t lineNumber = ~((size_t)0);
    while(true){
        UNToken token = unLexerNext(&lexer);
        if(token.lineNumber != lineNumber){
            printf("%4lu ", token.lineNumber);
            lineNumber = token.lineNumber;
        }
        else{
            printf("   | ");
        }
        printf(
            "%2d '%.*s'\n",
            token.type,
            token.length,
            token.start
        );
        if(token.type == un_eof){
            break;
        }
    }

    unLexerFree(&lexer);
}