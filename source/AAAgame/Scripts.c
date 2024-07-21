#include "Scripts.h"

/*
 * Used to form a singly linked list of virtual
 * machines
 */
typedef struct VMNode{
    UNVirtualMachine vm;
    struct VMNode *next;
} VMNode;

/* Used to allocate new block of virtual machines */
typedef struct VMBlockHandle{
    /* pointer to heap allocated block */
    VMNode *blockPtr;

    /* number of nodes in the block */
    int blockSize;
} VMBlockHandle;

/* The head of the VM pool */
static VMNode *poolHeadPtr = NULL;

/* list of VMBlockHandle */
static ArrayList blockHandleList;

static bool initialized = false;

static UNNativeFuncSet *_nativeFuncSetPtr = NULL;
static UNUserFuncSet *_userFuncSetPtr = NULL;

#define initBlockSize 1000
#define growBlockSize 100

/*
 * Allocates a new block of VMs of the specified size
 * and adds them to the pool
 */
static void addNewBlock(int size){
    assertTrue(
        initialized,
        "vm pool not initialized; " SRC_LOCATION
    );
    assertFalse(
        size < 0,
        "negative size for add new block; "
        SRC_LOCATION
    );
    /* bail if requested 0 VMs */
    if(size == 0){
        return;
    }

    /* allocate a new block of the requested size */
    VMBlockHandle blockHandle = {0};
    blockHandle.blockPtr
        = pgAlloc(size, sizeof(*blockHandle.blockPtr));
    blockHandle.blockSize = size;
    /*
     * add the ptr to the start of the block to the
     * list for freeing later
     */
    arrayListPushBack(VMBlockHandle,
        &blockHandleList,
        blockHandle
    );
    /* initialize and link up the whole block */
    VMNode *currentPtr = blockHandle.blockPtr;
    for(int i = 0; i < size; ++i){
        currentPtr->vm = unVirtualMachineMake(
            _nativeFuncSetPtr,
            _userFuncSetPtr
        );
        currentPtr->next = poolHeadPtr;
        poolHeadPtr = currentPtr;
        ++currentPtr;
    }
}

/*
 * Initializes the VM pool if it has not already
 * been initialized
 */
void vmPoolInit(
    UNNativeFuncSet *nativeFuncSetPtr,
    UNUserFuncSet *userFuncSetPtr
){
    if(initialized){
        return;
    }
    initialized = true;
    blockHandleList = arrayListMake(VMBlockHandle, 10);
    _nativeFuncSetPtr = nativeFuncSetPtr;
    _userFuncSetPtr = userFuncSetPtr;
    
    addNewBlock(initBlockSize);
}

/*
 * Returns a pointer to a fresh virtual machine from
 * the VM pool
 */
UNVirtualMachine *vmPoolRequest(){
    assertTrue(
        initialized,
        "vm pool not initialized; " SRC_LOCATION
    );
    if(!poolHeadPtr){
        addNewBlock(growBlockSize);
    }
    VMNode *nodePtr = poolHeadPtr;
    assertNotNull(
        nodePtr,
        "unexpected null head; " SRC_LOCATION
    );
    poolHeadPtr = nodePtr->next;
    return &(nodePtr->vm);
}

/* Reclaims the specified VM pointer into the pool */
void vmPoolReclaim(UNVirtualMachine *vmPtr){
    if(!vmPtr){
        return;
    }
    assertTrue(
        initialized,
        "vm pool not initialized; " SRC_LOCATION
    );

    VMNode *nodePtr = (VMNode*)vmPtr;
    nodePtr->next = poolHeadPtr;
    poolHeadPtr = nodePtr;
    unVirtualMachineReset(vmPtr);
}

/*
 * Reclaims the virtual machines associated with the
 * specified script component
 */
void scriptsReclaim(Scripts *scriptsPtr){
    if(!scriptsPtr){
        return;
    }
    assertTrue(
        initialized,
        "vm pool not initialized; " SRC_LOCATION
    );

    vmPoolReclaim(scriptsPtr->vm1);
    vmPoolReclaim(scriptsPtr->vm2);
    vmPoolReclaim(scriptsPtr->vm3);
    vmPoolReclaim(scriptsPtr->vm4);

    memset(scriptsPtr, 0, sizeof(*scriptsPtr));
}

/*
 * Frees the vm block held by the specified block
 * handle
 */
static void vmBlockHandleFree(
    VMBlockHandle *handlePtr
){
    /* walk thru the block and free every VM */
    VMNode *nodePtr = handlePtr->blockPtr;
    for(int i = 0; i < handlePtr->blockSize; ++i){
        unVirtualMachineFree(&(nodePtr->vm));
        ++nodePtr;
    }
    pgFree(handlePtr->blockPtr);
    memset(handlePtr, 0, sizeof(*handlePtr));
}

/*
 * Destroys the VM pool if it has not yet been
 * destroyed
 */
void vmPoolDestroy(){
    if(!initialized){
        return;
    }
    poolHeadPtr = NULL;
    arrayListApply(VMBlockHandle,
        &(blockHandleList),
        vmBlockHandleFree
    );
    arrayListFree(VMBlockHandle,
        &(blockHandleList)
    );
    _nativeFuncSetPtr = NULL;
    _userFuncSetPtr = NULL;
    initialized = false;
}