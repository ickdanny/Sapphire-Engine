#include "ZMath_MT.h"

#include "PGUtil.h"

/* implementation from Wikipedia */

#define n 624
#define m 397
#define w 32
#define r 31
#define UMASK (0xffffffffUL << r)
#define LMASK (0xffffffffUL >> (w-r))
#define a 0x9908b0dfUL
#define u 11
#define s 7
#define t 15
#define l 18
#define b 0x9d2c5680UL
#define c 0xefc60000UL
#define f 1812433253UL

/*
 * Initializes and returns a new Mersenne Twister
 * by value, seeded with the given integer
 */
ZMT zmtMake(uint32_t seed){
    ZMT toRet = {0};

    /* allocate state array */
    toRet.stateArray = pgAlloc(
        n,
        sizeof(*(toRet.stateArray))
    );

    /* init state array */
    toRet.stateArray[0] = seed;
    for(int i = 1; i < n; ++i){
        seed = f * (seed ^ (seed >> (w-2))) + i;
        toRet.stateArray[i] = seed; 
    }
    
    toRet.stateIndex = 0;

    return toRet;
}

/* 
 * Generates a random uint32_t from the given
 * Mersenne Twister
 */
static uint32_t randUInt32(ZMT *zmtPtr){
    /* k points to current state index */
    int k = zmtPtr->stateIndex;
    
    /*
     * j points to state n-1 iterations ago with mod n
     * circular indexing
     */
    int j = k - (n - 1);
    if(j < 0){
        j += n;
    }

    uint32_t x = (zmtPtr->stateArray[k] & UMASK)
        | (zmtPtr->stateArray[j] & LMASK);
    
    uint32_t xA = x >> 1;
    if(x & 0x00000001UL){
        xA ^= a;
    }
    
    /*
     * j points to state n-m iterations ago with mod n
     * circular indexing
     */
    j = k - (n - m);
    if(j < 0){
        j += n;
    }
    
    /* compute next value in state */
    x = zmtPtr->stateArray[j] ^ xA;
    zmtPtr->stateArray[k++] = x;
    
    /* 
     * advance state index with mod n circular
     * indexing (0 <= stateIndex <= n-1)
     */
    if(k >= n){
        k = 0;
    }
    zmtPtr->stateIndex = k;
    
    /* tempering */
    uint32_t y = x ^ (x >> u); 
    y = y ^ ((y << s) & b);
    y = y ^ ((y << t) & c);
    uint32_t z = y ^ (y >> l);
    
    return z; 
}

/*
 * Generates a random char from the given 
 * Mersenne Twister
 */
int8_t zmtRandChar(ZMT *zmtPtr){
    return randUInt32(zmtPtr);
}

/*
 * Generates a random char from the given
 * Mersenne Twister in the specified range inclusive
 */
int8_t zmtCharDie(
    ZMT *zmtPtr,
    int8_t lowInclusive,
    int8_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound char die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandChar(zmtPtr);
    }
    /* use larger type to account for overflows */
    int32_t range
        = ((int32_t)highInclusive) - lowInclusive;
    ++range;
    int8_t randOffset = randUInt32(zmtPtr) % range;
    int8_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure char die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random unsigned char from the given 
 * Mersenne Twister
 */
uint8_t zmtRandUChar(ZMT *zmtPtr){
    return randUInt32(zmtPtr);
}

/*
 * Generates a random unsigned char from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint8_t zmtUCharDie(
    ZMT *zmtPtr,
    uint8_t lowInclusive,
    uint8_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound uchar die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandUChar(zmtPtr);
    }
    /* use larger type to account for overflows */
    int32_t range
        = ((int32_t)highInclusive) - lowInclusive;
    ++range;
    uint8_t randOffset = randUInt32(zmtPtr) % range;
    uint8_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure uchar die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random short from the given 
 * Mersenne Twister
 */
int16_t zmtRandShort(ZMT *zmtPtr){
    return randUInt32(zmtPtr);
}

/*
 * Generates a random short from the given 
 * Mersenne Twister in the specified range inclusive
 */
int16_t zmtShortDie(
    ZMT *zmtPtr,
    int16_t lowInclusive,
    int16_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound short die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandShort(zmtPtr);
    }
    /* use larger type to account for overflows */
    int32_t range
        = ((int32_t)highInclusive) - lowInclusive;
    ++range;
    int16_t randOffset = randUInt32(zmtPtr) % range;
    int16_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure short die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random unsigned short from the given 
 * Mersenne Twister
 */
uint16_t zmtRandUShort(ZMT *zmtPtr){
    return randUInt32(zmtPtr);
}

/*
 * Generates a random unsigned short from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint16_t zmtUShortDie(
    ZMT *zmtPtr,
    uint16_t lowInclusive,
    uint16_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound ushort die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandUShort(zmtPtr);
    }
    /* use larger type to account for overflows */
    int32_t range
        = ((int32_t)highInclusive) - lowInclusive;
    ++range;
    uint16_t randOffset = randUInt32(zmtPtr) % range;
    uint16_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure ushort die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random int from the given 
 * Mersenne Twister
 */
int32_t zmtRandInt(ZMT *zmtPtr){
    return randUInt32(zmtPtr);
}

/*
 * Generates a random int from the given 
 * Mersenne Twister in the specified range inclusive
 */
int32_t zmtIntDie(
    ZMT *zmtPtr,
    int32_t lowInclusive,
    int32_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound int die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandInt(zmtPtr);
    }
    /* use larger type to account for overflows */
    int64_t range
        = ((int64_t)highInclusive) - lowInclusive;
    ++range;
    int32_t randOffset = zmtRandULong(zmtPtr) % range;
    int32_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure int die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random unsigned int from the given 
 * Mersenne Twister
 */
uint32_t zmtRandUInt(ZMT *zmtPtr){
    return randUInt32(zmtPtr);
}

/*
 * Generates a random unsigned int from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint32_t zmtUIntDie(
    ZMT *zmtPtr,
    uint32_t lowInclusive,
    uint32_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound uint die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandUInt(zmtPtr);
    }
    /* use larger type to account for overflows */
    int64_t range
        = ((int64_t)highInclusive) - lowInclusive;
    ++range;
    uint32_t randOffset = zmtRandULong(zmtPtr) % range;
    uint32_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure uint die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random long from the given 
 * Mersenne Twister
 */
int64_t zmtRandLong(ZMT *zmtPtr){
    int64_t upperHalf = randUInt32(zmtPtr);
    upperHalf <<= 32;
    int64_t lowerHalf = randUInt32(zmtPtr);
    return upperHalf | lowerHalf;
}

/*
 * Generates a random long from the given 
 * Mersenne Twister in the specified range inclusive;
 * undefined behavior if highInclusive - lowInclusive
 * exceeds the representable values of int64_t, but
 * will work on most machines
 */
int64_t zmtLongDie(
    ZMT *zmtPtr,
    int64_t lowInclusive,
    int64_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound long die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandLong(zmtPtr);
    }
    /* range is always representable as ulong */
    uint64_t range = highInclusive - lowInclusive;
    /* add one to range for modulo to work correctly */
    ++range;
    uint64_t randOffset = zmtRandULong(zmtPtr) % range;
    /*
     * adding will work even if range is not
     * representable as a long
     */
    int64_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure long die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random unsigned long from the given 
 * Mersenne Twister
 */
uint64_t zmtRandULong(ZMT *zmtPtr){
    uint64_t upperHalf = randUInt32(zmtPtr);
    upperHalf <<= 32;
    uint64_t lowerHalf = randUInt32(zmtPtr);
    return upperHalf | lowerHalf;
}

/*
 * Generates a random unsigned long from the given 
 * Mersenne Twister in the specified range inclusive
 */
uint64_t zmtULongDie(
    ZMT *zmtPtr,
    uint64_t lowInclusive,
    uint64_t highInclusive
){
    /* check if range is invalid */
    if(lowInclusive > highInclusive){
        pgError("low bound > high bound ulong die!");
    }
    /* check if range is 0 */
    if(lowInclusive == highInclusive){
        return lowInclusive;
    }
    /* check if range is the entire type */
    if(highInclusive == lowInclusive - 1){
        return zmtRandULong(zmtPtr);
    }
    /* range calculation cannot overflow */
    uint64_t range = highInclusive - lowInclusive;
    /* add one to range for modulo to work correctly */
    ++range;
    uint64_t randOffset = zmtRandULong(zmtPtr) % range;
    uint64_t toRet = lowInclusive + randOffset;
    #ifdef _DEBUG
    assertFalse(
        toRet < lowInclusive || toRet > highInclusive,
        "limit failure ulong die"
    );
    #endif
    return toRet;
}

/*
 * Generates a random float in the range [0.0, 1.0]
 * from the given Mersenne Twister
 */
float zmtRandFloat(ZMT *zmtPtr){
    static const uint64_t uLongMax = ~((uint64_t)0);
    uint64_t randULong = zmtRandULong(zmtPtr);
    float toRet = (float)randULong;
    /* 
     * divide by the max value to get number between
     * 0.0 and 1.0
     */
    toRet /= ((float)uLongMax);
    return toRet;
}

/*
 * Generates a random double in the range [0.0, 1.0]
 * from the given Mersenne Twister
 */
double zmtRandDouble(ZMT *zmtPtr){
    static const uint64_t uLongMax = ~((uint64_t)0);
    uint64_t randULong = zmtRandULong(zmtPtr);
    double toRet = (double)randULong;
    /* 
     * divide by the max value to get number between
     * 0.0 and 1.0
     */
    toRet /= ((double)uLongMax);
    return toRet;
}

/*
 * Generates a random bool from the given
 * Mersenne Twister
 */
bool zmtRandBool(ZMT *zmtPtr){
    if(randUInt32(zmtPtr) % 2 == 0){
        return true;
    }
    return false;
}

/*
 * Frees the memory associated with the given
 * Mersenne Twister
 */
void zmtFree(ZMT *zmtPtr){
    pgFree(zmtPtr->stateArray);
}