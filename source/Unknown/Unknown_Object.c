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
 * An equals function for UNObjectString* that actually
 * walks the strings, used for string interning
 */
static bool _unObjectStringPtrCharwiseEquals(
    const void *voidPtr1,
    const void *voidPtr2
){
    UNObjectString **stringPtrPtr1
        = (UNObjectString**)voidPtr1;
    UNObjectString *stringPtr1 = *stringPtrPtr1;
    UNObjectString **stringPtrPtr2
        = (UNObjectString**)voidPtr2;
    UNObjectString *stringPtr2 = *stringPtrPtr2;

    return constructureStringEquals(
        &(stringPtr1->string),
        &(stringPtr2->string)
    );
}

/*
 * Allocates and returns a new UNObjectString by
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
UNObjectString *unObjectStringCopy(
    const char *chars,
    size_t length,
    UNObject **listHeadPtrPtr,
    HashMap *stringMapPtr
){
    UNObjectString *toRet = unObjectAlloc(
        UNObjectString,
        un_stringObject,
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
        = _unObjectStringPtrCharwiseEquals;
    /*
     * if string map has an identical string, free
     * the newly allocated string and return a pointer
     * to the preexisting one
     */
    if(hashMapHasKey(UNObjectString*, UNValue,
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
        UNObjectString *preexistingStringPtr
            = hashMapGetKey(UNObjectString*, UNValue,
                stringMapPtr,
                toRet
            );
        unObjectFree((UNObject*)toRet);
        toRet = preexistingStringPtr;
    }
    /* otherwise, insert into the hashmap */
    else{
        hashMapPut(UNObjectString*, UNValue,
            stringMapPtr,
            toRet,
            0
        );
    }
    /* unswap the equals func */
    stringMapPtr->_equalsFunc
        = _unObjectStringPtrEquals;
    
    return toRet;
}

/*
 * Allocates and returns a new UNObjectString by
 * pointer, holding the concatenation of the two
 * specified UNObjectStrings, and also inserts that
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
UNObjectString *unObjectStringConcat(
    UNObjectString *leftStringPtr,
    UNObjectString *rightStringPtr,
    UNObject **listHeadPtrPtr,
    HashMap *stringMapPtr
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
    toRet->cachedHashCode = constructureStringHash(
        &(toRet->string)
    );

    /*
     * swap out the equals func for a O(n) charwise
     * string compare
     */
    stringMapPtr->_equalsFunc
        = _unObjectStringPtrCharwiseEquals;
    /*
     * if string map has an identical string, free
     * the newly allocated string and return a pointer
     * to the preexisting one
     */
    if(hashMapHasKey(UNObjectString*, UNValue,
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
        UNObjectString *preexistingStringPtr
            = hashMapGetKey(UNObjectString*, UNValue,
                stringMapPtr,
                toRet
            );
        unObjectFree((UNObject*)toRet);
        toRet = preexistingStringPtr;
    }
    /* otherwise, insert into the hashmap */
    else{
        hashMapPut(UNObjectString*, UNValue,
            stringMapPtr,
            toRet,
            0
        );
    }
    /* unswap the equals func */
    stringMapPtr->_equalsFunc
        = _unObjectStringPtrEquals;

    return toRet;
}

/*
 * Creates and returns a new UNObjectFunc by pointer;
 * the enclosing function object pointer is nullable
 */
UNObjectFunc *unObjectFuncMake(
    UNObjectFunc *enclosingPtr
){
    UNObjectFunc *toRet = unObjectAlloc(
        UNObjectFunc,
        un_funcObject,
        NULL /* null for list; called by compiler */
    );
    toRet->arity = 0;
    toRet->namePtr = NULL;
    if(!enclosingPtr){
        toRet->program = unProgramMake(NULL);
    }
    else{
        toRet->program = unProgramMake(
            &(enclosingPtr->program)
        );
    }
    return toRet;
}

/*
 * Creates and returns a new UNObjectNativeFunc by
 * pointer
 */
UNObjectNativeFunc *unObjectNativeFuncMake(
    UNNativeFunc func
){
    UNObjectNativeFunc *toRet = unObjectAlloc(
        UNObjectNativeFunc,
        un_nativeFuncObject,
        NULL /* null for list; called by compiler */
    );
    toRet->func = func;
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
static void printFunction(UNObjectFunc *funcPtr){
    printf(
        "<fn %s>",
        funcPtr->namePtr != NULL
            ? funcPtr->namePtr->string._ptr
            : "<unnamed script>"
    );
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
        case un_funcObject:
            printFunction(unObjectAsFunc(value));
            break;
        case un_nativeFuncObject:
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
void unObjectFree(UNObject *objectPtr){
    assertNotNull(
        objectPtr,
        "null passed to unObjectFree; "SRC_LOCATION
    );

    /* free any extra data allocated for the object */
    switch(objectPtr->type){
        case un_stringObject: {
            UNObjectString *stringPtr
                = (UNObjectString*)objectPtr;
            printf(
                "free: \"%s\"\n",
                stringPtr->string._ptr
            );
            stringFree(&(stringPtr->string));
            break;
        }
        case un_funcObject: {
            UNObjectFunc *funcPtr
                = (UNObjectFunc*)objectPtr;
            printf(
                "free func: %s\n",
                funcPtr->namePtr != NULL
                    ? funcPtr->namePtr->string._ptr
                    : "*unnamed func*"
            );
            /* functions own their own code; free it */
            unProgramFree(&(funcPtr->program));
            break;
        }
        case un_nativeFuncObject: {
            /* do nothing; free the object below */
            break;
        }
        default:
            pgError(
                "unexpected default; "
                SRC_LOCATION
            );
            return;
    }

    /* free all objects; they are on the heap */
    pgFree(objectPtr);
}

/* For use with the Constructure Hashmap */
size_t _unObjectStringPtrHash(const void *voidPtr){
    UNObjectString **stringPtrPtr
        = (UNObjectString**)voidPtr;
    UNObjectString *stringPtr = *stringPtrPtr;
    return stringPtr->cachedHashCode;
}

/* For use with the Constructure Hashmap */
bool _unObjectStringPtrEquals(
    const void *voidPtr1,
    const void *voidPtr2
){
    UNObjectString **stringPtrPtr1
        = (UNObjectString**)voidPtr1;
    UNObjectString *stringPtr1 = *stringPtrPtr1;
    UNObjectString **stringPtrPtr2
        = (UNObjectString**)voidPtr2;
    UNObjectString *stringPtr2 = *stringPtrPtr2;

    /* use pointer equality since string interning */
    return stringPtr1 == stringPtr2;
}