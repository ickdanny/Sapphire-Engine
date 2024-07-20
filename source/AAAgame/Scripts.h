#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "Unknown.h"

/*
 * Points to scripts that run in the context of the
 * associated entity; maximum of 4 concurrent
 * scripts
 */
typedef struct Scripts{
    UNVirtualMachine *vm1;
    UNVirtualMachine *vm2;
    UNVirtualMachine *vm3;
    UNVirtualMachine *vm4;
} Scripts;

/*
 * Initializes the VM pool if it has not already
 * been initialized
 */
void vmPoolInit(
    UNNativeFuncSet *nativeFuncSetPtr,
    UNUserFuncSet *userFuncSetPtr
);

/*
 * Returns a pointer to a fresh virtual machine from
 * the VM pool
 */
UNVirtualMachine *vmPoolRequest();

/* Reclaims the specified VM pointer into the pool */
void vmReclaim(UNVirtualMachine *vmPtr);

/*
 * Reclaims the virtual machines associated with the
 * specified script component
 */
void scriptsReclaim(Scripts *scriptsPtr);

/*
 * Destroys the VM pool if it has not yet been
 * destroyed
 */
void vmPoolDestroy();

#endif