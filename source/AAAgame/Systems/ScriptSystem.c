#include "ScriptSystem.h"

#include "NativeFuncs.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static Bitset accept;
static bool initialized = false;

/* destroys the script system */
static void destroy(){
    if(initialized){
        bitsetFree(&accept);
        initialized = false;
    }
}

/* inits the script system */
static void init(){
    if(!initialized){
        accept = bitsetMake(numComponents);
        bitsetSet(&accept, ScriptsID);

        registerSystemDestructor(destroy);
        
        initialized = true;
    }
}

/* runs a specified VM */
#define runVM(VMPTRNAME) \
    do{ \
        if(VMPTRNAME){ \
            UNInterpretResult result \
                = unVirtualMachineResume( \
                    VMPTRNAME \
                ); \
            switch(result){ \
                case un_success: \
                    vmPoolReclaim(VMPTRNAME); \
                    VMPTRNAME = NULL; \
                    break; \
                case un_yielded: \
                    break; \
                case un_runtimeError: \
                    pgError( \
                        "halt due to unknown " \
                        "runtime error" \
                    ); \
                    break; \
            } \
        } \
    } while(false)

/* runs scripts on entities */
void scriptSystem(Game *gamePtr, Scene *scenePtr){
    init();

    setGameForNativeFuncs(gamePtr);
    setSceneForNativeFuncs(scenePtr);

    /* get entities with position and velocity */
    WindQueryItr itr = windWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        &accept,
        NULL
    );
    while(windQueryItrHasEntity(&itr)){
        WindEntity handle = windWorldMakeHandle(
            &(scenePtr->ecsWorld),
            windQueryItrCurrentID(&itr)
        );

        setEntityForNativeFuncs(handle);

        Scripts *scriptsPtr = windQueryItrGetPtr(
            Scripts,
            &itr
        );
        
        runVM(scriptsPtr->vm1);
        runVM(scriptsPtr->vm2);
        runVM(scriptsPtr->vm3);
        runVM(scriptsPtr->vm4);

        /*
         * if all scripts are gone, remove the
         * component
         */
        if(!scriptsPtr->vm1 
            && !scriptsPtr->vm2
            && !scriptsPtr->vm3
            && !scriptsPtr->vm4
        ){
            windWorldHandleQueueRemoveComponent(
                Scripts,
                &(scenePtr->ecsWorld),
                handle
            );
        }

        windQueryItrAdvance(&itr);
    }

    windWorldHandleOrders(&(scenePtr->ecsWorld));
}