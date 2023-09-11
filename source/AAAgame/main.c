#include <stdio.h>
#include "ZMath.h"

#include <Constructure_ArrayList.h>

CONSTRUCTURE_ARRAYLIST(IntArrayList, intArrayList, int)

void printInt(int toPrint){
    printf("%d\n", toPrint);
}

int main(){
    IntArrayList intArrayList = intArrayListMake(3);
    intArrayListPushBackValue(&intArrayList, 1);
    intArrayListPushBackValue(&intArrayList, 2);
    intArrayListPushBackValue(&intArrayList, 3);
    intArrayListPushBackValue(&intArrayList, 4);
    intArrayListPushBackValue(&intArrayList, 5);
    intArrayListPopBack(&intArrayList);
    intArrayListErase(&intArrayList, 1);
    intArrayListAssignValue(&intArrayList, 12, 1);
    intArrayListInsertValue(&intArrayList, 222, 3);
    intArrayListInsertValue(&intArrayList, -23, 0);
    for(int i = 0; i < 100; ++i){
        intArrayListPushBackValue(&intArrayList, i);
    }
    intArrayListErase(&intArrayList, 2);

    intArrayListForEachValue(&intArrayList, printInt);
    printf("Size : %d\n", intArrayList.size);
    printf("Capacity : %d\n", intArrayList.capacity);
    printf("Back: %d\n", intArrayListBack(&intArrayList));
    printf("Front: %d\n", intArrayListFront(&intArrayList));

    intArrayListFree(&intArrayList);
    printf("Size : %d\n", intArrayList.size);
    printf("Capacity : %d\n", intArrayList.capacity);
}