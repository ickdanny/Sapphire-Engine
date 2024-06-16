#include "Constructure.h"

/* hash func for ints */
size_t intHash(const void *intPtr){
    return *((int*)intPtr);
}

/* equals func for ints */
bool intEquals(
    const void *intPtr1, 
    const void *intPtr2
){
    return *((int*)intPtr1) == *((int*)intPtr2);
}

#define NULL_TERMINATED_STRING_HASH_ALGORITHM( \
    CHAR_PTR_NAME \
) \
    do{ \
        static size_t prime = 53; \
        static size_t modulo = 1294967281; \
        size_t hash = 0; \
        while(*CHAR_PTR_NAME != 0){ \
            hash += *CHAR_PTR_NAME; \
            ++hash; \
            hash *= prime; \
            hash %= modulo; \
            ++charPtr; \
        } \
        return hash; \
    } while(false)

/* hash func for C strings */
size_t cStringHash(const void *stringPtr){
    const char *charPtr = *((char**)stringPtr);
    NULL_TERMINATED_STRING_HASH_ALGORITHM(charPtr);
}

/* equals func for C strings */
bool cStringEquals(
    const void *stringPtr1,
    const void *stringPtr2
){
    char *string1 = *((char**)stringPtr1);
    char *string2 = *((char**)stringPtr2);
    return strcmp(string1, string2) == 0;
}

/* hash func for Constructure Strings */
size_t constructureStringHash(const void *stringPtr){
    const char *charPtr = ((String*)stringPtr)->_ptr;
    NULL_TERMINATED_STRING_HASH_ALGORITHM(charPtr);
}

/* equals func for Constructure Strings */
bool constructureStringEquals(
    const void *stringPtr1,
    const void *stringPtr2
){
    return stringEquals(stringPtr1, stringPtr2);
}

/* hash func for Constructure WideStrings */
size_t constructureWideStringHash(
    const void *wideStringPtr
){
    const wchar_t *charPtr
        = ((WideString*)wideStringPtr)->_ptr;
    NULL_TERMINATED_STRING_HASH_ALGORITHM(charPtr);
}

/* equals func for Constructure WideStrings */
bool constructureWideStringEquals(
    const void *wideStringPtr1,
    const void *wideStringPtr2
){
    return wideStringEquals(
        wideStringPtr1,
        wideStringPtr2
    );
}

/* hash func for Constructure String16s */
size_t constructureString16Hash(
    const void *string16Ptr
){
    const char16_t *charPtr
        = ((String16*)string16Ptr)->_ptr;
    NULL_TERMINATED_STRING_HASH_ALGORITHM(charPtr);
}

/* equals func for Constructure String16s */
bool constructureString16Equals(
    const void *string16Ptr1,
    const void *string16Ptr2
){
    return string16Equals(string16Ptr1, string16Ptr2);
}

/* hash func for Constructure String32s */
size_t constructureString32Hash(
    const void *string32Ptr
){
    const char32_t *charPtr
        = ((String32*)string32Ptr)->_ptr;
    NULL_TERMINATED_STRING_HASH_ALGORITHM(charPtr);
}

/* equals func for Constructure String32s */
bool constructureString32Equals(
    const void *string32Ptr1,
    const void *string32Ptr2
){
    return string32Equals(string32Ptr1, string32Ptr2);
}