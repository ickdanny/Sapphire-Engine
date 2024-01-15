#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <time.h>

#include "ZMath.h"
#include "Constructure.h"

void printInt(int *toPrint){
    printf("%d\n", *toPrint);
}

void printString(const String *toPrint){
    printf("%s\n", toPrint->_ptr);
}

void printWideString(const WideString *toPrint){
    printf("%ls\n", toPrint->_ptr);
}

size_t intHash(const void *intPtr){
    return *((int*)intPtr);
}

bool intEquals(const void *intPtr1, const void *intPtr2){
    return *((int*)intPtr1) == *((int*)intPtr2);
}

#define intToLong(i) ((long)i)

int main(){
    HashMap map = hashMapMake(
        int, long, 
        1, 
        intHash, 
        intEquals
    );

    srand(0);

    int randomInt;
    for(int i = 0; i < 2000; ++i){
        randomInt = rand();
        hashMapPut(int, long, &map, randomInt, 
            intToLong(randomInt));
    }
    clock_t start = clock();
    for(int i = 0; i < INT_MAX; ++i){
        hashMapHasKey(int, long, &map, i);
    }
    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("time: %lf\n", time);
    printf("time per check: %lf\n", time / INT_MAX);

    hashMapFree(int, long, &map);
    assertTrue(hashMapIsEmpty(&map), 
        "should be freed");

    printf("completed main!\n");
    return 0;
}