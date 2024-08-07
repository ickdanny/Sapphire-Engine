#include "Trifecta_Directory.h"

#include "PGUtil.h"

#ifdef __APPLE__

#include <unistd.h>
#include <errno.h>

/* debug function to print the current directory */
void printCurrentDir(){
    #define bufferSize 50
    static char buffer[bufferSize] = {0};
    if(getcwd(buffer, bufferSize) != NULL){
        pgWarning(buffer);
    }
    else{
        pgWarning("failed to get current dir");
        switch(errno){
            case EACCES:
                pgWarning("permission denied");
                break;
            case EFAULT:
                pgWarning("bad buffer ptr");
                break;
            case EINVAL:
                pgWarning("invalid args");
                break;
            case ENAMETOOLONG:
                pgWarning("path too long");
                break;
            case ENOENT:
                pgWarning("cwd unlinked");
                break;
            case ENOMEM:
                pgWarning("insufficient memory");
                break;
            case ERANGE:
                pgWarning("buffer too small");
                break;
            default:
                pgWarning("unknown error");
                break;
        }
    }
    #undef bufferSize
}

/*
 * Opens the specified directory and returns a
 * new TFDirectory by value
 */
TFDirectory tfDirectoryOpen(const char *dirName){
    TFDirectory toRet = {0};

    /* try to open the directory */
    toRet._dirPtr = opendir(dirName);
    if(!toRet._dirPtr){
        pgWarning(dirName);
        switch(errno){
            case EACCES:
                pgWarning("permission denied");
                break;
            case EBADF:
                pgWarning("bad fd");
                break;
            case EMFILE:
                pgWarning("process fd limit reached");
                break;
            case ENFILE:
                pgWarning("system fd limit reached");
                break;
            case ENOENT:
                pgWarning("dir does not exist");
                break;
            case ENOMEM:
                pgWarning("insufficient memory");
                break;
            case ENOTDIR:
                pgWarning("not a directory");
                break;
            default:
                pgWarning("unknown error");
                break;
        }
        pgWarning("Current directory: ");
        printCurrentDir();
        pgError("failed to opendir");
    }

    /* copy directory name */
    int nameLength = strlen(dirName);
    int allocLength = nameLength + 1;
    toRet._dirName = pgAlloc(
        allocLength,
        sizeof(char)
    );
    strncpy(toRet._dirName, dirName, allocLength);

    return toRet;
}

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
){
    assertNotNull(
        fileNameOut,
        "null out param passed to dir next file name"
    );
    if(arraySize <= 0){
        return;
    }
    if(!dirPtr){
        *fileNameOut = '\0';
        return;
    }
    struct dirent *nextDirentPtr = readdir(
        dirPtr->_dirPtr
    );
    /* if null is returned, we are out so print \0 */
    if(!nextDirentPtr){
        *fileNameOut = '\0';
        return;
    }
    /* otherwise, print filename to out param */
    else{
        snprintf(
            fileNameOut,
            arraySize,
            "%s/%s",
            dirPtr->_dirName, /* prepend dir name */
            nextDirentPtr->d_name /* then file name */
        );
    }
}

/*
 * Frees the memory associated with the specified
 * TFDirectory
 */
void tfDirectoryFree(TFDirectory *dirPtr){
    if(dirPtr){
        /* 
         * since closedir can be interrupted by a
         * signal, repeat until successful
         */
        int retCode = 0;
        do{
            retCode = closedir(dirPtr->_dirPtr);
        } while(retCode != 0 && errno == EINTR);
        pgFree(dirPtr->_dirName);
    }
}

#endif /* __APPLE__ */