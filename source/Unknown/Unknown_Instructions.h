#ifndef UNKNOWN_INSTRUCTIONS
#define UNKNOWN_INSTRUCTIONS

typedef enum UNInstruction {

    /*
     * instructs VM to load a literal value;
     * FORMAT: [op][index]
     */
    un_literal,
    /* instructs VM to pop the top value */
    un_pop,
    /* instructs VM to define a new global variable */
    un_defineGlobal,
    /*
     * instructs VM to retrieve the value of a global
     * variable
     */
    un_getGlobal,
    /*
     * instructs VM to set the value of a global
     * variable to the top of the stack
     */
    un_setGlobal,
    /*
     * instructs VM to retrieve the value of a local
     * variable
     */
    un_getLocal,
    /*
     * instructs VM to set the value of a local
     * variable to the top of the stack
     */
    un_setLocal,
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
    /* integer modulus */
    un_modulo,
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
    /* creates a new (polar) vector */
    un_makeVector,
    /* creates a new point */
    un_makePoint,
    /* gets the magnitude of a vector */
    un_getR,
    /* gets the angle of a vector in degrees */
    un_getTheta,
    /* gets the x coordinate of a point */
    un_getX,
    /* gets the y coordinate of a point */
    un_getY,
    /* sets the magnitude of a global vector */
    un_setRGlobal,
    /* sets the angle of a global vector in degrees */
    un_setThetaGlobal,
    /* sets the x coordinate of a global point */
    un_setXGlobal,
    /* sets the y coordinate of a global point */
    un_setYGlobal,
    /* sets the magnitude of a local vector */
    un_setRLocal,
    /* sets the angle of a local vector in degrees */
    un_setThetaLocal,
    /* sets the x coordinate of a local point */
    un_setXLocal,
    /* sets the y coordinate of a local point */
    un_setYLocal,
    /* prints out a string */
    un_print,
    /*
     * unconditionally jumps the specified distance;
     * distance is 2 bytes long
     */
    un_jump,
    /*
     * jumps the specified distance if FALSE is on
     * the top of the stack; distance is 2 bytes long
     */
    un_jumpIfFalse,
    /*
     * unconditionally jumps backwards the specified
     * distance; distance is 2 bytes long
     */
    un_loop,
    /* calls a function */
    un_call,
    /* returns values from functions */
    un_return,
    /*
     * halts the execution of the virtual machine but
     * preserves its state
     */
    un_yield,
    /* ends the execution of a program */
    un_end,
} UNInstruction;

#endif