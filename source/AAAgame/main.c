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

int main(){
    String str1 = stringMakeC("hello world");
    stringInsertC(&str1, 7u, "test");
    printString(&str1);

    WideString ws1 = wideStringMakeC(L"hello world");
    wideStringInsertC(&ws1, 7u, L"test");
    printWideString(&ws1);

    return 0;
}