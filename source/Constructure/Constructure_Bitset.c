#include "Constructure_Bitset.h"

#define blockSize 32

/* 2 levels of macro needed */
#define _BlockType(BLOCKSIZE) uint##BLOCKSIZE##_t
#define _BlockType_(BLOCKSIZE) _BlockType(BLOCKSIZE)
#define BlockType _BlockType_(blockSize)

/* Converts the given bitIndex to its blockIndex */
#define getBlockIndex(BITINDEX) (BITINDEX / blockSize)

/* Constructs and returns a new bitset by value */
Bitset bitsetMake(size_t initBitCapacity){
    Bitset toRet = {0};
    size_t initBlockCapacity
        = initBitCapacity / blockSize;
    toRet._blockArray = arrayListMake(BlockType,
        initBlockCapacity
    );
}

/* 
 * Adds new blocks to the block array of the specified
 * bitset if necessary to access the given bitIndex
 */
static void bitsetGrowIfNeeded(
    Bitset *bitsetPtr,
    size_t bitIndex
){
    size_t blockIndex = getBlockIndex(bitIndex);
    if(blockIndex >= bitsetPtr->_blockArray.size){
        size_t blocksToAdd 
            = blockIndex - bitsetPtr->_blockArray.size
                + 1; /* +1 since zero indexed */
        /* add blocks until blockIndex is valid */
        for(size_t i = 0; i < blocksToAdd; ++i){
            arrayListPushBack(BlockType,
                &(bitsetPtr->_blockArray),
                0
            );
        }
    }
}

/*
 * Sets the specified bit in the given bitset and 
 * returns the previous value of that bit
 */
bool bitsetSet(Bitset *bitsetPtr, size_t bitIndex){
    bitsetGrowIfNeeded(bitsetPtr, bitIndex);
    size_t blockIndex = getBlockIndex(bitIndex);
    BlockType *blockPtr = arrayListGetPtr(BlockType,
        &(bitsetPtr->_blockArray),
        blockIndex
    );
    //todo: probably want a macro to get bit within the block and to calculate which bit that is (%32)
}

/*
 * Unsets the specified bit in the given bitset and
 * returns the previous value of that bit
 */
bool bitsetUnset(Bitset *bitsetPtr, size_t bitIndex);

/*
 * Flips the specified bit in the given bitset and
 * returns the previous value of that bit
 */
bool bitsetFlip(Bitset *bitsetPtr, size_t bitIndex);

/* Gets the specified bit in the given bitset */
bool bitsetGet(Bitset *bitsetPtr, size_t bitIndex);

/* Clears all bits in the given bitset */
void bitsetClear(Bitset *bitsetPtr);

/* 
 * Returns true if any bit is set in the given bitset,
 * false otherwise
 */
bool bitsetAny(Bitset *bitsetPtr);

/* 
 * Returns true if no bits are set in the given bitset,
 * false otherwise
 */
bool bitsetNone(Bitset *bitsetPtr);

/* 
 * Returns the number of bits set in the given
 * bitset
 */
bool bitsetCount(Bitset *bitsetPtr);

/* 
 * Sets the first bitset to the result of performing
 * binary AND with the second
 */
void bitsetAnd(Bitset *bitsetPtr, Bitset *otherPtr);

/* 
 * Sets the first bitset to the result of performing
 * binary OR with the second
 */
void bitsetOr(Bitset *bitsetPtr, Bitset *otherPtr);

/* 
 * Sets the first bitset to the result of performing
 * binary XOR with the second
 */
void bitsetXor(Bitset *bitsetPtr, Bitset *otherPtr);

/* Performs left shift on the given bitset */
void bitsetLeftShift(
    Bitset *bitsetPtr,
    size_t leftShiftAmount
);

/* Performs right shift on the given bitset */
void bitsetRightShift(
    Bitset *bitsetPtr,
    size_t rightShiftAmount
);

/* 
 * Frees the memory associated with the specified
 * bitset
 */
void bitsetFree(Bitset *bitsetPtr){
    arrayListFree(uint32_t, &(bitsetPtr->_intArray));
}