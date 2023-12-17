#include <stdio.h>

#include "ZMath.h"
#include "Constructure.h"

void printInt(int *toPrint){
    printf("%d\n", *toPrint);
}

int main(){
    Array array = arrayMake(int, 10);
    arraySet(int, &array, 3, 2);
    arraySet(int, &array, 8, 7);
    arraySet(int, &array, 2, 9);
    arraySet(int, &array, 9, 30);

    int value = 4;
    int *ptr = &value;
    arraySetPtr(int, &array, 0, ptr);

    printf("Get 9: %d\n", arrayGet(int, &array, 9));

    arrayApply(int, &array, printInt);
    printf("size: %ld\n", array.size);

    arrayClear(int, &array);

    arrayFree(int, &array);
    printf("size: %ld\n", array.size);
}