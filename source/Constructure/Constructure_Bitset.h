#ifndef CONSTRUCTURE_BITSET
#define CONSTRUCTURE_BITSET

#include "Constructure_ArrayList.h"

/* 
 * A bitset stores an unbounded set of bit flags
 * indexed by non-negative integers; bits grow to the
 * left
 */
typedef struct Bitset{
    ArrayList _blockArray;
} Bitset;

/* Constructs and returns a new bitset by value */
Bitset bitsetMake(size_t initBitCapacity);

//todo: bitset copy

/*
 * Sets the specified bit in the given bitset and 
 * returns the previous value of that bit
 */
bool bitsetSet(Bitset *bitsetPtr, size_t bitIndex);

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
#define bitsetNone(bitsetPtr) (!(bitsetAny(bitsetPtr)))

/* 
 * Returns the number of bits set in the given
 * bitset
 */
size_t bitsetCount(Bitset *bitsetPtr);

/* 
 * Sets the first bitset to the result of performing
 * bitwise AND with the second
 */
void bitsetAnd(Bitset *bitsetPtr, Bitset *otherPtr);

/* 
 * Sets the first bitset to the result of performing
 * bitwise OR with the second
 */
void bitsetOr(Bitset *bitsetPtr, Bitset *otherPtr);

/* 
 * Sets the first bitset to the result of performing
 * bitwise XOR with the second
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
void bitsetFree(Bitset *bitsetPtr);

/* Prints the given bitset to the given C String */
void printBitset(
    Bitset *bitsetPtr,
    char* charPtr,
    int arraySize
);

#endif