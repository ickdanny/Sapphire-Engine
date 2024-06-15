#ifndef TRIFECTA_DIRECTORY_H
#define TRIFECTA_DIRECTORY_H

#ifdef __APPLE__

#include <dirent.h>

#endif /* __APPLE__ */

/* A type representing a directory */
typedef struct TFDirectory{
    /*
     * a dynamically allocated copy of the directory
     * name used to open this directory object
     */
    char *_dirName;

    #ifdef __APPLE__

    DIR *_dirPtr;
    
    #endif /* __APPLE__ */

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