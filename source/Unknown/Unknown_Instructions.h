#ifndef UNKNOWN_INSTRUCTIONS
#define UNKNOWN_INSTRUCTIONS

typedef enum UNInstruction {

    /*
     * instructs VM to load a literal value;
     * FORMAT: [op][index]
     */
    un_literal,
    /* instructs VM to load "true" */
    un_true,
    /* instructs VM to load "false" */
    un_false,
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
    /* binary equality comparison */
    un_equal,
    /* binary greater comparison */
    un_greater,
    /* binary less comparison */
    un_less,
    /* unary boolean not */
    un_not,
    /* returns values from functions */
    un_return,
} UNInstruction;

#endif