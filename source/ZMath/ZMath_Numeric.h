#ifndef ZMATH_NUMERIC_H
#define ZMATH_NUMERIC_H

/* Returns the min int: evaluates operands once */
int minInt(int a, int b);

/* Returns the max int: evaluates operands once */
int maxInt(int a, int b);

/* Returns the min float: evaluates operands once */
float minFloat(float a, float b);

/* Returns the max float: evaluates operands once */
float maxFloat(float a, float b);

/* Returns ceiling of the quotient of x/y */
int ceilingIntegerDivide(int x, int y);

#endif