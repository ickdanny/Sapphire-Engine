#ifndef NECRO_INSTRUCTION_H
#define NECRO_INSTRUCTION_H

typedef enum NecroInstruction {
    /*
     * instructs VM to load a literal value;
     * FORMAT: [op][index]
     */
    necro_literal,
    /* instructs VM to pop the top value */
    necro_pop,
    /* instructs VM to define a new global variable */
    necro_defineGlobal,
    /*
     * instructs VM to retrieve the value of a global
     * variable
     */
    necro_getGlobal,
    /*
     * instructs VM to set the value of a global
     * variable to the top of the stack
     */
    necro_setGlobal,
    /*
     * instructs VM to retrieve the value of a local
     * variable
     */
    necro_getLocal,
    /*
     * instructs VM to set the value of a local
     * variable to the top of the stack
     */
    necro_setLocal,
    /* instructs VM to load "true" */
    necro_true,
    /* instructs VM to load "false" */
    necro_false,
    /* binary addition */
    necro_add,
    /* binary subtraction */
    necro_subtract,
    /* binary multiplication */
    necro_multiply,
    /* binary division */
    necro_divide,
    /* integer modulus */
    necro_modulo,
    /* unary negation */
    necro_negate,
    /* binary equality comparison */
    necro_equal,
    /* binary greater comparison */
    necro_greater,
    /* binary less comparison */
    necro_less,
    /* unary boolean not */
    necro_not,
    /* creates a new (polar) vector */
    necro_makeVector,
    /* creates a new point */
    necro_makePoint,
    /* gets the magnitude of a vector */
    necro_getR,
    /* gets the angle of a vector in degrees */
    necro_getTheta,
    /* gets the x coordinate of a point */
    necro_getX,
    /* gets the y coordinate of a point */
    necro_getY,
    /* sets the magnitude of a global vector */
    necro_setRGlobal,
    /* sets the angle of a global vector in degrees */
    necro_setThetaGlobal,
    /* sets the x coordinate of a global point */
    necro_setXGlobal,
    /* sets the y coordinate of a global point */
    necro_setYGlobal,
    /* sets the magnitude of a local vector */
    necro_setRLocal,
    /* sets the angle of a local vector in degrees */
    necro_setThetaLocal,
    /* sets the x coordinate of a local point */
    necro_setXLocal,
    /* sets the y coordinate of a local point */
    necro_setYLocal,
    /* prints out a string */
    necro_print,
    /*
     * unconditionally jumps the specified distance;
     * distance is 2 bytes long
     */
    necro_jump,
    /*
     * jumps the specified distance if FALSE is on
     * the top of the stack; distance is 2 bytes long
     */
    necro_jumpIfFalse,
    /*
     * unconditionally jumps backwards the specified
     * distance; distance is 2 bytes long
     */
    necro_loop,
    /* calls a function */
    necro_call,
    /* returns values from functions */
    necro_return,
    /*
     * halts the execution of the virtual machine but
     * preserves its state
     */
    necro_yield,
    /* ends the execution of a program */
    necro_end,
} NecroInstruction;

#endif