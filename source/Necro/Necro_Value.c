#include "Necro_Value.h"
#include "Necro_Object.h"

#include "PGUtil.h"

/*
 * Returns true if the two specified values are equal,
 * false otherwise
 */
bool necroValueEquals(NecroValue a, NecroValue b){
    if(a.type != b.type){
        return false;
    }
    switch(a.type){
        case necro_bool:
            return necroAsBool(a) == necroAsBool(b);
        case necro_int:
            return necroAsInt(a) == necroAsInt(b);
        case necro_float:
            return necroAsFloat(a) == necroAsFloat(b);
        case necro_vector: {
            Polar vecA = necroAsVector(a);
            Polar vecB = necroAsVector(b);
            return vecA.magnitude == vecB.magnitude
                && vecA.angle == vecB.angle;
        }
        case necro_point: {
            Point2D pointA = necroAsPoint(a);
            Point2D pointB = necroAsPoint(b);
            return pointA.x == pointB.x
                && pointA.y == pointB.y;
        }
        case necro_object:
            return necroObjectEquals(
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
void necroValuePrint(NecroValue value){
    #define bufferSize 32
    static char buffer[bufferSize] = {0};
    switch(value.type){
        case necro_bool:
            printf(necroAsBool(value) ? "true" : "false");
            break;
        case necro_int:
            printf("%d", necroAsInt(value));
            break;
        case necro_float:
            printf("%.4f", necroAsFloat(value));
            break;
        case necro_vector: {
            Polar vector = necroAsVector(value);
            printPolar(vector, buffer, bufferSize);
            printf("%s", buffer);
            break;
        }
        case necro_point: {
            Point2D point = necroAsPoint(value);
            printPoint2D(point, buffer, bufferSize);
            printf("%s", buffer);
            break;
        }
        case necro_object:
            necroObjectPrint(value);
            break;
        case necro_invalidValue:
            printf("invalid value");
            break;
        default:
            pgError(
                "unexpected default; "
                SRC_LOCATION
            );
            break;
    }
    #undef bufferSize
}

/*
 * Frees the memory associated with the specified value
 */
void necroValueFree(NecroValue value){
    switch(value.type){
        case necro_object:
            necroObjectFree(value.as.object);
            break;
        default:
            /* do nothing */
            return;
    }
}