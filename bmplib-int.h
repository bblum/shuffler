#ifndef BMPLIB_INT_H
#define BMPLIB_INT_H

#include "bmplib.h"

typedef struct {
  unsigned short padding;
  unsigned short bfType;
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
  unsigned int biSize;
  int biWidth;
  int biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int biCompression;
  unsigned int biSizeImage;
  unsigned int biXPelsPerMeter;
  unsigned int biYPelsPerMeter;
  unsigned int biClrUsed;
  unsigned int biClrImportant;
} BITMAPINFOHEADER;


#define DEFAULT_OFFSET 1078

static int writeBits(int, int, int, pixel *, unsigned int);
static int writeHeader(int, unsigned int, unsigned int, unsigned int);

#endif

