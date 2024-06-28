#ifndef CONSTRUCTURE_STRING_H
#define CONSTRUCTURE_STRING_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "PGUtil.h"
#include "Trifecta_Uchar.h"

/* 
 * For index searching operations, failure
 * to find will be signaled by a return of
 * this constant equivalent to SIZE_MAX
 */
#define indexNotFound SIZE_MAX

/* Utility macro for +1 for readability */
#define lengthIncludingNull(length) \
    (length + ((unsigned char)1u))

/*
 * Unlike the generic containers in the rest
 * of Constructure, the String types will
 * only accept integral character types and
 * will have a different container type for
 * each - implemented via this type-declare 
 * macro:
 */
#define CONSTRUCTURE_STRING_DECL( \
    TYPENAME, \
    PREFIX, \
    CHARTYPE \
) \
\
/* \
 * A wrapper for a continguous null terminated \
 * fixed width character array on the heap \
 */ \
typedef struct TYPENAME{ \
    /* null terminated */ \
    CHARTYPE *_ptr; \
\
    /* does not include null terminator */ \
    size_t length; \
\
    size_t _capacity; \
} TYPENAME; \
\
/* \
 * Counts the number of characters in a null \
 * terminated C string of the character type, \
 * not including the null terminator itself \
 */ \
size_t _##PREFIX##CLength( \
    const CHARTYPE *cStringPtr \
); \
\
/* \
 * Creates a string copy of the given null \
 * terminated C string of the character type \
 * and returns it by value \
 */ \
TYPENAME PREFIX##MakeC( \
    const CHARTYPE *cStringPtr \
); \
\
/* \
 * Creates a string copy of the given null \
 * terminated C string of the character type \
 * up to the given maximum number of characters \
 * copied (not including the null terminator) and \
 * returns it by value \
 */ \
TYPENAME PREFIX##MakeCLength( \
    const CHARTYPE *cStringPtr, \
    size_t maxCharsCopied \
); \
\
/* \
 * Creates a string copy of the given null \
 * terminated C string of type "char" and returns \
 * it by value \
 */ \
TYPENAME PREFIX##MakeCharC( \
    const char *cStringPtr \
); \
\
/* \
 * Creates a string copy of the given null \
 * terminated C string of type "char" \
 * up to the given maximum number of characters \
 * copied (not including the null terminator) and \
 * returns it by value \
 */ \
TYPENAME PREFIX##MakeCharCLength( \
    const char *cStringPtr, \
    size_t maxCharsCopied \
); \
\
/* \
 * Creates a new empty string and reserves enough \
 * space for the requested number of characters \
 * including the null terminator \
 */ \
TYPENAME PREFIX##MakeAndReserve( \
    size_t initCapacity \
); \
\
/* \
 * Creates and allocates space for an \
 * empty string \
 */ \
TYPENAME PREFIX##MakeEmpty(); \
\
/* \
 * Makes a copy of the given string and returns \
 * it by value \
 */ \
TYPENAME PREFIX##Copy( \
    const TYPENAME *toCopyPtr \
); \
\
/* \
 * Copies the contents of the source string \
 * into the destination string \
 */ \
void PREFIX##CopyInto( \
    TYPENAME *destPtr, \
    const TYPENAME *srcPtr \
); \
\
/* \
 * Returns true if the given string is empty, \
 * false otherwise \
 */ \
bool PREFIX##IsEmpty( \
    const TYPENAME *stringPtr \
); \
\
/* \
 * Clears the given string but does not deallocate \
 * its memory \
 */ \
void PREFIX##Clear( \
    TYPENAME *stringPtr \
); \
\
/* \
 * Reallocates more space for the given string  \
 * if its capacity does not fit its nominal  \
 * length; returns false as error code, true \
 * otherwise \
 */ \
bool _##PREFIX##GrowToFitNewLength( \
    TYPENAME *stringPtr \
); \
\
/* \
 * Reallocates more space for the given string \
 * if it is currently at or above capacity;  \
 * returns false as error code, true otherwise \
 */ \
bool _##PREFIX##GrowIfNeeded( \
    TYPENAME *stringPtr \
); \
\
/*  \
 * Prepends the given null terminated C string \
 * of the specified length onto the front of  \
 * the given string; do not attempt to pass in \
 * the same string twice \
 */ \
void _##PREFIX##PrependHelper( \
    TYPENAME *stringPtr, \
    const CHARTYPE *toPrependPtr, \
    size_t toPrependLength \
); \
\
/*  \
 * Prepends the given null terminated C string \
 * onto the front of the given string; do not \
 * attempt to pass in the same string twice \
 */ \
void PREFIX##PrependC( \
    TYPENAME *stringPtr, \
    const CHARTYPE *toPrependPtr \
); \
\
/*  \
 * Prepends the second given string onto the \
 * front of the first; do not attempt to pass \
 * in the same string twice \
 */ \
void PREFIX##Prepend( \
    TYPENAME *stringPtr, \
    TYPENAME *toPrependPtr \
); \
\
/* \
 * Appends the given null terminated C string \
 * of the specified length onto the back of the  \
 * given string; do not attempt to pass in the \
 * same string twice \
 */ \
void _##PREFIX##AppendHelper( \
    TYPENAME *stringPtr, \
    const CHARTYPE *toAppendPtr, \
    size_t toAppendLength \
); \
\
/* \
 * Appends the given null terminated C string \
 * onto the back of the given string; do not \
 * attempt to pass in the same string twice \
 */ \
void PREFIX##AppendC( \
    TYPENAME *stringPtr, \
    const CHARTYPE *toAppendPtr \
); \
\
/* \
 * Appends the second given string onto the back \
 * of the first; do not attempt to pass in the \
 * same string twice \
 */ \
void PREFIX##Append( \
    TYPENAME *stringPtr, \
    TYPENAME *toAppendPtr \
); \
 \
/* \
 * Writes a null terminator at the back of \
 * the given string after its length \
 */ \
void _##PREFIX##WriteNull( \
    TYPENAME *stringPtr \
); \
\
/* \
 * Pushes the given char onto the back of the \
 * given string \
 */ \
void PREFIX##PushBack( \
    TYPENAME *stringPtr, \
    CHARTYPE toPush \
); \
\
/* \
 * Removes the last character of the given \
 * string; error if empty \
 */ \
void PREFIX##PopBack( \
    TYPENAME *stringPtr \
); \
\
/* \
 * Erases the character of the given string \
 * at the given index \
 */ \
void PREFIX##EraseChar( \
    TYPENAME *stringPtr, \
    size_t index \
); \
\
/* \
 * Returns the character of the given string \
 * at the given index \
 */ \
CHARTYPE PREFIX##CharAt( \
    const TYPENAME *stringPtr, \
    size_t index \
); \
\
/* \
 * Returns the front character of the given \
 * string \
 */ \
CHARTYPE PREFIX##Front( \
    const TYPENAME *stringPtr \
); \
\
/* \
 * Returns the back character of the given  \
 * string \
 */ \
CHARTYPE PREFIX##Back( \
    const TYPENAME *stringPtr \
); \
\
/* \
 * Sets the character of the given string \
 * at the given index to the given value, \
 * overwriting its previous value \
 */ \
void PREFIX##SetChar( \
    TYPENAME *stringPtr, \
    size_t index, \
    CHARTYPE newValue \
); \
\
/* \
 * Inserts the given character into the given \
 * string at the given index by making room \
 * for the new character \
 */ \
void PREFIX##InsertChar( \
    TYPENAME *stringPtr, \
    size_t index, \
    CHARTYPE toInsert \
); \
\
/* \
 * Inserts the given null terminated C string \
 * of the specified length into the given \
 * string starting at the given index by making \
 * room for the new characters; do not attempt \
 * to pass the same string as both the base and \
 * the insertion \
 */ \
void _##PREFIX##InsertHelper( \
    TYPENAME *stringPtr, \
    size_t startingIndex, \
    const CHARTYPE *toInsertPtr, \
    size_t toInsertLength \
); \
\
/* \
 * Inserts the given null terminated C string \
 * into the given string starting at the given \
 * index by making room for the new characters; \
 * do not attempt to pass the same string as \
 * both the base and the insertion \
 */ \
void PREFIX##InsertC( \
    TYPENAME *stringPtr, \
    size_t startingIndex, \
    const CHARTYPE *toInsertPtr \
); \
\
/* \
 * Inserts the latter given string into the former \
 * starting at the given index by making room for  \
 * the new characters; do not attempt to pass the  \
 * same string as both the base and the insertion \
 */ \
void PREFIX##Insert( \
    TYPENAME *stringPtr, \
    size_t startingIndex, \
    const TYPENAME *toInsertPtr \
); \
\
/* \
 * Returns the index of the first occurrence \
 * of the given character in the given string, \
 * or returns indexNotFound if no such \
 * character exists \
 */ \
size_t PREFIX##IndexOfChar( \
    const TYPENAME *stringPtr, \
    CHARTYPE toFind \
); \
\
/* \
 * Returns the index of the last occurrence \
 * of the given character in the given string, \
 * or returns indexNotFound if no such character \
 * exists \
 */ \
size_t PREFIX##LastIndexOfChar( \
    const TYPENAME *stringPtr, \
    CHARTYPE toFind \
); \
\
/* \
 * Returns the starting index of the first \
 * occurrence of the given C string of the \
 * specified length in the given string, or  \
 * returns indexNotFound if no such substring  \
 * exists \
 */ \
size_t _##PREFIX##IndexOfHelper( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *targetPtr, \
    size_t subLength \
); \
\
/* \
 * Returns the starting index of the first \
 * occurrence of the given null terminated \
 * C string in the given string, or returns \
 * indexNotFound if no such substring exists \
 */ \
size_t PREFIX##IndexOfC( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *targetPtr \
); \
\
/* \
 * Returns the starting index of the first \
 * occurrence of the given target string \
 * in the given base string, or returns \
 * indexNotFound if no such substring exists \
 */ \
size_t PREFIX##IndexOf( \
    const TYPENAME *stringPtr, \
    const TYPENAME *targetPtr \
); \
\
/* \
 * Returns the starting index of the last \
 * occurrence of the given C string of the \
 * specified length in the given string, or \
 * returns indexNotFound if no such substring \
 * exists \
 */ \
size_t _##PREFIX##LastIndexOfHelper( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *targetPtr, \
    size_t subLength \
); \
\
/* \
 * Returns the starting index of the last \
 * occurrence of the given null terminated \
 * C string in the given string, or returns \
 * indexNotFound if no such substring exists \
 */ \
size_t PREFIX##LastIndexOfC( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *targetPtr \
); \
\
/* \
 * Returns the starting index of the last \
 * occurrence of the given target string \
 * in the given base string, or returns \
 * indexNotFound if no such substring exists \
 */ \
size_t PREFIX##LastIndexOf( \
    const TYPENAME *stringPtr, \
    const TYPENAME *targetPtr \
); \
\
/* \
 * Returns true if the given string begins with \
 * the given character, false otherwise \
 */ \
bool PREFIX##BeginsWithChar( \
    const TYPENAME *stringPtr, \
    CHARTYPE toTest \
); \
\
/* \
 * Returns true if the given string ends with \
 * the given character, false otherwise \
 */ \
bool PREFIX##EndsWithChar( \
    const TYPENAME *stringPtr, \
    CHARTYPE toTest \
); \
\
/* \
 * Returns true if the given string begins with \
 * the given null terminated C string of the \
 * specified length, false otherwise \
 */ \
bool _##PREFIX##BeginsWithHelper( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *toTestPtr, \
    size_t toTestLength \
); \
\
/* \
 * Returns true if the given string begins with \
 * the given null terminated C string, false \
 * otherwise \
 */ \
bool PREFIX##BeginsWithC( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *toTestPtr \
); \
\
/* \
 * Returns true if the former given string begins \
 * with the latter, false otherwise \
 */ \
bool PREFIX##BeginsWith( \
    const TYPENAME *stringPtr, \
    const TYPENAME *toTestPtr \
); \
\
/* \
 * Returns true if the given string ends with \
 * the given null terminated C string of the \
 * specified length, false otherwise \
 */ \
bool _##PREFIX##EndsWithHelper( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *toTestPtr, \
    size_t toTestLength \
); \
\
/* \
 * Returns true if the given string ends with \
 * the given null terminated C string, false \
 * otherwise \
 */ \
bool PREFIX##EndsWithC( \
    const TYPENAME *stringPtr, \
    const CHARTYPE *toTestPtr \
); \
\
/* \
 * Returns true if the former given string ends \
 * with the latter, false otherwise \
 */ \
bool PREFIX##EndsWith( \
    const TYPENAME *stringPtr, \
    const TYPENAME *toTestPtr \
); \
\
/* \
 * Returns a newly created string by value \
 * which points to a copy of a substring \
 * of the given string; the substring is \
 * specified by inclusive start and end \
 * indices \
 */ \
TYPENAME PREFIX##Substring( \
    const TYPENAME *stringPtr, \
    size_t startIndexInclusive, \
    size_t endIndexInclusive \
); \
\
/* \
 * Compares the two given null terminated C \
 * strings of the character type and returns  \
 * >0 if the first is greater than the second,  \
 * <0 if the second is greater than the first, \
 * or 0 if the two are equal. \
 */ \
int _##PREFIX##CCompare( \
    const CHARTYPE *cStringPtr1, \
    const CHARTYPE *cStringPtr2 \
); \
\
/* \
 * Compares the two given strings and returns  \
 * >0 if the first is greater than the second,  \
 * <0 if the second is greater than the first, \
 * or 0 if the two are equal. \
 */ \
int PREFIX##Compare( \
    const TYPENAME *stringPtr1, \
    const TYPENAME *stringPtr2 \
); \
\
/* \
 * Returns true if the two given strings are \
 * equal, false otherwise \
 */ \
bool PREFIX##Equals( \
    const TYPENAME *stringPtr1, \
    const TYPENAME *stringPtr2 \
); \
\
/* Frees the given string */ \
void PREFIX##Free( \
    TYPENAME *strPtr \
);

/* end of macro */

CONSTRUCTURE_STRING_DECL(
    String,
    string,
    char
)
CONSTRUCTURE_STRING_DECL(
    WideString,
    wideString,
    wchar_t
)
CONSTRUCTURE_STRING_DECL(
    String16,
    string16,
    char16_t
)
CONSTRUCTURE_STRING_DECL(
    String32,
    string32,
    char32_t
)

#endif