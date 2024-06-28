#ifndef UNKNOWN_OBJECT_H
#define UNKNOWN_OBJECT_H

#include "Unknown_Value.h"
#include "PGUtil.h"
#include "Constructure.h"

/*
 * Defines the types of objects which Unknown supports
 */
typedef enum UNObjectType{
    un_invalidObject,
    un_stringObject,
} UNObjectType;

//todo

/* the base struct for all object types */
typedef struct UNObject{
    UNObjectType type;
} UNObject;

/* An object holding a string */
typedef struct UNObjectString{
    UNObject objectBase;
    String string;
} UNObjectString;

/*
 * Returns true if the specified value is an object of
 * the given type, false otherwise
 */
static inline bool unObjectCheckType(
    UNValue value,
    UNObjectType expectedType
){
    return unIsObject(value)
        && unAsObject(value)->type == expectedType;
}

/*
 * access the type of the specified object; error if
 * the given UNValue is not actually an object
 */
#define unObjectGetType(VALUE) \
    (unAsObject(VALUE)->type)

/*
 * Returns true if the specified value is a string
 * object, false otherwise
 */
#define unIsString(VALUE) \
    unObjectCheckType((VALUE), un_stringObject)

/*
 * Returns a pointer to the String Object contained in
 * the specified value, error if the value is not a
 * String object
 */
static inline UNObjectString *unObjectAsString(
    UNValue value
){
    if(!unIsString(value)){
        pgError(
            "value is not a string; "
            SRC_LOCATION
        );
    }
    return (UNObjectString*) unAsObject(value);
}

/*
 * Returns a pointer to the C string contained in the
 * specified value, error if the value is not a String
 * object
 */
static inline char *unObjectAsCString(UNValue value){
    return unObjectAsString(value)->string._ptr;
}

/*
 * Creates and returns a new UNObjectString by copying
 * the specified number of characters from the given
 * character pointer
 */
UNObjectString unObjectStringCopy(
    const char *chars,
    size_t length
);

#endif