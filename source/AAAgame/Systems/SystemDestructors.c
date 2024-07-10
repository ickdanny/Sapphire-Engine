#include "SystemDestructors.h"

#include "Constructure.h"

static ArrayList systemDestructors;
static bool initialized;

/* initializes the system destructor list */
static void init(){
    if(!initialized){
        systemDestructors = arrayListMake(
            SystemDestructorType,
            50
        );
        initialized = true;
    }
}

/*
 * Frees all memory associated with every system
 */
void freeSystems(){
    if(initialized){
        for(size_t i = 0;
            i < systemDestructors.size;
            ++i
        ){
            SystemDestructorType destructor
                = arrayListGet(SystemDestructorType,
                    &systemDestructors,
                    i
                );
            destructor();
        }
        arrayListFree(SystemDestructorType,
            &systemDestructors
        );
        initialized = false;
    }
}

/*
 * Used by systems to register their destruction
 * function
 */
void registerSystemDestructor(
    SystemDestructorType destructor
){
    init();
    arrayListPushBack(SystemDestructorType,
        &systemDestructors,
        destructor
    );
}