#ifndef SYSTEMDESTRUCTORS_H
#define SYSTEMDESTRUCTORS_H

/* a void() function used as a system destructor */
typedef void(*SystemDestructorType)();

/*
 * Frees all memory associated with every system
 */
void freeSystems();

/*
 * Used by systems to register their destruction
 * function
 */
void registerSystemDestructor(
    SystemDestructorType destructor
);

#endif