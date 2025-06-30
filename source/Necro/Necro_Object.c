#include "Necro_Object.h"

/*
 * Allocates a new NecroObject of the specified size
 * with the given type and inserts it at the head of
 * the specified object list (nullable)
 */
static NecroObject *_necroObjectAlloc(
    size_t size,
    NecroObjectType type,
    NecroObject **listHeadPtrPtr
){
    NecroObject *toRet = pgAlloc(1, size);
    toRet->type = type;

    if(listHeadPtrPtr){
        toRet->nextPtr = *listHeadPtrPtr;
        *(listHeadPtrPtr) = toRet;
    }

    return toRet;
}

/* Allocates a new NecroObject of the specified type */
#define necroObjectAlloc( \
    TYPE, \
    OBJTYPE, \
    LISTHEADPTRPTR \
) \
    (TYPE*)_necroObjectAlloc( \
        sizeof(TYPE), \
        OBJTYPE, \
        LISTHEADPTRPTR \
    )

/*
 * An equals function for NecroObjectString* that
 * actually walks the strings, used for string
 * interning
 */
bool _necroObjectStringPtrCharwiseEquals(
    const void *voidPtr1,
    const void *voidPtr2
){
    NecroObjectString **stringPtrPtr1
        = (NecroObjectString**)voidPtr1;
    NecroObjectString *stringPtr1 = *stringPtrPtr1;
    NecroObjectString **stringPtrPtr2
        = (NecroObjectString**)voidPtr2;
    NecroObjectString *stringPtr2 = *stringPtrPtr2;

    return constructureStringEquals(
        &(stringPtr1->string),
        &(stringPtr2->string)
    );
}

/*
 * Allocates and returns a new NecroObjectString by
 * pointer, copying the specified number of characters
 * from the given character pointer, and inserting that
 * object at the start of the given object list
 * (nullable); however, if the string to copy is
 * already interned in the given HashMap, this
 * function simply returns a pointer to the
 * preexisting string and does not insert it in the
 * object list, otherwise inserts the string
 * into the map (NOTE: compile-constant strings are
 * owned by the literals and do not show up in the
 * virtual machine object list)
 */
NecroObjectString *necroObjectStringCopy(
    const char *chars,
    size_t length,
    NecroObject **listHeadPtrPtr,
    HashMap *stringMapPtr
){
    NecroObjectString *toRet = necroObjectAlloc(
        NecroObjectString,
        necro_stringObject,
        listHeadPtrPtr
    );
    toRet->string = stringMakeCLength(chars, length);
    toRet->cachedHashCode = constructureStringHash(
        &(toRet->string)
    );

    /*
     * swap out the equals func for a O(n) charwise
     * string compare
     */
    stringMapPtr->_equalsFunc
        = _necroObjectStringPtrCharwiseEquals;
    /*
     * if string map has an identical string, free
     * the newly allocated string and return a pointer
     * to the preexisting one
     */
    if(hashMapHasKey(NecroObjectString*, NecroValue,
        stringMapPtr,
        toRet
    )){
        /*
         * remove the temp string from the object list
         */
        if(listHeadPtrPtr){
            *listHeadPtrPtr
                = toRet->objectBase.nextPtr;
        }
        /*
         * get the matching key in the string map,
         * i.e. the pointer to the interned string
         */
        NecroObjectString *preexistingStringPtr
            = hashMapGetKey(
                NecroObjectString*,
                NecroValue,
                stringMapPtr,
                toRet
            );
        necroObjectFree((NecroObject*)toRet);
        toRet = preexistingStringPtr;
    }
    /* otherwise, insert into the hashmap */
    else{
        hashMapPut(NecroObjectString*, NecroValue,
            stringMapPtr,
            toRet,
            0
        );
    }
    /* unswap the equals func */
    stringMapPtr->_equalsFunc
        = _necroObjectStringPtrEquals;
    
    return toRet;
}

/*
 * Allocates and returns a new NecroObjectString by
 * pointer, holding the concatenation of the two
 * specified NecroObjectStrings, and also inserts that
 * object at the start of the given object list
 * (nullable); however, if the string to copy is
 * already interned in the given HashMap, this
 * function simply returns a pointer to the
 * preexisting string and does not insert it in the
 * object list, otherwise inserts the string
 * into the map (NOTE: compile-constant strings are
 * owned by the literals and do not show up in the
 * virtual machine object list)
 */
NecroObjectString *necroObjectStringConcat(
    NecroObjectString *leftStringPtr,
    NecroObjectString *rightStringPtr,
    NecroObject **listHeadPtrPtr,
    HashMap *stringMapPtr
){
    NecroObjectString *toRet = necroObjectAlloc(
        NecroObjectString,
        necro_stringObject,
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
    toRet->cachedHashCode = constructureStringHash(
        &(toRet->string)
    );

    /*
     * swap out the equals func for a O(n) charwise
     * string compare
     */
    stringMapPtr->_equalsFunc
        = _necroObjectStringPtrCharwiseEquals;
    /*
     * if string map has an identical string, free
     * the newly allocated string and return a pointer
     * to the preexisting one
     */
    if(hashMapHasKey(NecroObjectString*, NecroValue,
        stringMapPtr,
        toRet
    )){
        /*
         * remove the temp string from the object list
         */
        if(listHeadPtrPtr){
            *listHeadPtrPtr
                = toRet->objectBase.nextPtr;
        }
        /*
         * get the matching key in the string map,
         * i.e. the pointer to the interned string
         */
        NecroObjectString *preexistingStringPtr
            = hashMapGetKey(
                NecroObjectString*,
                NecroValue,
                stringMapPtr,
                toRet
            );
        necroObjectFree((NecroObject*)toRet);
        toRet = preexistingStringPtr;
    }
    /* otherwise, insert into the hashmap */
    else{
        hashMapPut(NecroObjectString*, NecroValue,
            stringMapPtr,
            toRet,
            0
        );
    }
    /* unswap the equals func */
    stringMapPtr->_equalsFunc
        = _necroObjectStringPtrEquals;

    return toRet;
}

/*
 * Creates and returns a new NecroObjectFunc by
 * pointer; the enclosing function object pointer is
 * nullable
 */
NecroObjectFunc *necroObjectFuncMake(
    NecroObjectFunc *enclosingPtr
){
    NecroObjectFunc *toRet = necroObjectAlloc(
        NecroObjectFunc,
        necro_funcObject,
        NULL /* null for list; called by compiler */
    );
    toRet->arity = 0;
    toRet->namePtr = NULL;
    if(!enclosingPtr){
        toRet->program = necroProgramMake(NULL);
    }
    else{
        toRet->program = necroProgramMake(
            &(enclosingPtr->program)
        );
    }
    return toRet;
}

/*
 * Creates and returns a new NecroObjectNativeFunc by
 * pointer and also inserts the object at the start of
 * the given object list
 */
NecroObjectNativeFunc *necroObjectNativeFuncMake(
    NecroNativeFunc func,
    NecroObject **listHeadPtrPtr
){
    NecroObjectNativeFunc *toRet = necroObjectAlloc(
        NecroObjectNativeFunc,
        necro_nativeFuncObject,
        listHeadPtrPtr
    );
    toRet->func = func;
    return toRet;
}

/*
 * Returns true if the two specified objects are equal,
 * false otherwise
 */
bool necroObjectEquals(NecroObject *a, NecroObject *b){
    if(a->type != b->type){
        return false;
    }
    switch(a->type){
        case necro_stringObject: {
            /*
             * due to string interning, we can check
             * string equality with pointer equality
             */
            return a == b;
        }
        default:
            pgError(
                "unexpected default; "
                SRC_LOCATION
            );
            return false;
    }
}

/* prints the specified function object */
static void printFunction(NecroObjectFunc *funcPtr){
    printf(
        "<fn %s>",
        funcPtr->namePtr != NULL
            ? funcPtr->namePtr->string._ptr
            : "<unnamed script>"
    );
}

/* Prints the value if it an object, error otherwise */
void necroObjectPrint(NecroValue value){
    assertTrue(
        necroIsObject(value),
        "error: non-object passed to print object; "
        SRC_LOCATION
    );
    switch(necroObjectGetType(value)){
        case necro_stringObject:
            printf("%s", necroObjectAsCString(value));
            break;
        case necro_funcObject:
            printFunction(necroObjectAsFunc(value));
            break;
        case necro_nativeFuncObject:
            printf("<native fn>");
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
void necroObjectFree(NecroObject *objectPtr){
    assertNotNull(
        objectPtr,
        "null passed to necroObjectFree; "SRC_LOCATION
    );

    /* free any extra data allocated for the object */
    switch(objectPtr->type){
        case necro_stringObject: {
            NecroObjectString *stringPtr
                = (NecroObjectString*)objectPtr;
            stringFree(&(stringPtr->string));
            break;
        }
        case necro_funcObject: {
            NecroObjectFunc *funcPtr
                = (NecroObjectFunc*)objectPtr;
            /* functions own their own code; free it */
            necroProgramFree(&(funcPtr->program));
            break;
        }
        case necro_nativeFuncObject: {
            /* do nothing; free the object below */
            break;
        }
        default:
            pgError(
                "unexpected default object type; "
                SRC_LOCATION
            );
            return;
    }

    /* free all objects; they are on the heap */
    pgFree(objectPtr);
}

/* For use with the Constructure Hashmap */
size_t _necroObjectStringPtrHash(const void *voidPtr){
    NecroObjectString **stringPtrPtr
        = (NecroObjectString**)voidPtr;
    NecroObjectString *stringPtr = *stringPtrPtr;
    return stringPtr->cachedHashCode;
}

/* For use with the Constructure Hashmap */
bool _necroObjectStringPtrEquals(
    const void *voidPtr1,
    const void *voidPtr2
){
    NecroObjectString **stringPtrPtr1
        = (NecroObjectString**)voidPtr1;
    NecroObjectString *stringPtr1 = *stringPtrPtr1;
    NecroObjectString **stringPtrPtr2
        = (NecroObjectString**)voidPtr2;
    NecroObjectString *stringPtr2 = *stringPtrPtr2;

    /* use pointer equality since string interning */
    return stringPtr1 == stringPtr2;
}