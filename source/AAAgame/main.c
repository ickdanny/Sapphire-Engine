#include <stdio.h>

#include "ZMath.h"
#include "Constructure.h"

void printInt(int *toPrint){
    printf("%d\n", *toPrint);
}

int main(){
    ArrayList list = arrayListMake(int, 2);

    for(int i = 0; i < 100; ++i){
        arrayListPushBack(int, &list, i);
    }

    printf("size: %lu\n", list.size);
    printf("capacity: %lu\n", list._capacity);
    arrayListApply(int, &list, printInt);

    arrayListFree(int, &list);
    printf("size: %lu\n", list.size);
    printf("capacity: %lu\n", list._capacity);
}