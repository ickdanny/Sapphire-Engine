#include "ScriptSystem.h"

#include "NativeFuncs.h"

/* apparently Win32 defines "accept" already */
#ifdef WIN32
#define accept _accept
#endif

static VecsComponentSet accept
    = vecsComponentSetFromId(VecsEntityId)
    | vecsComponentSetFromId(ScriptsId);

/* runs a specified VM */
#define runVM(VMPTRNAME) \
    do{ \
        if(VMPTRNAME){ \
            NecroInterpretResult result \
                = necroVirtualMachineResume( \
                    VMPTRNAME \
                ); \
            switch(result){ \
                case necro_success: \
                    vmPoolReclaim(VMPTRNAME); \
                    VMPTRNAME = NULL; \
                    break; \
                case necro_yielded: \
                    break; \
                case necro_runtimeError: \
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
    setGameForNativeFuncs(gamePtr);
    setSceneForNativeFuncs(scenePtr);

    /* get entities with position and velocity */
    VecsQueryItr itr = vecsWorldRequestQueryItr(
        &(scenePtr->ecsWorld),
        accept,
        vecsEmptyComponentSet
    );
    while(vecsQueryItrHasEntity(&itr)){
        VecsEntity entity = vecsQueryItrGet(VecsEntity,
            &itr
        );

        setEntityForNativeFuncs(entity);

        Scripts *scriptsPtr = vecsQueryItrGetPtr(
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
            vecsWorldEntityQueueRemoveComponent(
                Scripts,
                &(scenePtr->ecsWorld),
                entity
            );
        }

        vecsQueryItrAdvance(&itr);
    }

    vecsWorldHandleOrders(&(scenePtr->ecsWorld));
}