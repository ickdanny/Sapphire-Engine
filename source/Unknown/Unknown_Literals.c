#include "Unknown_Literals.h"

/* todo: prints the specified value */
void unValuePrint(UNValue value){
    switch(value.type){
        case un_bool:
            printf(unAsBool(value) ? "true" : "false");
            break;
        case un_number:
            printf("%g", unAsNumber(value));
            break;
        case un_invalidValue:
            printf("invalid value");
            break;
    }
}

#define literalsInitCapacity 8

/* Constructs and returns a new UNLiterals by value */
UNLiterals unLiteralsMake(){
    UNLiterals toRet = {0};
    toRet.literals = arrayListMake(UNValue,
        literalsInitCapacity
    );
    return toRet;
}

/*
 * Returns the todo: UNValue at the specified index
 * in the given UNLiterals
 */
UNValue unLiteralsGet(
    UNLiterals *literalsPtr,
    size_t index
){
    return arrayListGet(UNValue,
        &(literalsPtr->literals),
        index
    );
}

/*
 * Pushes the specified todo: UNValue to the back of
 * the specified UNLiterals and returns the index
 * where it was written
 */
size_t unLiteralsPushBack(
    UNLiterals *literalsPtr,
    UNValue value
){
    arrayListPushBack(UNValue,
        &(literalsPtr->literals),
        value
    );
    return literalsPtr->literals.size - 1;
}

/*
 * Frees the memory associated with the specified
 * UNLiterals
 */
void unLiteralsFree(UNLiterals *literalsPtr){
    arrayListFree(UNValue, &(literalsPtr->literals));
    memset(literalsPtr, 0, sizeof(*literalsPtr));
}