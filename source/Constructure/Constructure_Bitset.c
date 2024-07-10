#include "Constructure_Bitset.h"

#include "ZMath.h"
#include "PGUtil.h"

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

/* The block with all bits set */
#define fullBlock (~((BlockType)0))

/* Constructs and returns a new bitset by value */
Bitset bitsetMake(size_t initBitCapacity){
    Bitset toRet = {0};
    size_t initBlockCapacity
        = (initBitCapacity / blockSize) + 1;
    toRet._blockArray = arrayListMake(BlockType,
        initBlockCapacity
    );
    for(int i = 0; i < initBlockCapacity; ++i){
        arrayListPushBack(BlockType,
            &(toRet._blockArray),
            0
        );
    }
    return toRet;
}

/* 
 * Makes a deep copy of the given bitset 
 * and returns it by value
 */
Bitset bitsetCopy(const Bitset *toCopyPtr){
    Bitset toRet = {0};
    toRet._blockArray = arrayListCopy(BlockType,
        &(toCopyPtr->_blockArray)
    );
    return toRet;
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
 * Returns the index of the first set bit in the
 * given bitset or -1 if no bits are set
 */
int bitsetFirstSet(Bitset *bitsetPtr){
    /* return -1 if array is empty */
    size_t blockArraySize
        = bitsetPtr->_blockArray.size;
    if(blockArraySize == 0){
        return -1;
    }

    /* find first set block */
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );
    size_t firstSetBlockIndex = 0;
    for(firstSetBlockIndex = 0;
        firstSetBlockIndex < blockArraySize;
        ++firstSetBlockIndex
    ){
        if(frontPtr[firstSetBlockIndex] != 0){
            break;
        }
    }
    /* return -1 if no set blocks */
    if(firstSetBlockIndex == blockArraySize){
        return -1;
    }

    /* find first set bit in the block */
    BlockType firstSetBlock
        = frontPtr[firstSetBlockIndex];
    size_t firstSetBitSubIndex = 0;
    for(firstSetBitSubIndex = 0;
        firstSetBitSubIndex < blockSize;
        ++firstSetBitSubIndex
    ){
        if(getBit(firstSetBlock, firstSetBitSubIndex)){
            break;
        }
    }
    assertFalse(
        firstSetBitSubIndex == blockSize,
        "failed to find first bit when block was set"
        SRC_LOCATION
    );

    /* calculate total index and return */
    return (firstSetBlockIndex * blockSize)
        + firstSetBitSubIndex;
}

/*
 * Returns the index of the last set bit in the
 * given bitset or -1 if no bits are set
 */
int bitsetLastSet(Bitset *bitsetPtr){
    /* return -1 if array is empty */
    if(bitsetPtr->_blockArray.size == 0){
        return -1;
    }

    /* find last set block */
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );
    size_t lastBlockIndex 
        = bitsetPtr->_blockArray.size - 1;
    size_t lastSetBlockIndex = 0;
    for(lastSetBlockIndex = lastBlockIndex;
        lastSetBlockIndex != SIZE_MAX;
        --lastSetBlockIndex
    ){
        if(frontPtr[lastSetBlockIndex] != 0){
            break;
        }
    }
    /* return -1 if no set blocks */
    if(lastSetBlockIndex == SIZE_MAX){
        return -1;
    }

    /* find last set bit in the block */
    BlockType lastSetBlock
        = frontPtr[lastSetBlockIndex];
    size_t lastSetBitSubIndex = 0;
    for(lastSetBitSubIndex = blockSize - 1;
        lastSetBitSubIndex != SIZE_MAX;
        --lastSetBitSubIndex
    ){
        if(getBit(lastSetBlock, lastSetBitSubIndex)){
            break;
        }
    }
    assertFalse(
        lastSetBitSubIndex == SIZE_MAX,
        "failed to find last bit when block was set"
    );

    /* calculate total index and return */
    return (lastSetBlockIndex * blockSize)
        + lastSetBitSubIndex;
}

/*
 * Returns the index of the first unset bit in the
 * given bitset
 */
int bitsetFirstUnset(Bitset *bitsetPtr){
    /* return 0 if array is empty since it's unset */
    size_t blockArraySize
        = bitsetPtr->_blockArray.size;
    if(blockArraySize == 0){
        return 0;
    }

    /* find first non-full block */
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );
    size_t firstNonFullBlockIndex = 0;
    for(firstNonFullBlockIndex = 0;
        firstNonFullBlockIndex < blockArraySize;
        ++firstNonFullBlockIndex
    ){
        if(frontPtr[firstNonFullBlockIndex]
            != fullBlock
        ){
            break;
        }
    }
    /*
     * return the first bit after the last block if all
     * blocks are full
     */
    if(firstNonFullBlockIndex == blockArraySize){
        return (firstNonFullBlockIndex * blockSize)
            + 1;
    }

    /* find first unset bit in the block */
    BlockType firstNonFullBlock
        = frontPtr[firstNonFullBlockIndex];
    size_t firstUnsetBitSubIndex = 0;
    for(firstUnsetBitSubIndex = 0;
        firstUnsetBitSubIndex < blockSize;
        ++firstUnsetBitSubIndex
    ){
        if(!getBit(
            firstNonFullBlock,
            firstUnsetBitSubIndex
        )){
            break;
        }
    }
    assertFalse(
        firstUnsetBitSubIndex == blockSize,
        "failed to find first unset bit when block "
        "was not full"
        SRC_LOCATION
    );

    /* calculate total index and return */
    return (firstNonFullBlockIndex * blockSize)
        + firstUnsetBitSubIndex;
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
    size_t oldNumBlocks = bitsetPtr->_blockArray.size;
    if(oldNumBlocks == 0){
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
        BlockType *frontPtr = arrayListFrontPtr(
            BlockType,
            &(bitsetPtr->_blockArray)
        );
        /*
         * going backwards starting at last originally
         * present index, stopping after index 0
         */
        for(size_t i = oldNumBlocks - 1;
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
        if(*(arrayListBackPtr(BlockType,
            &(bitsetPtr->_blockArray))) != 0
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
        BlockType *frontPtr = arrayListFrontPtr(
            BlockType,
            &(bitsetPtr->_blockArray)
        );
        for(size_t i = bitsetPtr->_blockArray.size - 1;
            i > 0;
            --i
        ){
            /* shift block left */
            frontPtr[i] <<= subShiftAmount;
            /* copy bits from prev block */
            frontPtr[i] |= (frontPtr[i - 1]
                >> (blockSize - subShiftAmount));
        }
        /* finally shift first block */
        frontPtr[0] <<= subShiftAmount;
    }
}

/* Performs right shift on the given bitset */
void bitsetRightShift(
    Bitset *bitsetPtr,
    size_t rightShiftAmount
){
    /* bail if size is 0 */
    size_t numBlocks = bitsetPtr->_blockArray.size;
    if(numBlocks == 0){
        return;
    }
    /* bail if bitset was all 0s */
    if(bitsetNone(bitsetPtr)){
        return;
    }

    size_t blockShiftAmount
        = rightShiftAmount / blockSize;
    size_t subShiftAmount
        = rightShiftAmount % blockSize;

    /* if block shift >= numBlocks, simply clear */
    if(blockShiftAmount >= numBlocks){
        bitsetClear(bitsetPtr);
        return;
    }

    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );

    /* 
     * destroy blocks at start of block array for each
     * block shift
     */
    if(blockShiftAmount > 0){
        /* copy data into earlier blocks */
        for(size_t i = 0;
            i < numBlocks - blockShiftAmount;
            ++i
        ){
            frontPtr[i]
                = frontPtr[i + blockShiftAmount];
        }

        /* zero out blocks at the end */
        memset(
            frontPtr + (numBlocks - blockShiftAmount),
            0,
            sizeof(BlockType) * blockShiftAmount
        );
    }

    /* shift bits between blocks for subshift */
    if(subShiftAmount > 0){
        /* 
         * going forwards, first shift the block,
         * then copy bits over from the next
         * block 
         */
        size_t i = 0;
        for(i = 0;
            i < (bitsetPtr->_blockArray.size - 1);
            ++i
        ){
            /* shift block right */
            frontPtr[i] >>= subShiftAmount;
            /* copy bits from next block */
            frontPtr[i] |= (frontPtr[i + 1]
                << (blockSize - subShiftAmount));
        }
        /* finally shift last block */
        frontPtr[i] <<= subShiftAmount;
    }
}

/* 
 * Frees the memory associated with the specified
 * bitset
 */
void bitsetFree(Bitset *bitsetPtr){
    arrayListFree(BlockType,
        &(bitsetPtr->_blockArray)
    );
}

/* Prints the given bitset to the given C String */
void printBitset(
    Bitset *bitsetPtr,
    char* charPtr,
    int arraySize
){
    assertNotNull(
        bitsetPtr,
        "null bitset ptr in printBitset"
    );
    assertNotNull(
        bitsetPtr,
        "null string ptr in printBitset"
    );

    /* get index of last bit */
    int lastSetBitIndex = bitsetLastSet(bitsetPtr);
    /* if no bits, write '0' */
    if(lastSetBitIndex == -1){
        snprintf(charPtr, arraySize, "0");
        return;
    }
    int lastSetBlockIndex = getBlockIndex(
        lastSetBitIndex
    );
    int lastSetBitSubIndex = getSubIndex(
        lastSetBitIndex
    );

    size_t numBitsToPrint = lastSetBitSubIndex + 1
        + (lastSetBlockIndex * blockSize);
    
    /* allocate space to write n number of bits */
    char *tempStorage = pgAlloc(
        numBitsToPrint + 1, /* for null terminator */
        sizeof(char)
    );
    size_t nextCharToWriteIndex = 0;

    /* write the bits in the last set block */
    for(size_t i = lastSetBitSubIndex;
        i != SIZE_MAX;
        --i
    ){
        if(getBit(lastSetBlockIndex, i)){
            tempStorage[nextCharToWriteIndex++] = '1';
        }
        else{
            tempStorage[nextCharToWriteIndex++] = '0';
        }
    }

    /* for each other block, write bits backwards */
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(bitsetPtr->_blockArray)
    );
    BlockType currentBlock = 0;
    for(size_t blockIndex = lastSetBlockIndex - 1;
        blockIndex != SIZE_MAX;
        --blockIndex
    ){
        currentBlock = frontPtr[blockIndex];
        for(size_t i = blockSize - 1;
            i != SIZE_MAX;
            --i
        ){
            if(getBit(currentBlock, i)){
                tempStorage[nextCharToWriteIndex++]
                    = '1';
            }
            else{
                tempStorage[nextCharToWriteIndex++]
                    = '0';
            }
        }
    }

    /* print to string */
    snprintf(
        charPtr,
        arraySize,
        "%s",
        tempStorage
    );

    /* free temp storage */
    pgFree(tempStorage);
}

/* implementation of hash and equals functions */

/* hash func for bitsets */
size_t bitsetHash(const void *bitsetPtr){
    static size_t prime = 53;
    static size_t modulo = 1294967281;
    size_t hash = 0;

    Bitset *castPtr = (Bitset *)bitsetPtr;
    
    /* return 0 if bitset array is empty */
    if(castPtr->_blockArray.size == 0){
        return 0;
    }

    /* find last set block */
    BlockType *frontPtr = arrayListFrontPtr(BlockType,
        &(castPtr->_blockArray)
    );
    size_t lastBlockIndex 
        = castPtr->_blockArray.size - 1;
    size_t lastSetBlockIndex = 0;
    for(lastSetBlockIndex = lastBlockIndex;
        lastSetBlockIndex != SIZE_MAX;
        --lastSetBlockIndex
    ){
        if(frontPtr[lastSetBlockIndex] != 0){
            break;
        }
    }
    /* return 0 if no set blocks */
    if(lastSetBlockIndex == SIZE_MAX){
        return 0;
    }

    /* start hashing up to the last set block */
    for(size_t i = 0; i <= lastBlockIndex; ++i){
        hash += frontPtr[i];
        ++hash;
        hash *= prime;
        hash %= modulo;
    }

    return hash;
}

/* equals func for bitsets */
bool bitsetEquals(
    const void *bitsetPtr1,
    const void *bitsetPtr2
){
    Bitset *castPtr1 = (Bitset *)bitsetPtr1;
    Bitset *castPtr2 = (Bitset *)bitsetPtr2;

    int lastSetBitIndex1 = bitsetLastSet(castPtr1);
    int lastSetBitIndex2 = bitsetLastSet(castPtr2);

    /*
     * if first bitset is empty, return true if second
     * is also empty, false otherwise
     */
    if(lastSetBitIndex1 == -1){
        return (lastSetBitIndex2 == -1);
    }

    /*
     * if their last bits are different, the bitsets
     * are clearly not equal
     */
    if(lastSetBitIndex1 != lastSetBitIndex2){
        return false;
    }

    /* the last set block in both bitsets */
    int lastSetBlockIndex
        = getBlockIndex(lastSetBitIndex1);

    BlockType *frontPtr1 = arrayListFrontPtr(BlockType,
        &(castPtr1->_blockArray)
    );
    BlockType *frontPtr2 = arrayListFrontPtr(BlockType,
        &(castPtr2->_blockArray)
    );

    /* check each block for equality */
    for(int i = 0; i <= lastSetBlockIndex; ++i){
        if(frontPtr1[i] != frontPtr2[i]){
            return false;
        }
    }
    return true;
}