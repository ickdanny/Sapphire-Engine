#ifndef ZMATH_NUMERIC_H
#define ZMATH_NUMERIC_H

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

extern inline int minInt(int a, int b){
    return min(a, b);
}

extern inline int maxInt(int a, int b){
    return max(a, b);
}

extern inline float minFloat(float a, float b){
    return min(a, b);
}

extern inline float maxFloat(float a, float b){
    return max(a, b);
}

#endif