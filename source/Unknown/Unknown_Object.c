#include "Unknown_Object.h"

/*
 * Creates and returns a new UNObjectString by copying
 * the specified number of characters from the given
 * character pointer
 */
UNObjectString unObjectStringCopy(
    const char *chars,
    size_t length
){
    UNObjectString toRet = {0};
    toRet.objectBase.type = un_stringObject;
    toRet.string = stringMakeAndReserve(length + 1);
    //todo: string write n chars in constructure
}