#include "Components.h"

/*
 * Allocates and returns a new WindComponents object
 * containing the RTTI details of every component
 * used in the game engine
 */
WindComponents *componentsMake(){
    WindComponents *toRet = pgAlloc(1, sizeof(*toRet));
    *toRet = windComponentsMake(numComponents);

    #define insertComponent(TYPENAME, DESTRUCTOR) \
        windComponentsInsert( \
            toRet, \
            windComponentMetadataMake( \
                TYPENAME, \
                DESTRUCTOR \
            ), \
            TYPENAME##ID \
        );

    insertComponent(Position, NULL);

    return toRet;

    #undef insertComponent
}