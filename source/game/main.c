#include <stdio.h>
#include "ZMath.h"

int main(){
    Point point = {1.0f, 2.0f};
    point = pointCopy(point);
    char string[20] = {0};
    pointToString(point, string, 20);
    printf("%s\n", string);
    Vector vector = {3.4f, 4.5f};
    vector = vectorCopy(vector);
    vectorToString(vector, string, 20);
    printf("%s\n", string);
    Polar polar = {4.0f, 90.0f};
    polar = polarCopy(polar);
    polarToString(&polar, string, 20);
    printf("%s\n", string);
}