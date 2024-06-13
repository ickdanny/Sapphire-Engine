#ifndef ZMATH_MT
#define ZMATH_MT

#include <stdint.h>
#include <stdbool.h>

/* A Mersenne Twister */
typedef struct ZMT{
    uint32_t *stateArray;
    int stateIndex;
} ZMT;

/*
 * Initializes and returns a new Mersenne Twister
 * by value, seeded with the given integer
 */
ZMT zmtMake(uint32_t seed);

/*
 * Generates a random char from the given 
 * Mersenne Twister
 */
int8_t zmtRandChar(ZMT *zmtPtr);

/*
 * Generates a random char from the given
 * Mersenne Twister in the specified range inclusive
 */
int8_t zmtCharDie(
    ZMT *zmtPtr,
    int8_t lowInclusive,
    int8_t highInclusive
);

/*
 * Generates a random unsigned char from the given 
 * Mersenne Twister
 */
uint8_t zmtRandUChar(ZMT *zmtPtr);

/*
 * Generates a random unsigned char from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint8_t zmtUCharDie(
    ZMT *zmtPtr,
    uint8_t lowInclusive,
    uint8_t highInclusive
);

/*
 * Generates a random short from the given 
 * Mersenne Twister
 */
int16_t zmtRandShort(ZMT *zmtPtr);

/*
 * Generates a random short from the given 
 * Mersenne Twister in the specified range inclusive
 */
int16_t zmtShortDie(
    ZMT *zmtPtr,
    int16_t lowInclusive,
    int16_t highInclusive
);

/*
 * Generates a random unsigned short from the given 
 * Mersenne Twister
 */
uint16_t zmtRandUShort(ZMT *zmtPtr);

/*
 * Generates a random unsigned short from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint16_t zmtUShortDie(
    ZMT *zmtPtr,
    uint16_t lowInclusive,
    uint16_t highInclusive
);

/*
 * Generates a random int from the given 
 * Mersenne Twister
 */
int32_t zmtRandInt(ZMT *zmtPtr);

/*
 * Generates a random int from the given 
 * Mersenne Twister in the specified range inclusive
 */
int32_t zmtIntDie(
    ZMT *zmtPtr,
    int32_t lowInclusive,
    int32_t highInclusive
);

/*
 * Generates a random unsigned int from the given 
 * Mersenne Twister
 */
uint32_t zmtRandUInt(ZMT *zmtPtr);

/*
 * Generates a random unsigned int from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint32_t zmtUIntDie(
    ZMT *zmtPtr,
    uint32_t lowInclusive,
    uint32_t highInclusive
);

/*
 * Generates a random long from the given 
 * Mersenne Twister
 */
int64_t zmtRandLong(ZMT *zmtPtr);

/*
 * Generates a random long from the given 
 * Mersenne Twister in the specified range inclusive
 */
int64_t zmtLongDie(
    ZMT *zmtPtr,
    int64_t lowInclusive,
    int64_t highInclusive
);

/*
 * Generates a random unsigned long from the given 
 * Mersenne Twister
 */
uint64_t zmtRandULong(ZMT *zmtPtr);

/*
 * Generates a random unsigned long from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint64_t zmtULongDie(
    ZMT *zmtPtr,
    uint64_t lowInclusive,
    uint64_t highInclusive
);

/*
 * Generates a random float in the range [0.0, 1.0]
 * from the given Mersenne Twister
 */
float zmtRandFloat(ZMT *zmtPtr);

/*
 * Generates a random double in the range [0.0, 1.0]
 * from the given Mersenne Twister
 */
double zmtRandDouble(ZMT *zmtPtr);

/*
 * Generates a random bool from the given
 * Mersenne Twister
 */
bool zmtRandBool(ZMT *zmtPtr);

/*
 * Frees the memory associated with the given
 * Mersenne Twister
 */
void zmtFree(ZMT *zmtPtr);

#endif