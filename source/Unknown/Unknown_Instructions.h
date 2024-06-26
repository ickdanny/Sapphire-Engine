#ifndef UNKNOWN_INSTRUCTIONS
#define UNKNOWN_INSTRUCTIONS

typedef enum UNInstruction {

    /*
     * instructs VM to load a literal value;
     * FORMAT: [op][index]
     */
    un_literal,
    /* binary addition */
    un_add,
    /* binary subtraction */
    un_subtract,
    /* binary multiplication */
    un_multiply,
    /* binary division */
    un_divide,
    /* unary negation */
    un_negate,
    /* returns values from functions */
    un_return,
} UNInstruction;

#endif