#ifndef ZMATH_POLAR_H
#define ZMATH_POLAR_H

#include "Vector.h"

typedef struct Polar{
    float magnitude;
    float angle;
    
    Vector asVector;
} Polar;

void _polarUpdateVector(){

}

inline Polar polarFromVector(Vector vector){
    //todo
}

void polarSetMagnitude(Polar *polar, float magnitude){
    //todo
}

void polarSetAngle(Polar *polar, float angle){

}

void polarNegate(Polar *polar){

}

void polarToVector(Polar *polar){

}

#endif