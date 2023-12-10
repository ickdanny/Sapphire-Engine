#include <stdio.h>

#include "ZMath.h"
#include "Constructure.h"

CONSTRUCTURE_ARRAY(IntArray, intArray, int)
CONSTRUCTURE_ARRAYLIST(IntArrayList, intArrayList, int)

void printInt(int toPrint){
    printf("%d\n", toPrint);
}

int main(){
    IntArray array = intArrayMake(10);
    intArraySetValue(&array, 3, 2);
    intArraySetValue(&array, 8, 7);
    intArrayForEachValue(&array, printInt);
    intArrayFree(&array);
    printf("size: %d\n", array.size);
}