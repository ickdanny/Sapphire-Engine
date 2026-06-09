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
static void runVM(NecroVirtualMachine **vmPtrPtr){
    if(vmPtrPtr && *vmPtrPtr){
        NecroVirtualMachine *vmPtr = *vmPtrPtr;
        NecroInterpretResult result
            = necroVirtualMachineResume(
                vmPtr
            );
        switch(result){
            case necro_success:
                vmPoolReclaim(vmPtr);
                *vmPtrPtr = NULL;
                break;
            case necro_yielded:
                break;
            case necro_runtimeError:
                pgError(
                    "halt due to unknown "
                    "runtime error"
                );
                break;
        }
    }
}

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
        
        for(int i = 0; i < SCRIPTS_NUM_VMS; ++i){
            runVM(&scriptsPtr->vms[i]);
        }

        /*
         * if all scripts are gone, remove the
         * component
         */
        bool allScriptsGone = true;
        for(int i = 0; i < SCRIPTS_NUM_VMS; ++i){
            if(scriptsPtr->vms[i]){
                allScriptsGone = false;
                break;
            }
        }
        if(allScriptsGone){
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