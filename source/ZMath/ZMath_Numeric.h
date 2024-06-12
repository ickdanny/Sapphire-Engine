#ifndef ZMATH_NUMERIC_H
#define ZMATH_NUMERIC_H

#include <stdint.h>
#include <stddef.h>

/* 
 * The following min and max functions will evaluate
 * their operands once
 */

#define MIN_MAX_FUNC_DECL(TYPENAME, SUFFIX) \
TYPENAME min##SUFFIX(TYPENAME a, TYPENAME b); \
TYPENAME max##SUFFIX(TYPENAME a, TYPENAME b);

MIN_MAX_FUNC_DECL(char, Char)
MIN_MAX_FUNC_DECL(short, Short)
MIN_MAX_FUNC_DECL(int, Int)
MIN_MAX_FUNC_DECL(long, Long)

MIN_MAX_FUNC_DECL(size_t, SizeT)

MIN_MAX_FUNC_DECL(int8_t, Int8)
MIN_MAX_FUNC_DECL(int16_t, Int16)
MIN_MAX_FUNC_DECL(int32_t, Int32)
MIN_MAX_FUNC_DECL(int64_t, Int64)
MIN_MAX_FUNC_DECL(uint8_t, UInt8)
MIN_MAX_FUNC_DECL(uint16_t, UInt16)
MIN_MAX_FUNC_DECL(uint32_t, UInt32)
MIN_MAX_FUNC_DECL(uint64_t, UInt64)

MIN_MAX_FUNC_DECL(float, Float)
MIN_MAX_FUNC_DECL(double, Double)

#undef MIN_MAX_FUNC_DECL

/* Returns ceiling of the quotient of x/y */
int ceilingIntegerDivide(int x, int y);

#endif