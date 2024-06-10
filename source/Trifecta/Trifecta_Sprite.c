#include "Trifecta_Sprite.h"

#include <stdio.h>

#include "PGUtil.h"
#include "ZMath.h"

/* Stores the information from the bmp file header */
typedef struct BitmapFileHeader{
    /* identifier "BM" */
	char id[2];
    /* file size (bmp files are little endian) */
    uint32_t fileSize;
    /* data offset (bmp files are little endian) */
    uint32_t dataOffset;
    /* 
     * size of secondary header which indicates variant
     * (bmp files are little endian)
     */
    uint32_t secondaryHeaderSize;
    /* pixel width (bmp files are little endian) */
	int32_t width;
    /* pixel height (bmp files are little endian) */
	int32_t height;
    /*
     * num color planes - must be 1
     * (bmp files are little endian)
     */
    uint16_t numColorPlanes;
    /*
     * bits used per pixel
     * (bmp files are little endian)
     */
    uint16_t bitsPerPixel;
    /*
     * identifies the compression method used
     * (bmp files are little endian)
     */
    uint32_t compressionCode;
    /*
     * size of pixel data in bytes
     * (bmp files are little endian)
     */
	uint32_t imageSize;
    /*
     * the horizontal resolution of the image in
     * pixels per metre (bmp files are little endian)
     */
    int32_t horizontalResolution;
    /*
     * the vertical resolution of the image in
     * pixels per metre (bmp files are little endian)
     */
    int32_t verticalResolution;
    /* 
     * number of colors in the palette
     * (bmp files are little endian)
     */
    uint32_t numColorsInPalette;
    /* 
     * number of important colors; generally ignored
     * (bmp files are little endian)
     */
    uint32_t numImportantColors;

    //todo: BI_BITFIELDS for transparency ??? compression
} BitmapFileHeader;

#define compressionCodeRGB 0
#define compressionCodeRLE8 1
#define compressionCodeRLE4 2
#define compressionCodeBitfields 3
#define compressionCodeJPEG 4
#define compressionCodePNG 5
#define compressionCodeAlphaBitfields 6
#define compressionCodeCMYK 11
#define compressionCodeCMYKRLE8 12
#define compressionCodeCMYKRLE4 13

/* Reads the header from the specified bmp file */
static BitmapFileHeader parseBitmapHeader(
    FILE *filePtr
){
    assertNotNull(filePtr, "null bmp filePtr");
    size_t bytesRead = 0;
    BitmapFileHeader toRet = {0};

    /* read ID */
    bytesRead = fread(&toRet.id[0], 1, 2, filePtr);
    assertTrue(
        bytesRead == 2,
        "failed to read bmp id"
    );
    assertTrue(
        toRet.id[0] == 'B' && toRet.id[1] == 'M',
        "bmp header should start with BM"
    );

    /* read file size */
    bytesRead = fread(&toRet.fileSize, 4, 1, filePtr);
    assertTrue(
        bytesRead == 4, //todo why only read 2 bytes?
        "failed to read bmp file size"
    );
    toRet.fileSize = fromLittleEndian32(
        toRet.fileSize
    );

    /* advance file ptr by 4 bytes to skip reserved */
    fseek(filePtr, 4, SEEK_CUR);

    /* read data offset */
    bytesRead = fread(&toRet.dataOffset, 4, 1, filePtr);
    assertTrue(
        bytesRead == 4,
        "failed to read bmp data offset"
    );
    toRet.dataOffset = fromLittleEndian32(
        toRet.dataOffset
    );

    /* read secondary header */
    bytesRead = fread(
        &toRet.secondaryHeaderSize,
        4,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 4,
        "failed to read bmp secondary header size"
    );
    toRet.secondaryHeaderSize = fromLittleEndian32(
        toRet.secondaryHeaderSize
    );
    assertTrue(
        toRet.secondaryHeaderSize == 40,
        "error: only accept Windows BITMAPINFOHEADER"
    );

    /* read width */
    bytesRead = fread(&toRet.width, 4, 1, filePtr);
    assertTrue(
        bytesRead == 4,
        "failed to read bmp width"
    );
    toRet.width = fromLittleEndian32(toRet.width);

    /* read height */
    bytesRead = fread(&toRet.height, 4, 1, filePtr);
    assertTrue(
        bytesRead == 4,
        "failed to read bmp height"
    );
    toRet.height = fromLittleEndian32(toRet.height);

    /* read num color planes */
    bytesRead = fread(
        &toRet.numColorPlanes,
        2,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 2,
        "failed to read bmp num color planes"
    );
    toRet.numColorPlanes = fromLittleEndian16(
        toRet.numColorPlanes
    );
    assertTrue(
        toRet.numColorPlanes == 1,
        "num color planes must be 1"
    );

    /* read bits per pixel */
    bytesRead = fread(
        &toRet.bitsPerPixel,
        2,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 2,
        "failed to read bmp bits per pixel"
    );
    toRet.bitsPerPixel = fromLittleEndian16(
        toRet.bitsPerPixel
    );

    /* read compression code */
    bytesRead = fread(
        &toRet.compressionCode,
        4,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 4,
        "failed to read bmp compression code"
    );
    toRet.compressionCode = fromLittleEndian32(
        toRet.compressionCode
    );

    /* read image size */
    bytesRead = fread(
        &toRet.imageSize,
        4,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 4,
        "failed to read bmp image size"
    );
    toRet.imageSize = fromLittleEndian32(
        toRet.imageSize
    );
    /* default value */
    if(toRet.imageSize == 0){
        toRet.imageSize
            = toRet.width * toRet.height * 3;
    }

    /* read horizontal resolution */
    bytesRead = fread(
        &toRet.horizontalResolution,
        4,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 4,
        "failed to read bmp horizontal resolution"
    );
    toRet.horizontalResolution = fromLittleEndian32(
        toRet.horizontalResolution
    );

    /* read vertical resolution */
    bytesRead = fread(
        &toRet.verticalResolution,
        4,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 4,
        "failed to read bmp vertical resolution"
    );
    toRet.verticalResolution = fromLittleEndian32(
        toRet.verticalResolution
    );

    /* read num colors in palette */
    bytesRead = fread(
        &toRet.numColorsInPalette,
        4,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 4,
        "failed to read bmp num colors in palette"
    );
    toRet.numColorsInPalette = fromLittleEndian32(
        toRet.numColorsInPalette
    );

    /* read num importnat colors */
    bytesRead = fread(
        &toRet.numImportantColors,
        4,
        1,
        filePtr
    );
    assertTrue(
        bytesRead == 4,
        "failed to read bmp num important colors"
    );
    toRet.numImportantColors = fromLittleEndian32(
        toRet.numImportantColors
    );

    return toRet;
}

/* Loads a sprite from the specified .bmp file */
TFSprite parseBitmapFile(const char *fileName){
    /* try to open file */
    FILE *filePtr = fopen(fileName, "rb");
    assertNotNull(filePtr, fileName);

    BitmapFileHeader header = parseBitmapHeader(
        filePtr
    );

    switch(header.compressionCode){
        case compressionCodeRGB:
            pgWarning("rgb");
            break;
        case compressionCodeRLE8:
            pgWarning("rle8");
            break;
        case compressionCodeRLE4:
            pgWarning("rle4");
            break;
        case compressionCodeBitfields:
            pgWarning("bitfields");
            break;
        case compressionCodeJPEG:
            pgWarning("jpeg");
            break;
        case compressionCodePNG:
            pgWarning("png");
            break;
        case compressionCodeAlphaBitfields:
            pgWarning("alpha bitfields");
            break;
        case compressionCodeCMYK:
            pgWarning("cmyk");
            break;
        case compressionCodeCMYKRLE8:
            pgWarning("cmykrle8");
            break;
        case compressionCodeCMYKRLE4:
            pgWarning("cmykrle4");
            break;
    }

    /* ptr to actual data */
    unsigned char *dataPtr;

    TFSprite toRet = {0};



    //todo: load sprite
    return toRet;
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