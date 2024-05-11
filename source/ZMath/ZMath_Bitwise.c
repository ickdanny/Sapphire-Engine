#include "ZMath_Bitwise.h"

/* Swaps endianness of a short */
uint16_t byteSwap16(uint16_t i){
    return (i << 8) | (i >> 8);
}

/* Swaps endianness of an int */
uint32_t byteSwap32(uint32_t i){
    return (i << 24)
        | ((i & 0x0000FF00) << 8)
        | ((i & 0x00FF0000) >> 8)
        | (i >> 24);
}

/* Swaps endianness of a long */
uint64_t byteSwap64(uint64_t i){
    return (i << 56)
        | ((i & 0x000000000000FF00) << 40)
        | ((i & 0x0000000000FF0000) << 24)
        | ((i & 0x00000000FF000000) << 8)
        | ((i & 0x000000FF00000000) >> 8)
        | ((i & 0x0000FF0000000000) >> 24)
        | ((i & 0x00FF000000000000) >> 40)
        | (i >> 56);
}

/* Converts a little endian short for the CPU */
uint16_t fromLittleEndian16(uint16_t i){
    if(isMachineLittleEndian){
        return i;
    }
    else{
        return byteSwap16(i);
    }
}

/* Converts little endian int for the CPU */
uint32_t fromLittleEndian32(uint32_t i){
    if(isMachineLittleEndian){
        return i;
    }
    else{
        return byteSwap32(i);
    }
}

/* Converts little endian long for the CPU */
uint64_t fromLittleEndian64(uint64_t i){
    if(isMachineLittleEndian){
        return i;
    }
    else{
        return byteSwap64(i);
    }
}

/* Converts a big endian short for the CPU */
uint16_t fromBigEndian16(uint16_t i){
    if(isMachineBigEndian){
        return i;
    }
    else{
        return byteSwap16(i);
    }
}

/* Converts big endian int for the CPU */
uint32_t fromBigEndian32(uint32_t i){
    if(isMachineBigEndian){
        return i;
    }
    else{
        return byteSwap32(i);
    }
}

/* Converts big endian long for the CPU */
uint64_t fromBigEndian64(uint64_t i){
    if(isMachineBigEndian){
        return i;
    }
    else{
        return byteSwap64(i);
    }
}