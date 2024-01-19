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

bool intEquals(
    const void *intPtr1, 
    const void *intPtr2
){
    return *((int*)intPtr1) == *((int*)intPtr2);
}

#define intToLong(i) ((long)i)

int main(){
    SparseSet sparseSet = sparseSetMake(float, 50);
    assertTrue(
        sparseSet.capacity == 50, 
        "capacity should be 50"
    );
    assertTrue(
        sparseSet._size == 0,
        "size should be 0"
    );

    float four = 4.0f;

    sparseSetSet(float, &sparseSet, 20, 1.0f);
    sparseSetSet(float, &sparseSet, 10, 2.0f);
    sparseSetSet(float, &sparseSet, 40, 3.0f);
    sparseSetSetPtr(float, &sparseSet, 4, &four);

    assertTrue(
        sparseSetContains(float, &sparseSet, 10),
        "should contain 10"
    );
    assertTrue(
        sparseSetContains(float, &sparseSet, 4),
        "should contain 4"
    );

    assertTrue(
        sparseSetGet(float, &sparseSet, 40) == 3.0f,
        "should retrieve 3.0f"
    );

    sparseSetRemove(float, &sparseSet, 20);

    assertFalse(
        sparseSetContains(float, &sparseSet, 20),
        "should not contain 20"
    );

    SparseSetItr itr 
        = sparseSetItr(float, &sparseSet);
    assertTrue(
        sparseSetItrHasNext(float, &itr),
        "itr should have next"
    );
    float temp = sparseSetItrNext(float, &itr);
    assertTrue(
        temp == four,
        "expect first element to be four"
    );
    assertTrue(
        sparseSetItrIndex(float, &itr) == 4,
        "expect last index to be 4"
    );
    assertTrue(
        sparseSetItrNext(float, &itr) == 2.0f,
        "expect first element to be 2.0f"
    );
    assertTrue(
        sparseSetItrNext(float, &itr) == 3.0f,
        "expect first element to be 3.0f"
    );
    assertFalse(
        sparseSetItrHasNext(float, &itr),
        "itr should not have next"
    );

    sparseSetClear(float, &sparseSet);
    assertTrue(
        sparseSet.capacity == 50, 
        "capacity should be 50"
    );
    assertTrue(
        sparseSet._size == 0,
        "size should be 0"
    );

    for(int i = 0; i < 50; ++i){
        /* test to see if we can fill capacity */
        sparseSetSet(float, &sparseSet, i, (float)i);
    }
    itr = sparseSetItr(float, &sparseSet);
    for(int i = 0; i < 50; ++i){
        assertTrue(
            sparseSetItrNext(float, &itr) == (float)i,
            "should get i back"
        );
    }
    assertFalse(
        sparseSetItrHasNext(float, &itr),
        "itr should not have next"
    );

    sparseSetFree(float, &sparseSet);

    assertNull(sparseSet._densePtr, "dense null");
    assertNull(sparseSet._sparsePtr, "sparse null");
    assertNull(sparseSet._reflectPtr, "reflect null");

    printf("completed main!\n");
    return 0;
}