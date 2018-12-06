#ifndef _READPNG
#define _READPNG
#include <png.h>

#ifndef _UINT8
#define _UINT8

typedef unsigned char uint8;

#endif 

uint8 *read_png(FILE *fp,                       size_t *Nr,       size_t *Nc);
int   write_png(FILE *fp, const uint8 *p, const size_t  Nr, const size_t  Nc);

#endif
