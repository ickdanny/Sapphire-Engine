#include "Trifecta_Sprite.h"

#include <stdio.h>

#include "PGUtil.h"
#include "ZMath.h"

/* Stores the information from the bmp file header */
typedef struct BitmapFileHeader{
    /* identifier "BM" */
	uint16_t id;
    /* file size (bmp files are little endian) */
    uint32_t fileSize;
    /* data offset (bmp files are little endian) */
    uint32_t dataOffset;
    /* pixel width (bmp files are little endian) */
	uint32_t width;
    /* pixel height (bmp files are little endian) */
	uint32_t height;
    /*
     * size of pixel data in bytes
     * (bmp files are little endian)
     */
	uint32_t imageSize;

    //todo: BI_BITFIELDS for transparency ??? compression
} BitmapFileHeader;

/* Loads a sprite from the specified .bmp file */
TFSprite parseBitmapFile(const char *fileName){
    /* try to open file */
    FILE *filePtr = fopen(fileName, "rb");
    assertNotNull(filePtr, fileName);

    /* try to read 54 byte header*/
    char header[54] = {0};
    size_t bytesRead = fread(header, 1, 54, filePtr);
    assertTrue(
        bytesRead == 54,
        "bitmap header not 54 bytes"
    );

    /* check that first 2 chars are "BM" */
    assertTrue(
        header[0] == 'B' && header[1] == 'M',
        "bitmap header should start with BM"
    );

    /* get the file size */
    uint32 fileSize = 0;

    /* get the file offset where data array begins */
    uint32_t dataOffset = 0;

    /* the width of the image in pixels */
    unsigned int width = 0;
    /* the height of the image in pixels */
    unsigned int height = 0;
    /* the size of the image in bytes */
    unsigned int imageSize = 0;   // = width*height*3
    /* ptr to actual data */
    unsigned char *dataPtr;
}

/* 
 * Constructs and returns a TFSpriteInstruction
 * by value
 */
TFSpriteInstruction tfSpriteInstructionMake(
    TFSprite *spritePtr,
    int depth,
    Vector2D offset,
    float rotation,
    float scale
){
    TFSpriteInstruction toRet = {0};
    toRet.spritePtr = spritePtr;
    toRet.depth = depth;
    toRet.offset = offset;
    toRet.rotation = rotation;
    toRet.scale = scale;
    return toRet;
}