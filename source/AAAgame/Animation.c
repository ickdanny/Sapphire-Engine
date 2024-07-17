#include "Animation.h"

/* Constructs and returns a new empty animation */
Animation animationMake(bool looping){
    Animation toRet = {0};
    toRet.frameNames = arrayListMake(String, 10);
    toRet.currentIndex = 0;
    toRet.looping = looping;

    return toRet;
}

/*
 * Adds the sprite specified by the given c string id
 * to the back of the given animation; does not take
 * ownership of the c string
 */
void animationAddFrame(
    Animation *animationPtr,
    const char *spriteID
){
    String stringID = stringMakeC(spriteID);
    arrayListPushBack(String,
        &(animationPtr->frameNames),
        stringID
    );
}

/*
 * Frees the memory associated with the specified
 * Animation
 */
void animationFree(Animation *animationPtr){
    arrayListApply(String,
        &(animationPtr->frameNames),
        stringFree
    );
    arrayListFree(String, &(animationPtr->frameNames));
    memset(animationPtr, 0, sizeof(*animationPtr));
}

/* Constructs and returns a new empty animation list */
AnimationList animationListMake(){
    AnimationList toRet = {0};
    toRet.animations = arrayListMake(Animation, 10);
    return toRet;
}

/*
 * Frees the memory associated with the specified
 * AnimationList
 */
void animationListFree(
    AnimationList *animationListPtr
){
    arrayListApply(Animation,
        &(animationListPtr->animations),
        animationFree
    );
    arrayListFree(Animation,
        &(animationListPtr->animations)
    );
    memset(
        animationListPtr,
        0,
        sizeof(*animationListPtr)
    );
}