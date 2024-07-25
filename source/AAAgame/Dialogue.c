#include "Dialogue.h"

/* used to help parse dialogue files */
typedef struct DialogueLexer{
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
} DialogueLexer;

/*
 * Loads the string contents of the specified file 
 * to the heap and returns a pointer to it; returns
 * NULL on error
 */
static char* loadDialogueFile(const char *fileName){
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
        pgError(
            "failed to read full bytes in dialogue "
            "lexer"
        );
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
static DialogueLexer dialogueLexerMake(
    const char *fileName
){
    DialogueLexer toRet = {0};

    /* load dialogue file into memory */
    toRet.sourcePtr = loadDialogueFile(fileName);
    assertNotNull(
        toRet.sourcePtr,
        "error: failed to load dialogue file; "
        SRC_LOCATION
    );

    toRet.startPtr = toRet.sourcePtr;
    toRet.currentPtr = toRet.sourcePtr;
    toRet.lineNumber = 1;

    return toRet;
}

/*
 * Returns true if the specified lexer is at eof,
 * false otherwise
 */
#define dialogueLexerIsAtEnd(LEXERPTR) \
    (*((LEXERPTR)->currentPtr) == '\0')

/*
 * Advances the specified lexer to the next 
 * character and returns the it
 */
#define dialogueLexerAdvance(LEXERPTR) \
    (*((LEXERPTR)->currentPtr++))

/*
 * Peeks at the next character in the specified lexer
 * but does not advance it
 */
#define dialogueLexerPeek(LEXERPTR) \
    (*((LEXERPTR)->currentPtr))

/*
 * Returns true if the specified character counts as
 * whitespace, false otherwise
 */
static bool isWhitespace(char c){
    switch(c){
        case ' ':
        case '\r':
        case '\t':
            return true;
        default:
            return false;
    }
}

/*
 * Returns true if the specified character counts as
 * whitespace including newlines, false otherwise
 */
static bool isWhitespaceIncludingNewline(char c){
    switch(c){
        case ' ':
        case '\r':
        case '\t':
        case '\n':
            return true;
        default:
            return false;
    }
}

/*
 * Skips whitespace to the next character for the
 * given lexer
 */
static void dialogueLexerSkipWhitespace(
    DialogueLexer *lexerPtr
){
    while(isWhitespace(dialogueLexerPeek(lexerPtr))){
        dialogueLexerAdvance(lexerPtr);
    }
}

/*
 * Tests the rest of the token the specified lexer is
 * currently looking at if it matches the given string;
 * returns true if it does, false otherwise
 */
bool dialogueLexerStringMatch(
    DialogueLexer *lexerPtr,
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
 * Deduces the dialogue command the specified lexer is
 * currently looking at; dialogue_error if invalid
 */
DialogueCommand dialogueLexerDeduceCommand(
    DialogueLexer *lexerPtr
){
    /* use a DFA/trie to match keywords */
    switch(*(lexerPtr->startPtr)){
        case 'l': return dialogueLexerStringMatch(
            lexerPtr,
            1,
            9,
            "eft_image"
        ) ? dialogue_setLeftImage : dialogue_error;
        case 'm': return dialogueLexerStringMatch(
            lexerPtr,
            1,
            4,
            "usic"
        ) ? dialogue_setTrack : dialogue_error;
        case 'r': return dialogueLexerStringMatch(
            lexerPtr,
            1,
            10,
            "ight_image"
        ) ? dialogue_setRightImage : dialogue_error;
        case 's': return dialogueLexerStringMatch(
            lexerPtr,
            1,
            3,
            "top"
        ) ? dialogue_stop : dialogue_error;
        case 't': return dialogueLexerStringMatch(
            lexerPtr,
            1,
            3,
            "ext"
        ) ? dialogue_setText : dialogue_error;
        default:
            return dialogue_error;
    }
}

/*
 * Copies the current string that the specified lexer
 * is currently looking at, trimming white space from
 * the end
 */
String dialogueLexerCopyString(
    DialogueLexer *lexerPtr
){
    /*
     * special case: if current ptr is null terminator,
     * copy the whole string from start
     */
    if(*(lexerPtr->currentPtr) == '\0'){
        return stringMakeC(lexerPtr->startPtr);
    }

    char *lastCharPtr = lexerPtr->currentPtr - 1;
    while(lastCharPtr + 1 != lexerPtr->startPtr){
        if(isWhitespace(*lastCharPtr)){
            --lastCharPtr;
        }
        else{
            break;
        }
    }

    /*
     * special case: all white space, don't even
     * allocate a string
     */
    if(lastCharPtr + 1 == lexerPtr->startPtr){
        return ((String){0});
    }

    /* place a temp null terminator to make the string
     * end */
    char *lastWhitespacePtr = lastCharPtr + 1;
    char replacedChar = *lastWhitespacePtr;
    *lastWhitespacePtr = '\0';

    String toRet = stringMakeC(lexerPtr->startPtr);

    *lastWhitespacePtr = replacedChar;

    return toRet;
}

/*
 * Parses the next instruction using the specified
 * dialogue lexer
 */
static DialogueInstruction dialogueLexerNext(
    DialogueLexer *lexerPtr
){
    DialogueInstruction toRet = {0};

    dialogueLexerSkipWhitespace(lexerPtr);
    while(*(lexerPtr->currentPtr) == '\n'){
        ++(lexerPtr->lineNumber);
        dialogueLexerAdvance(lexerPtr);
        dialogueLexerSkipWhitespace(lexerPtr);
    }
    lexerPtr->startPtr = lexerPtr->currentPtr;

    /* if at end, return end command */
    if(dialogueLexerIsAtEnd(lexerPtr)){
        toRet.command = dialogue_end;
        return toRet;
    }

    /* grab the first "token" */
    while(!dialogueLexerIsAtEnd(lexerPtr)
        && !isWhitespaceIncludingNewline(
            dialogueLexerPeek(lexerPtr)
        )
    ){
        dialogueLexerAdvance(lexerPtr);
    }
    /* use first "token" as command */
    toRet.command = dialogueLexerDeduceCommand(
        lexerPtr
    );

    dialogueLexerSkipWhitespace(lexerPtr);
    lexerPtr->startPtr = lexerPtr->currentPtr;
    if(*(lexerPtr->currentPtr) != '\n'){
        while(!dialogueLexerIsAtEnd(lexerPtr)
            && dialogueLexerPeek(lexerPtr) != '\n'
        ){
            dialogueLexerAdvance(lexerPtr);
        }
        toRet.data = dialogueLexerCopyString(lexerPtr);
    }
    if(*(lexerPtr->currentPtr) == '\n'){
        ++(lexerPtr->lineNumber);
        dialogueLexerAdvance(lexerPtr);
    }

    return toRet;
}

/*
 * Frees the memory associated with the specified
 * dialogue lexer
 */
static void dialogueLexerFree(DialogueLexer *lexerPtr){
    if(lexerPtr && lexerPtr->sourcePtr){
        if(lexerPtr->sourcePtr){
            pgFree(lexerPtr->sourcePtr);
        }
        memset(lexerPtr, 0, sizeof(*lexerPtr));
    }
}

/*
 * Frees the memory associated with the specified
 * dialogue instruction
 */
void dialogueInstructionFree(
    DialogueInstruction *instrPtr
){
    if(!instrPtr){
        return;
    }
    if(instrPtr->data._ptr){
        stringFree(&(instrPtr->data));
    }
    memset(instrPtr, 0, sizeof(*instrPtr));
}

/*
 * Constructs and returns a new empty dialogue by value
 */
static Dialogue dialogueMake(){
    Dialogue toRet = {0};
    toRet.instructionList = arrayListMake(
        DialogueInstruction,
        10
    );
    return toRet;
}

/*
 * Reads the specified dialogue file and returns a
 * Dialogue by value, ready to be used
 */
Dialogue parseDialogueFile(const char *fileName){
    DialogueLexer lexer = dialogueLexerMake(fileName);

    Dialogue toRet = dialogueMake();

    while(!dialogueLexerIsAtEnd(&lexer)){
        arrayListPushBack(DialogueInstruction,
            &(toRet.instructionList),
            dialogueLexerNext(&lexer)
        );
    }
    /* add the end instruction afterwards */
    arrayListPushBack(DialogueInstruction,
        &(toRet.instructionList),
        ((DialogueInstruction){dialogue_end})
    );

    dialogueLexerFree(&lexer);

    return toRet;
}

/*
 * Frees the memory associated with the specified
 * dialogue
 */
void dialogueFree(Dialogue *dialoguePtr){
    if(!dialoguePtr){
        return;
    }

    if(!arrayListIsEmpty(
        &(dialoguePtr->instructionList)
    )){
        arrayApply(DialogueInstruction,
            &(dialoguePtr->instructionList),
            dialogueInstructionFree
        );
    }
    arrayListFree(DialogueInstruction,
        &(dialoguePtr->instructionList)
    );

    memset(dialoguePtr, 0, sizeof(*dialoguePtr));
}