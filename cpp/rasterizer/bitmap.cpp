#include "bitmap.h"

#include <cstdio>
#include <cstring>
#include <cmath>

namespace BMP {

enum Compression {
  RGB = 0,
  RLE8,
  RLE4,
  BITFIELDS,
  JPEG,
  PNG,
};

#pragma pack(push, 1)

//2 bytes
struct FileMagic {
  uint8 num0, num1;
};

//12 bytes
struct FileHeader {
  uint32 fileSize;
  uint16 creators[2];
  uint32 dataOffset;
};

//40 bytes, all windows versions since 3.0
struct DibHeader {
  uint32 headerSize;
  int32 width, height;
  uint16 numPlanes, bitsPerPixel;
  uint32 compression;
  uint32 dataSize;
  int32 hPixelsPer, vPixelsPer;  //horizontal and vertical pixels-per-meter
  uint32 numPalColors, numImportantColors;
};

#pragma pack(pop)

Image imageFromBitmapData(std::vector<uint8> rawData, std::string_view source) {
  Image image;
  if (!rawData.size())
    return image;

  size_t streamPos = 0;

  const FileMagic *magic = (const FileMagic*) &rawData[streamPos];
  if (magic->num0 != 'B' && magic->num1 != 'M') {
    printf("%s - error: '%s' is not a valid bitmap image", __FUNCTION__, source.data());
    return image;
  }
  streamPos += sizeof(FileMagic);

  const FileHeader *header = (const FileHeader*) &rawData[streamPos];
  streamPos += sizeof(FileHeader);

  const DibHeader *dib = (const DibHeader*) &rawData[streamPos];
  if (dib->compression != Compression::RGB || dib->bitsPerPixel != 24) {
    printf("%s - error: '%s' is not a 24 bit bitmap image", __FUNCTION__, source.data());
    return image;
  }

  image.w = dib->width;
  image.h = dib->height;
  image.pixels = std::vector<Color>(image.w * image.h);

  int bypp = dib->bitsPerPixel / 8;
  int rem = 0;
  if ((image.w * bypp) & 0x03) {
    rem = 4 - ((image.w * bypp) & 0x03);
  }

  streamPos = header->dataOffset;
  for (int y = 0; y < image.h; y++) {
    for (int x = 0; x < image.w; x++) {
      image.pixels[y * image.w + x].b = rawData[streamPos++];
      image.pixels[y * image.w + x].g = rawData[streamPos++];
      image.pixels[y * image.w + x].r = rawData[streamPos++];
    }
    for (int i = 0; i < rem; i++)
      streamPos++;
  }
  image.source = std::string(source);
  printf("%s - image dimensions: %d x %d\n", __FUNCTION__, image.w, image.h);

  return image;
}

bool writeImageToBitmapFile(const Image &image, std::string_view name) {
  char file[256];

  if (!image.w || !image.h || !image.pixels.size()) {
    return false;
  }

  strcpy(file, name.data());
  int ext = 0;
  for (const char *p = name.data(); *p != '\0'; p++) {
    if ('.' == p[0] && 'b' == p[1] && 'm' == p[2] && 'p' == p[3] && '\0' == p[4]) {
      ext++;
      break;
    }
  }
  if (!ext)
    strcat(file, ".bmp");

  FILE *fp = fopen(file, "wb");

  FileMagic magic;
  magic.num0 = 'B';
  magic.num1 = 'M';
  fwrite(&magic, 2, 1, fp);  //hard coding 2 bytes, (our structure isn't packed).

  FileHeader fileHeader;
  fileHeader.fileSize = image.w * image.h * sizeof(int) + 54;
  fileHeader.creators[0] = fileHeader.creators[1] = 0;
  fileHeader.dataOffset = 54;
  fwrite(&fileHeader, sizeof(fileHeader), 1, fp);

  DibHeader dibHeader;
  dibHeader.headerSize = 40;
  dibHeader.width = image.w;
  dibHeader.height = image.h;
  dibHeader.numPlanes = 1;
  dibHeader.bitsPerPixel = 24;
  dibHeader.compression = Compression::RGB;
  dibHeader.dataSize = 0;
  dibHeader.hPixelsPer = dibHeader.vPixelsPer = 1000;
  dibHeader.numPalColors = dibHeader.numImportantColors = 0;
  fwrite(&dibHeader, sizeof(DibHeader), 1, fp);

  int rem = 0;
  if ((image.w * 3) & 0x03)
    rem = 4 - ((image.w * 3) & 0x03);

  for (int y = 0; y < image.h; y++) {
    for (int x = 0; x < image.w; x++) {
      Color pixel = image.pixels[y * image.w + x];
      fputc(pixel.b, fp);
      fputc(pixel.g, fp);
      fputc(pixel.r, fp);
    }
    for (int i = 0; i < rem; i++)
      fputc(0xff, fp);
  }

  fclose(fp);
  return true;
}

Color Image::sample(float x, float y, bool clamp) const {

  float xStart = floorf(x);
  float xStop = ceilf(x);

  float yStart = floorf(y);
  float yStop = ceilf(y);

  float u = x - xStart;
  float v = y - yStart;

  Color colors[2][2];
  colors[0][0] = get(xStart, yStart, clamp);
  colors[0][1] = get(xStop, yStart, clamp);
  colors[1][0] = get(xStart, yStop, clamp);
  colors[1][1] = get(xStop, yStop, clamp);

  auto color = colors[0][0].lerp(colors[0][1], u);
  auto color2 = colors[1][0].lerp(colors[1][1], u);

  return color.lerp(color2, v);
}

}
