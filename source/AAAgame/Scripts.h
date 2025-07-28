#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "Necro.h"

/*
 * Points to scripts that run in the context of the
 * associated entity; maximum of 4 concurrent
 * scripts
 */
typedef struct Scripts{
    NecroVirtualMachine *vm1;
    NecroVirtualMachine *vm2;
    NecroVirtualMachine *vm3;
    NecroVirtualMachine *vm4;
} Scripts;

/*
 * Initializes the VM pool if it has not already
 * been initialized
 */
void vmPoolInit(
    NecroNativeFuncSet *nativeFuncSetPtr
);

/*
 * Returns a pointer to a fresh virtual machine from
 * the VM pool
 */
NecroVirtualMachine *vmPoolRequest();

/* Reclaims the specified VM pointer into the pool */
void vmPoolReclaim(NecroVirtualMachine *vmPtr);

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