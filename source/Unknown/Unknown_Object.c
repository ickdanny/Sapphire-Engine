#include "Unknown_Object.h"

/*
 * Allocates a new UNObject of the specified size
 * with the given type and inserts it at the head of
 * the specified object list (nullable)
 */
static UNObject *_unObjectAlloc(
    size_t size,
    UNObjectType type,
    UNObject **listHeadPtrPtr
){
    UNObject *toRet = pgAlloc(1, size);
    toRet->type = type;

    if(listHeadPtrPtr){
        toRet->nextPtr = *listHeadPtrPtr;
        *(listHeadPtrPtr) = toRet;
    }

    return toRet;
}

/* Allocates a new UNObject of the specified type */
#define unObjectAlloc(TYPE, OBJTYPE, LISTHEADPTRPTR) \
    (TYPE*)_unObjectAlloc( \
        sizeof(TYPE), \
        OBJTYPE, \
        LISTHEADPTRPTR \
    )

/*
 * Allocates and returns a new UNObjectString by
 * pointer, copying the specified number of characters
 * from the given character pointer, and inserting that
 * object at the start of the given object list
 * (nullable)
 */
UNObjectString *unObjectStringCopy(
    const char *chars,
    size_t length,
    UNObject **listHeadPtrPtr
){
    UNObjectString *toRet = unObjectAlloc(
        UNObjectString,
        un_stringObject,
        listHeadPtrPtr
    );
    toRet->string = stringMakeCLength(chars, length);
    return toRet;
}

/*
 * Allocates and returns a new UNObjectString by
 * pointer, holding the concatenation of the two
 * specified UNObjectStrings, and also inserts that
 * object at the start of the given object list
 * (nullable)
 */
UNObjectString *unObjectStringConcat(
    UNObjectString *leftStringPtr,
    UNObjectString *rightStringPtr,
    UNObject **listHeadPtrPtr
){
    UNObjectString *toRet = unObjectAlloc(
        UNObjectString,
        un_stringObject,
        listHeadPtrPtr
    );
    size_t totalLength = leftStringPtr->string.length
        + rightStringPtr->string.length;
    toRet->string = stringMakeAndReserve(
        totalLength + 1
    );
    /* append the contesnts of the left operand */
    stringAppend(
        &(toRet->string),
        &(leftStringPtr->string)
    );
    /* append the contents of the right operand */
    stringAppend(
        &(toRet->string),
        &(rightStringPtr->string)
    );

    return toRet;
}

/*
 * Returns true if the two specified objects are equal,
 * false otherwise
 */
bool unObjectEquals(UNObject *a, UNObject *b){
    if(a->type != b->type){
        return false;
    }
    switch(a->type){
        case un_stringObject: {
            UNObjectString *strPtr1
                = (UNObjectString*)a;
            UNObjectString *strPtr2
                = (UNObjectString*)b;
            return stringEquals(
                &(strPtr1->string),
                &(strPtr2->string)
            );
        }
        default:
            pgError(
                "unexpected default; "
                SRC_LOCATION
            );
            return false;
    }
}

/* Prints the value if it an object, error otherwise */
void unObjectPrint(UNValue value){
    assertTrue(
        unIsObject(value),
        "error: non-object passed to print object; "
        SRC_LOCATION
    );
    switch(unObjectGetType(value)){
        case un_stringObject:
            printf("%s", unObjectAsCString(value));
            break;
        default:
            pgError(
                "unexpected default; "
                SRC_LOCATION
            );
            break;
    }
}

/*
 * Frees the memory associated with the specified
 * object
 */
void unObjectFree(UNObject *objectPtr){
    assertNotNull(
        objectPtr,
        "null passed to unObjectFree; "SRC_LOCATION
    );

    switch(objectPtr->type){
        case un_stringObject: {
            UNObjectString *stringPtr
                = (UNObjectString*)objectPtr;
            printf(
                "free: %s\n",
                stringPtr->string._ptr
            );
            stringFree(&(stringPtr->string));
            break;
        }
        default:
            /* do nothing */
            return;
    }

    /* all objects are on the heap */
    pgFree(objectPtr);
}