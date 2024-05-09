#include "ZMath_Numeric.h"

#ifndef min
/* A min macro: will evaluate expressions twice */
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
/* A max macro: will evaluate expressions twice */
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

/* Returns the min int: evaluates operands once */
int minInt(int a, int b){
    return min(a, b);
}

/* Returns the max int: evaluates operands once */
int maxInt(int a, int b){
    return max(a, b);
}

/* Returns the min float: evaluates operands once */
float minFloat(float a, float b){
    return min(a, b);
}

/* Returns the max float: evaluates operands once */
float maxFloat(float a, float b){
    return max(a, b);
}

/* Returns ceiling of the quotient of x/y */
int ceilingIntegerDivide(int x, int y){
    return (x / y) + (x % y != 0);
}