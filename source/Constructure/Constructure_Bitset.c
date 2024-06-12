#include "Constructure_Bitset.h"

#include "ZMath.h"

#define blockSize 32

/* 2 levels of macro needed */
#define _BlockType(BLOCKSIZE) uint##BLOCKSIZE##_t
#define _BlockType_(BLOCKSIZE) _BlockType(BLOCKSIZE)
#define BlockType _BlockType_(blockSize)

/* Converts the given bitIndex to its blockIndex */
#define getBlockIndex(BITINDEX) (BITINDEX / blockSize)

/* 
 * Calculates the subIndex of the given bitIndex
 * into the block
 */
#define getSubIndex(BITINDEX) (BITINDEX % blockSize)

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

    /* find the block containing the bit */
    size_t blockIndex = getBlockIndex(bitIndex);
    BlockType *blockPtr = arrayListGetPtr(BlockType,
        &(bitsetPtr->_blockArray),
        blockIndex
    );

    /* calculate index of the bit within the block */
    size_t subIndex = getSubIndex(bitIndex);

    /* save previous value of bit */
    bool toRet = getBit(*blockPtr, subIndex);

    /* set the bit */
    setBit(*blockPtr, subIndex);

    return toRet;
}

/*
 * Unsets the specified bit in the given bitset and
 * returns the previous value of that bit
 */
bool bitsetUnset(Bitset *bitsetPtr, size_t bitIndex){
    bitsetGrowIfNeeded(bitsetPtr, bitIndex);

    /* find the block containing the bit */
    size_t blockIndex = getBlockIndex(bitIndex);
    BlockType *blockPtr = arrayListGetPtr(BlockType,
        &(bitsetPtr->_blockArray),
        blockIndex
    );

    /* calculate index of the bit within the block */
    size_t subIndex = getSubIndex(bitIndex);

    /* save previous value of bit */
    bool toRet = getBit(*blockPtr, subIndex);

    /* unset the bit */
    unsetBit(*blockPtr, subIndex);

    return toRet;
}

/*
 * Flips the specified bit in the given bitset and
 * returns the previous value of that bit
 */
bool bitsetFlip(Bitset *bitsetPtr, size_t bitIndex){
    bitsetGrowIfNeeded(bitsetPtr, bitIndex);

    /* find the block containing the bit */
    size_t blockIndex = getBlockIndex(bitIndex);
    BlockType *blockPtr = arrayListGetPtr(BlockType,
        &(bitsetPtr->_blockArray),
        blockIndex
    );

    /* calculate index of the bit within the block */
    size_t subIndex = getSubIndex(bitIndex);

    /* save previous value of bit */
    bool toRet = getBit(*blockPtr, subIndex);

    /* flip the bit */
    flipBit(*blockPtr, subIndex);

    return toRet;
}

/* Gets the specified bit in the given bitset */
bool bitsetGet(Bitset *bitsetPtr, size_t bitIndex){
    bitsetGrowIfNeeded(bitsetPtr, bitIndex);

    /* find the block containing the bit */
    size_t blockIndex = getBlockIndex(bitIndex);
    BlockType *blockPtr = arrayListGetPtr(BlockType,
        &(bitsetPtr->_blockArray),
        blockIndex
    );

    /* calculate index of the bit within the block */
    size_t subIndex = getSubIndex(bitIndex);

    /* return value of bit */
    return getBit(*blockPtr, subIndex);
}

/* Clears all bits in the given bitset */
void bitsetClear(Bitset *bitsetPtr){
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );

    /* set all bytes in the block array to 0 */
    memset(
        frontPtr,
        0,
        sizeof(BlockType) * bitsetPtr->_blockArray.size
    );
}

/* 
 * Returns true if any bit is set in the given bitset,
 * false otherwise
 */
bool bitsetAny(Bitset *bitsetPtr){
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );

    /* check each block */
    for(size_t i = 0;
        i < bitsetPtr->_blockArray.size;
        ++i
    ){
        if(frontPtr[i] != 0){
            return true;
        }
    }

    return false;
}

/* Returns the number of bits set in the given block */
static size_t blockCount(BlockType block){
    /* Kernighan's algorithm */
    size_t count = 0;
    while(block != 0){
        block = block & (block - 1);
        ++count;
    }
    return count;
}

/* 
 * Returns the number of bits set in the given
 * bitset
 */
size_t bitsetCount(Bitset *bitsetPtr){
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );

    /* count number of set bits in each block */
    size_t count = 0;
    for(size_t i = 0;
        i < bitsetPtr->_blockArray.size;
        ++i
    ){
        count += blockCount(frontPtr[i]);
    }

    return count;
}

/* 
 * Sets the first bitset to the result of performing
 * bitwise AND with the second
 */
void bitsetAnd(Bitset *bitsetPtr, Bitset *otherPtr){
    size_t leftBlockCount
        = bitsetPtr->_blockArray.size;
    size_t rightBlockCount
        = otherPtr->_blockArray.size;

    /* 
     * if first bitset has less blocks than the second,
     * add new blocks until they are the same size
     */
    if(leftBlockCount < rightBlockCount){
        size_t blockCountDiff
            = rightBlockCount - leftBlockCount;
        for(size_t i = 0; i < blockCountDiff; ++i){
            arrayListPushBack(BlockType,
                &(bitsetPtr->_blockArray),
                (BlockType)0
            );
        }
    }

    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );
    BlockType *otherFrontPtr = arrayListFrontPtr(
        BlockType,
        &(otherPtr->_blockArray)
    );

    /*
     * if the first bitset has more blocks than the
     * second, the result of bitwise AND will be to
     * zero those blocks
     */
    if(leftBlockCount > rightBlockCount){
        size_t blockCountDiff
            = leftBlockCount - rightBlockCount;
        /* zero bytes after right block count */
        memset(
            frontPtr + rightBlockCount,
            0,
            sizeof(BlockType) * blockCountDiff
        );
    }

    /* 
     * perform bitwise AND on blocks present in both
     * bitsets; we can use rightBlockCount since
     * we pad with 0 blocks in case left < right
     */
    for(size_t i = 0; i < rightBlockCount; ++i){
        frontPtr[i] &= otherFrontPtr[i];
    }
}

/* 
 * Sets the first bitset to the result of performing
 * bitwise OR with the second
 */
void bitsetOr(Bitset *bitsetPtr, Bitset *otherPtr){
    size_t leftBlockCount
        = bitsetPtr->_blockArray.size;
    size_t rightBlockCount
        = otherPtr->_blockArray.size;

    /* 
     * if first bitset has less blocks than the second,
     * add new blocks until they are the same size
     */
    if(leftBlockCount < rightBlockCount){
        size_t blockCountDiff
            = rightBlockCount - leftBlockCount;
        for(size_t i = 0; i < blockCountDiff; ++i){
            arrayListPushBack(BlockType,
                &(bitsetPtr->_blockArray),
                (BlockType)0
            );
        }
    }

    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );
    BlockType *otherFrontPtr = arrayListFrontPtr(
        BlockType,
        &(otherPtr->_blockArray)
    );

    /*
     * if the first bitset has more blocks than the
     * second, the result of bitwise OR will be to
     * leave those blocks the same; do nothing in
     * that case
     */

    /* 
     * perform bitwise OR on blocks present in both
     * bitsets; we can use rightBlockCount since
     * we pad with 0 blocks in case left < right
     */
    for(size_t i = 0; i < rightBlockCount; ++i){
        frontPtr[i] |= otherFrontPtr[i];
    }
}

/* 
 * Sets the first bitset to the result of performing
 * bitwise XOR with the second
 */
void bitsetXor(Bitset *bitsetPtr, Bitset *otherPtr){
    size_t leftBlockCount
        = bitsetPtr->_blockArray.size;
    size_t rightBlockCount
        = otherPtr->_blockArray.size;

    /* 
     * if first bitset has less blocks than the second,
     * add new blocks until they are the same size
     */
    if(leftBlockCount < rightBlockCount){
        size_t blockCountDiff
            = rightBlockCount - leftBlockCount;
        for(size_t i = 0; i < blockCountDiff; ++i){
            arrayListPushBack(BlockType,
                &(bitsetPtr->_blockArray),
                (BlockType)0
            );
        }
    }

    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );
    BlockType *otherFrontPtr = arrayListFrontPtr(
        BlockType,
        &(otherPtr->_blockArray)
    );

    /*
     * if the first bitset has more blocks than the
     * second, the result of bitwise XOR will be to
     * leave those blocks the same; do nothing in
     * that case
     */

    /* 
     * perform bitwise XOR on blocks present in both
     * bitsets; we can use rightBlockCount since
     * we pad with 0 blocks in case left < right
     */
    for(size_t i = 0; i < rightBlockCount; ++i){
        frontPtr[i] ^= otherFrontPtr[i];
    }
}

/* Performs left shift on the given bitset */
void bitsetLeftShift(
    Bitset *bitsetPtr,
    size_t leftShiftAmount
){
    /* bail if size is 0 */
    size_t oldBlockSize = bitsetPtr->_blockArray.size;
    if(oldBlockSize == 0){
        return;
    }
    /* bail if bitset was all 0s */
    if(bitsetNone(bitsetPtr)){
        return;
    }

    size_t blockShiftAmount
        = leftShiftAmount / blockSize;
    size_t subShiftAmount
        = leftShiftAmount % blockSize;
    
    /* 
     * add zeroed blocks to start of block array
     * for each block shift
     */
    if(blockShiftAmount > 0){
        /* add zeroed blocks to end */
        for(size_t i = 0; i < blockShiftAmount; ++i){
            arrayListPushBack(BlockType,
                &(bitsetPtr->_blockArray),
                (BlockType)0
            );
        }

        /* copy data into later blocks */
        BlockType *frontPtr = arrayListFrontPtr(BlockType,
            &(bitsetPtr->_blockArray)
        );
        /*
         * going backwards starting at last originally
         * present index, stopping after index 0
         */
        for(size_t i = oldBlockSize - 1;
            i != SIZE_MAX;
            --i
        ){
            frontPtr[i + blockShiftAmount]
                = frontPtr[i];
        }

        /* zero out blocks at the start */
        memset(
            frontPtr,
            0,
            sizeof(BlockType) * blockShiftAmount
        );
    }

    /* shift bits between blocks for subshift */
    if(subShiftAmount > 0){
        /* 
         * sub shift may require 1 additional block,
         * which is added if the last block is nonempty
         */
        if(arrayListBack(BlockType, 
            &(bitsetPtr->_blockArray)) != 0
        ){
            arrayListPushBack(BlockType,
                &(bitsetPtr->_blockArray),
                (BlockType)0
            );
        }
        /* 
         * going backwards, first shift the block,
         * then copy bits over from the previous
         * block 
         */
        //todo
    }
}

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
    arrayListFree(blockType,
        &(bitsetPtr->_blockArray)
    );
}