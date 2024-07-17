#ifndef ANIMATION_H
#define ANIMATION_H

#include "Constructure.h"

/* Represents a series of frames that possibly loops */
typedef struct Animation{
    /* an arraylist of String sprite ids */
    ArrayList frameNames;
    /* index of current sprite id in the list */
    size_t currentIndex;
    bool looping;
} Animation;

/* Constructs and returns a new empty animation */
Animation animationMake(bool looping);

/*
 * Adds the sprite specified by the given c string id
 * to the back of the given animation; does not take
 * ownership of the c string
 */
void animationAddFrame(
    Animation *animationPtr,
    const char *spriteID
);

/*
 * Frees the memory associated with the specified
 * Animation
 */
void animationFree(Animation *animationPtr);

/* Represents a series of animations*/
typedef struct AnimationList{
    /* an arraylist of Animation */
    ArrayList animations;
    /* the index of the idle animation in the list */
    size_t idleIndex;
    /* index of current animation in the list */
    size_t currentIndex;
    /*
     * used for timekeeping; starts at 0, ends at
     * max tick exclusive
     */
    int _tick;
    int _maxTick;
} AnimationList;

/* Constructs and returns a new empty animation list */
AnimationList animationListMake();

/*
 * Frees the memory associated with the specified
 * AnimationList
 */
void animationListFree(
    AnimationList *animationListPtr
);

#endif