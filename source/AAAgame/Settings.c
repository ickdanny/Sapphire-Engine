#include "Settings.h"

#include <stdio.h>
#include <stdbool.h>

#include "PGUtil.h"

/* 
 * Reads settings from file if present, otherwise
 * creates default settings and a new file
 */
Settings readOrCreateSettingsFromFile(
    const char *fileName
){
    Settings toRet = {0};

    /* try to open file as binary read */
    FILE *filePtr = fopen(fileName, "rb");

    /* if file exists and we successfully opened */
    if(filePtr != NULL){
        /* attempt to read data */
        size_t itemsRead = fread(
            &toRet,
            sizeof(toRet),
            1,
            filePtr
        );
        /* if failed to read, write defaults to file */
        if(itemsRead != 1){
            fclose(filePtr);
            filePtr = NULL;
            toRet.muted = false;
            toRet.fullscreen = false;
            writeSettingsToFile(&toRet, fileName);
        }
    }

    /* if file doesn't exit */
    else{
        writeSettingsToFile(&toRet, fileName);
    }

    return toRet;
}

/*
 * Writes the specified settings to the specified file
 */
void writeSettingsToFile(
    const Settings *settingsPtr,
    const char *fileName
){
    /* try to open file as binary write */
    FILE *filePtr = fopen(fileName, "wb");

    /* if successfully opened file */
    if(filePtr){
        /* attempt to write data */
        fwrite(
            settingsPtr,
            sizeof(*settingsPtr),
            1,
            filePtr
        );

        fclose(filePtr);
    }
}