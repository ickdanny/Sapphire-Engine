#ifndef NECRO_OBJECT_H
#define NECRO_OBJECT_H

#include "Necro_Value.h"
#include "Necro_Program.h"

#include "PGUtil.h"
#include "Constructure.h"

/*
 * Defines the types of objects which Necro supports
 */
typedef enum NecroObjectType{
    necro_invalidObject,
    necro_stringObject,
    necro_funcObject,
    necro_nativeFuncObject
} NecroObjectType;

/* the base struct for all object types */
typedef struct NecroObject{
    NecroObjectType type;
    /* forms a linked list of objects for GC */
    struct NecroObject *nextPtr;
} NecroObject;

/* An object holding a string */
typedef struct NecroObjectString{
    NecroObject objectBase;
    String string;
    size_t cachedHashCode;
} NecroObjectString;

/* An object referring to a function */
typedef struct NecroObjectFunc{
    NecroObject objectBase;
    int arity;
    int depth;
    NecroProgram program;
    NecroObjectString *namePtr;
} NecroObjectFunc;

/* The prototype for a native function */
typedef NecroValue (*NecroNativeFunc)(
    int argc,
    NecroValue* argv
);

/* An object referring to a native function */
typedef struct NecroObjectNativeFunc{
    NecroObject objectBase;
    NecroNativeFunc func;
} NecroObjectNativeFunc;

/*
 * Returns true if the specified value is an object of
 * the given type, false otherwise
 */
static inline bool necroObjectCheckType(
    NecroValue value,
    NecroObjectType expectedType
){
    return necroIsObject(value)
        && necroAsObject(value)->type == expectedType;
}

/*
 * access the type of the specified object; error if
 * the given NecroValue is not actually an object
 */
#define necroObjectGetType(VALUE) \
    (necroAsObject(VALUE)->type)

/*
 * Returns true if the specified value is a string
 * object, false otherwise
 */
#define necroIsString(VALUE) \
    necroObjectCheckType((VALUE), necro_stringObject)

/*
 * Returns true if the specified value is a func
 * object, false otherwise
 */
#define necroIsFunc(VALUE) \
    necroObjectCheckType((VALUE), necro_funcObject)

/*
 * Returns true if the specified value is a native func
 * object, false otherwise
 */
#define necroIsNativeFunc(VALUE) \
    necroObjectCheckType((VALUE), \
        necro_nativeFuncObject)

/*
 * Returns a pointer to the String Object contained in
 * the specified value, error if the value is not a
 * String object
 */
static inline NecroObjectString *necroObjectAsString(
    NecroValue value
){
    if(!necroIsString(value)){
        pgError(
            "value is not a string; "
            SRC_LOCATION
        );
    }
    return (NecroObjectString*) necroAsObject(value);
}

/*
 * Returns a pointer to the C string contained in the
 * specified value, error if the value is not a String
 * object
 */
static inline char *necroObjectAsCString(
    NecroValue value
){
    return necroObjectAsString(value)->string._ptr;
}

/*
 * Returns a pointer to the func object contained in
 * the specified value, error if the value is not a
 * func object
 */
static inline NecroObjectFunc *necroObjectAsFunc(
    NecroValue value
){
    if(!necroIsFunc(value)){
        pgError(
            "value is not a func; "
            SRC_LOCATION
        );
    }
    return (NecroObjectFunc*) necroAsObject(value);
}

/*
 * Returns a pointer to the native func object
 * contained in the specified value, error if the
 * value is not a func object
 */
static inline NecroObjectNativeFunc
    *necroObjectAsNativeFunc(
        NecroValue value
){
    if(!necroIsNativeFunc(value)){
        pgError(
            "value is not a native func; "
            SRC_LOCATION
        );
    }
    return (NecroObjectNativeFunc*)
        necroAsObject(value);
}

/*
 * An equals function for NecroObjectString* that
 * actually walks the strings, used for string
 * interning
 */
bool _necroObjectStringPtrCharwiseEquals(
    const void *voidPtr1,
    const void *voidPtr2
);

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
);

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
);

/*
 * Creates and returns a new NecroObjectFunc by
 * pointer; the enclosing function object pointer is
 * nullable
 */
NecroObjectFunc *necroObjectFuncMake(
    NecroObjectFunc *enclosingPtr,
    int depth
);

/*
 * Creates and returns a new NecroObjectNativeFunc by
 * pointer and also inserts the object at the start of
 * the given object list
 */
NecroObjectNativeFunc *necroObjectNativeFuncMake(
    NecroNativeFunc func,
    NecroObject **listHeadPtrPtr
);

/*
 * Returns true if the two specified objects are equal,
 * false otherwise
 */
bool necroObjectEquals(NecroObject *a, NecroObject *b);

/* Prints the value if it an object, error otherwise */
void necroObjectPrint(NecroValue value);

/*
 * Frees the memory associated with the specified
 * object
 */
void necroObjectFree(NecroObject *objectPtr);

/* For use with the Constructure Hashmap */
size_t _necroObjectStringPtrHash(const void *voidPtr);

/* For use with the Constructure Hashmap */
bool _necroObjectStringPtrEquals(
    const void *voidPtr1,
    const void *voidPtr2
);

#endif