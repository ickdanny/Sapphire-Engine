#ifndef ZMATH_NUMERIC_H
#define ZMATH_NUMERIC_H

extern inline int minInt(int a, int b){
    return a < b ? a : b;
}

extern inline int maxInt(int a, int b){
    return a > b ? a : b;
}

extern inline float minFloat(float a, float b){
    return a < b ? a : b;
}

extern inline float maxFloat(float a, float b){
    return a > b ? a : b;
}

#endif