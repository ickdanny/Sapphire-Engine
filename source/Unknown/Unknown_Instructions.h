#ifndef UNKNOWN_INSTRUCTIONS
#define UNKNOWN_INSTRUCTIONS

typedef enum UNInstruction {

    /*
     * instructs VM to load a literal value;
     * FORMAT: [op][index]
     */
    UN_LITERAL,

    /* returns values from functions */
    UN_RETURN,
} UNInstruction;

#endif