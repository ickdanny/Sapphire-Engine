#ifndef TRIFECTA_DIRECTORY_H
#define TRIFECTA_DIRECTORY_H

#ifdef __unix__

#include <dirent.h>

#endif /* __unix__ */

#ifdef WIN32

#include "Trifecta_Win32.h"
#include <fileapi.h>

#endif

/* A type representing a directory */
typedef struct TFDirectory{
    /*
     * a dynamically allocated copy of the directory
     * name used to open this directory object
     */
    char *_dirName;

    #ifdef __unix__

    DIR *_dirPtr;
    
    #endif /* __unix__ */

    #ifdef WIN32

    HANDLE _searchHandle;

    #endif /* WIN32 */

} TFDirectory;

/*
 * Opens the specified directory and returns a
 * new TFDirectory by value
 */
TFDirectory tfDirectoryOpen(const char *dirName);

/* 
 * Prints the name of the next file in the specified
 * directory to the given C string; prints only the
 * null terminator if the directory has no more
 * files
 */
void tfDirectoryGetNextFileName(
    TFDirectory *dirPtr,
    char *fileNameOut,
    int arraySize
);

/*
 * Frees the memory associated with the specified
 * TFDirectory
 */
void tfDirectoryFree(TFDirectory *dirPtr);

#endif