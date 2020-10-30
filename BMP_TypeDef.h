#ifndef BMP_TYPEDEF_H
#define BMP_TYPEDEF_H

#include <stddef.h>

#define UINT8   unsigned char
#define UINT16  unsigned short
#define UINT32  unsigned int
#define INT8    char
#define INT16   short
#define INT32   int

typedef struct FILE_HEADER
{
    FILE_HEADER(): Identifier(19778), FileSize(1036854), Reserved(0), BitmapDataOffset(54) {}
    FILE_HEADER& operator=( const FILE_HEADER& );
    UINT16 Identifier;
    UINT32 FileSize;
    UINT32 Reserved;
    UINT32 BitmapDataOffset;
} FILE_HEADER;

typedef struct INFO_HEADER
{
    INFO_HEADER(): BitmapHeaderSize(40), Width(720), Height(480), Planes(1), BitPerPixel(24), Compression(0), BitmapDataSize(1036800),
                   HorizontalResolution(2835), VerticalResolution(2835), UsedColors(0), ImportanColors(0) {}
    INFO_HEADER& operator=( const INFO_HEADER& );
    UINT32 BitmapHeaderSize;
    UINT32 Width;
    UINT32 Height;
    UINT16 Planes;
    UINT16 BitPerPixel;
    UINT32 Compression;
    UINT32 BitmapDataSize;
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 UsedColors;
    UINT32 ImportanColors;
} INFO_HEADER;

typedef struct BMP_HEADER
{
    BMP_HEADER(): FileHeader(new FILE_HEADER), InfoHeader(new INFO_HEADER) {}
    BMP_HEADER& operator=( const BMP_HEADER& );
    FILE_HEADER* FileHeader;
    INFO_HEADER* InfoHeader;
} BMP_HEADER;

typedef union PIXEL
{
    PIXEL(): ByteValue(0) {}
    PIXEL& operator=( const PIXEL& );
    UINT32 ByteValue;
    struct
    {
        UINT32 B:8;
        UINT32 G:8;
        UINT32 R:8;
        UINT32 Res:8;
    };
} PIXEL;

typedef struct YUV_PIXEL
{
    YUV_PIXEL(): Y(0), U(0), V(0) {}
    YUV_PIXEL& operator=( const YUV_PIXEL& );
    UINT32 ByteValue;
    struct
    {
        UINT8 Y:8;
        UINT8 U:8;
        UINT8 V:8;
        UINT32 Res:8;
    };
} YUV_PIXEL;

typedef struct HSI_PIXEL
{
    HSI_PIXEL(): H(0), S(0), I(0) {}
    double H;
    double S;
    double I;
} HSI_PIXEL;

typedef struct HISTOGRAM
{
    HISTOGRAM(): HistogramBin(0), HistogramVector(NULL), MinValue(0), MaxValue(0) {}
    UINT32  HistogramBin;
    UINT32* HistogramVector;
    UINT32  MinValue;
    UINT32  MaxValue;
} HISTOGRAM;

#endif