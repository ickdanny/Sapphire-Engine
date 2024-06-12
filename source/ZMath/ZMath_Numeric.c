#include "ZMath_Numeric.h"

#ifndef min
/* A min macro: will evaluate expressions twice */
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
/* A max macro: will evaluate expressions twice */
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

/* 
 * The following min and max functions will evaluate
 * their operands once
 */

#define MIN_MAX_FUNC_DEFINE(TYPENAME, SUFFIX) \
TYPENAME min##SUFFIX(TYPENAME a, TYPENAME b){ \
    return min(a, b); \
} \
TYPENAME max##SUFFIX(TYPENAME a, TYPENAME b){ \
    return max(a, b); \
}

MIN_MAX_FUNC_DEFINE(char, Char)
MIN_MAX_FUNC_DEFINE(short, Short)
MIN_MAX_FUNC_DEFINE(int, Int)
MIN_MAX_FUNC_DEFINE(long, Long)

MIN_MAX_FUNC_DEFINE(size_t, SizeT)

MIN_MAX_FUNC_DEFINE(int8_t, Int8)
MIN_MAX_FUNC_DEFINE(int16_t, Int16)
MIN_MAX_FUNC_DEFINE(int32_t, Int32)
MIN_MAX_FUNC_DEFINE(int64_t, Int64)
MIN_MAX_FUNC_DEFINE(uint8_t, UInt8)
MIN_MAX_FUNC_DEFINE(uint16_t, UInt16)
MIN_MAX_FUNC_DEFINE(uint32_t, UInt32)
MIN_MAX_FUNC_DEFINE(uint64_t, UInt64)

MIN_MAX_FUNC_DEFINE(float, Float)
MIN_MAX_FUNC_DEFINE(double, Double)

#undef MIN_MAX_FUNC_DEFINE

/* Returns ceiling of the quotient of x/y */
int ceilingIntegerDivide(int x, int y){
    return (x / y) + (x % y != 0);
}