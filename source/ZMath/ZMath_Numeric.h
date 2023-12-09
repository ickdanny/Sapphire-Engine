#ifndef ZMATH_NUMERIC_H
#define ZMATH_NUMERIC_H

#ifndef min
/* A min macro: will evaluate expressions twice */
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
/* A max macro: will evaluate expressions twice */
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

/* Returns the min int: evaluates operands once */
extern inline int minInt(int a, int b){
    return min(a, b);
}

/* Returns the max int: evaluates operands once */
extern inline int maxInt(int a, int b){
    return max(a, b);
}

/* Returns the min float: evaluates operands once */
extern inline float minFloat(float a, float b){
    return min(a, b);
}

/* Returns the max float: evaluates operands once */
extern inline float maxFloat(float a, float b){
    return max(a, b);
}

/* Returns ceiling of the quotient of x/y */
extern inline int ceilingIntegerDivide(int x, int y){
    return (x / y) + (x % y != 0);
}

#endif