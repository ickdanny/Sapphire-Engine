#include "Components.h"

void textInstructionDestructor(void *voidPtr){
    TextInstruction *textInstructionPtr = voidPtr;

    if(textInstructionPtr->text._ptr){
        wideStringFree(&(textInstructionPtr->text));
    }
    memset(
        textInstructionPtr,
        0,
        sizeof(*textInstructionPtr)
    );
}

void animationsDestructor(void *voidPtr){
    animationListFree((AnimationList*)voidPtr);
}

void scriptsDestructor(void *voidPtr){
    scriptsReclaim((Scripts*)voidPtr);
}

void deathScriptsDestructor(void *voidPtr){
    DeathScripts *deathScriptsPtr = voidPtr;

    #define freeStringIfAllocated(SLOT) \
        do{ \
            if(deathScriptsPtr->scriptID##SLOT._ptr){ \
                stringFree( \
                    &(deathScriptsPtr \
                        ->scriptID##SLOT) \
                ); \
            } \
        } while(false)
    
    freeStringIfAllocated(1);
    freeStringIfAllocated(2);
    freeStringIfAllocated(3);
    freeStringIfAllocated(4);

    #undef freeStringIfAllocated
}

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
        )
    
    #define insertMarker(NAME) \
        windComponentsInsert( \
            toRet, \
            _windComponentMetadataMake( \
                0, /* size 0 for markers */ \
                NULL, /* no destructor for markers */ \
                #NAME /* type name */ \
            ), \
            NAME##ID \
        )

    insertComponent(Position, NULL);
    insertComponent(Velocity, NULL);
    insertMarker(VisibleMarker);
    insertComponent(SpriteInstruction, NULL);
    insertComponent(SubImage, NULL);
    insertComponent(TilingInstruction, NULL);
    insertComponent(TileScroll, NULL);
    insertComponent(
        TextInstruction,
        textInstructionDestructor
    );
    insertMarker(RotateSpriteForwardMarker);
    insertComponent(SpriteSpin, NULL);
    insertComponent(Animations, animationsDestructor);
    insertComponent(MenuCommands, NULL);
    insertComponent(NeighborElements, NULL);
    insertComponent(ButtonData, NULL);
    insertComponent(Inbound, NULL);
    insertComponent(Outbound, NULL);
    insertComponent(PlayerData, NULL);
    insertMarker(CollidableMarker);
    insertComponent(Hitbox, NULL);
    insertComponent(Health, NULL);
    insertComponent(Damage, NULL);
    insertComponent(PlayerCollisionSource, NULL);
    insertComponent(PlayerCollisionTarget, NULL);
    insertComponent(EnemyCollisionSource, NULL);
    insertComponent(EnemyCollisionTarget, NULL);
    insertComponent(BulletCollisionSource, NULL);
    insertComponent(BulletCollisionTarget, NULL);
    insertComponent(PickupCollisionSource, NULL);
    insertComponent(PickupCollisionTarget, NULL);
    insertComponent(PowerGain, NULL);
    insertMarker(ClearableMarker);
    insertComponent(Scripts, scriptsDestructor);
    insertComponent(DeathCommand, NULL);
    insertComponent(
        DeathScripts,
        deathScriptsDestructor
    );

    return toRet;

    #undef insertComponent
    #undef insertMarker
}