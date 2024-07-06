#include "Unknown_Value.h"
#include "Unknown_Object.h"

#include "PGUtil.h"

/*
 * Returns true if the two specified values are equal,
 * false otherwise
 */
bool unValueEquals(UNValue a, UNValue b){
    if(a.type != b.type){
        return false;
    }
    switch(a.type){
        case un_bool:
            return unAsBool(a) == unAsBool(b);
        case un_int:
            return unAsInt(a) == unAsInt(b);
        case un_float:
            return unAsFloat(a) == unAsFloat(b);
        case un_object:
            return unObjectEquals(
                a.as.object,
                b.as.object
            );
        default:
            pgError(
                "unexpected default in value equals; "
                SRC_LOCATION
            );
            return false;
    }
}

/* prints the specified value to stdout */
void unValuePrint(UNValue value){
    switch(value.type){
        case un_bool:
            printf(unAsBool(value) ? "true" : "false");
            break;
        case un_int:
            printf("%d", unAsInt(value));
            break;
        case un_float:
            printf("%.4f", unAsFloat(value));
            break;
        case un_object:
            unObjectPrint(value);
            break;
        case un_invalidValue:
            printf("invalid value");
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
 * Frees the memory associated with the specified value
 */
void unValueFree(UNValue value){
    switch(value.type){
        case un_object:
            unObjectFree(value.as.object);
            break;
        default:
            /* do nothing */
            return;
    }
}