#pragma once

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


#define BI_RGB        0
#define BI_RLE8       1
#define BI_RLE4       2
#define BI_BITFIELDS  3
#define BI_JPEG       4
#define BI_PNG        5


#pragma pack(push, 1)

typedef struct pixel_s {
  union {
    struct {
      uint8 r, g, b;
    };
    uint8 rgb[3];
  };
} pixel_t;

typedef struct
{
  uint8 num0, num1;
}bmp_file_magic_t;      //2 bytes

typedef struct
{
  uint32 filesize;
  uint16 creators[2];
  uint32 dataoffset;
}bmp_file_header_t;     //12 bytes

typedef struct
{
  uint32 headersize;
  int32  width, height;
  uint16 numplanes, bitsperpixel;
  uint32 compression;
  uint32 datasize;
  int32  hpixelsper, vpixelsper;  //horizontal and vertical pixels-per-meter
  uint32 numpalcolors, numimpcolors;

}bmp_dib_header_t;   //40 bytes, all windows versions since 3.0


typedef struct
{
  unsigned int headersize;
  int width, height;
  unsigned short numplanes, bitsperpixel;
  unsigned int compression;
  unsigned int datasize;
  int hpixelsper, vpixelsper;
  unsigned int numpalcolors, numimpcolors;
  unsigned int redmask, greenmask, bluemask;

}bmp_dib_header_v2_t;   //52 bytes, 40 + RGB double word masks (added by adobe)

typedef struct
{
  unsigned int headersize;
  int width, height;
  unsigned short numplanes, bitsperpixel;
  unsigned int compression;
  unsigned int datasize;
  int hpixelsper, vpixelsper;
  unsigned int numpalcolors, numimpcolors;
  unsigned int redmask, greenmask, bluemask, alphamask;

}bmp_dib_header_v3_t;   //56 bytes, 40 + RGBA double word masks (added by adobe)

#pragma pack(pop)

void writeBMP( const pixel_t *pixels, int32 w, int32 h, const char name[] );
