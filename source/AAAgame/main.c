#include <stdio.h>
#include <wchar.h>

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

void incrementChar(char *c){
    ++(*c);
}

int main(){
    HashMap map = hashMapMake(
        int, char, 
        1, 
        intHash, 
        intEquals
    );

    assertTrue(hashMapIsEmpty(&map), "should be empty");
    
    int a = 8;
    char z = 'z';
    assertFalse(
        hashMapHasKeyPtr(int, char, &map, &a),
        "should not have such a value"
    );

    hashMapPutPtr(int, char, &map, &a, &z);
    assertTrue(
        hashMapHasKeyPtr(int, char, &map, &a),
        "should now have such a value"
    );
    assertTrue(
        hashMapHasKey(int, char, &map, a),
        "testing value passing"
    );
    assertFalse(
        hashMapHasKey(int, char, &map, 3),
        "should fail to find"
    );

    assertTrue(
        z == *hashMapGetPtr(int, char, &map, &a),
        "should get z"
    );

    hashMapPut(int, char, &map, 12, 'b');

    assertTrue(
        'b' == hashMapGet(int, char, &map, 12),
        "should get b"
    );

    hashMapApply(int, char, &map, incrementChar);
    assertTrue(
        'c' == hashMapGet(int, char, &map, 12),
        "should get c"
    );

    hashMapRemove(int, char, &map, a);
    assertFalse(
        hashMapHasKey(int, char, &map, a),
        "should not have such a value anymore"
    );

    hashMapClear(int, char, &map);
    assertTrue(hashMapIsEmpty(&map), "should be cleared");

    hashMapFree(int, char, &map);
    assertTrue(hashMapIsEmpty(&map), "should be freed");

    printf("completed main!\n");
    return 0;
}