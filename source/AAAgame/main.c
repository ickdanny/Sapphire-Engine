#include <stdio.h>

#include "ZMath.h"
#include "Constructure.h"

void printInt(int *toPrint){
    printf("%d\n", *toPrint);
}

typedef TYPENAME CharString;

void printCharString(const CharString *toPrint){
    printf("%s\n", toPrint->_ptr);
}

int main(){
    CharString str1 = makeC("");
    CharString str2 = makeC("1");
    printf("%d\n", compare(&str1, &str2));

    Free(&str1);
    Free(&str2);

    assertTrue(
        isEmpty(&str1), "expect empty"
    );
}