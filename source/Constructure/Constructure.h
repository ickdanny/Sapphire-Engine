#ifndef CONSTRUCTURE_H
#define CONSTRUCTURE_H

#include "Constructure_Array.h"
#include "Constructure_ArrayList.h"
#include "Constructure_Bitset.h"
#include "Constructure_HashMap.h"
#include "Constructure_SparseSet.h"
#include "Constructure_String.h"

/*
 * The following functions are for use with the
 * Constructure HashMap type
 */

/* hash func for ints */
size_t intHash(const void *intPtr);

/* equals func for ints */
bool intEquals(
    const void *intPtr1, 
    const void *intPtr2
);

/* hash func for bitsets */
/* (implementation in Constructure_Bitset.c) */
size_t bitsetHash(const void *bitsetPtr);

/* equals func for bitsets */
/* (implementation in Constructure_Bitset.c) */
bool bitsetEquals(
    const void *bitsetPtr1,
    const void *bitsetPtr2
);

/* hash func for C strings */
size_t cStringHash(const void *stringPtr);

/* equals func for C strings */
bool cStringEquals(
    const void *stringPtr1,
    const void *stringPtr2
);

/* hash func for Constructure Strings */
size_t constructureStringHash(const void *stringPtr);

/* equals func for Constructure Strings */
bool constructureStringEquals(
    const void *stringPtr1,
    const void *stringPtr2
);

/* hash func for Constructure WideStrings */
size_t constructureWideStringHash(
    const void *wideStringPtr
);

/* equals func for Constructure WideStrings */
bool constructureWideStringEquals(
    const void *wideStringPtr1,
    const void *wideStringPtr2
);

/* hash func for Constructure String16s */
size_t constructureString16Hash(
    const void *string16Ptr
);

/* equals func for Constructure String16s */
bool constructureString16Equals(
    const void *string16Ptr1,
    const void *string16Ptr2
);

/* hash func for Constructure String32s */
size_t constructureString32Hash(
    const void *string32Ptr
);

/* equals func for Constructure String32s */
bool constructureString32Equals(
    const void *string32Ptr1,
    const void *string32Ptr2
);

#endif