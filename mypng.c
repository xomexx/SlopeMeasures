#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include "mypng.h"
/*
 * Based on example code by Guillaume Cottenceau: Copyright 2002-2010.
 *
 */


/* ********************************************************************* */
uint8 *read_png(FILE *fp, size_t *Nr, size_t *Nc)
/* Reads in an 8-bit greyscale  PNG image  and returns a flat uint8 array.
   code by: Paul Jackway. 07 July 2018.
   uses: libpng (link with -lpng) 
*/
/* ********************************************************************* */
{
  int         width, height;
  png_byte    color_type;
  png_byte    bit_depth;
  png_structp png_ptr;
  png_infop   info_ptr;
  png_bytep  *row_pointers;
  uint8      *p;
  size_t      index;
  size_t      r, c;
  unsigned char header[8];    // 8 is the maximum size that can be checked

  if (8 != fread(header, 1, 8, fp)) {
    fprintf(stderr, "read_png(): 8 byte header cannot be read from file.\n");
    return NULL;
  }
  if (png_sig_cmp(header, 0, 8)) {
    fprintf(stderr, "read_png(): File header is not recognized as a PNG file.\n");
    return NULL;
  }
  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fprintf(stderr, "read_png(): png_create_read_struct failed.\n");
    return NULL;
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fprintf(stderr, "read_png(): png_create_info_struct failed.\n");
    return NULL;
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "read_png(): Error during init_io.\n");
    return NULL;
  }
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  
  png_read_info(png_ptr, info_ptr);
  
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  /* sanity tests for 8-bit grey image */

  if (color_type != PNG_COLOR_TYPE_GRAY) {
    fprintf(stderr, "read_png(): Sorry only greyscale images are supported for now!\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return NULL;
  }
  if (bit_depth != 8) {
    fprintf(stderr, "read_png(): Sorry only 8-bit images are supported for now!\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    return NULL;
  }     
  png_read_update_info(png_ptr, info_ptr);
  
  /* read file */
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "read_png(): Error during read_image.\n");
    return NULL;
  }
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (r=0; r<height; r++)
    row_pointers[r] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  png_read_image(png_ptr, row_pointers);

  png_read_end(png_ptr, NULL);
 
  /* now copy out pixels into my flat uint8 array */
  p  = (uint8  *)calloc(height * width, sizeof(uint8));
  if (!p) {
    fprintf(stderr, "read_png(): Error during calloc array for image.\n");
    return NULL;
  }
  index = 0L;
  for (r = 0; r < height; r++) {
    for (c = 0; c < width; c++) {
      p[index++] = (uint8)(0xFF & row_pointers[r][c]);
    }
  }
  /* clean up png stuff */
  /* cleanup heap allocation */
  for (r=0; r<height; r++)
    free(row_pointers[r]);
  free(row_pointers);
  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
  
  (*Nr) = (size_t)height;
  (*Nc) = (size_t)width;
  return p;
}



/* ********************************************************************* */
int write_png(FILE *fp, const uint8 *p, const size_t Nr, const size_t Nc) {
/* Writes out a 8-bit greyscale  PNG image from a flat uint8 array.
   code by: Paul Jackway. 07 July 2018.
   uses: libpng (link with -lpng)
*/
/* ********************************************************************* */
  size_t      index;
  size_t      r, c;
  int         width, height;
  png_byte    color_type;
  png_byte    bit_depth;
  png_structp png_ptr;
  png_infop   info_ptr;
  png_bytep  *row_pointers;

  width  = (int)Nc;
  height = (int)Nr;
  bit_depth = 8; /* 8-bit per pixel only supported */
  color_type = PNG_COLOR_TYPE_GRAY; /* greyscale only supported */
  
  /* initialize stuff */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fprintf(stderr, "write_png(): png_create_write_struct failed.\n");
    return -1;
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fprintf(stderr, "write_png(): png_create_info_struct failed.\n");
    return -1;
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "write_png(): Error during init_io.\n");
    return -1;
  }
  png_init_io(png_ptr, fp);

  /* write header */
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "write_png(): Error during writing header.\n");
    return -1;
  }
  png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
	       PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png_ptr, info_ptr);
  /* setup row pointers */
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (r=0; r<height; r++)
    row_pointers[r] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
  /* fill row pointers with pixels from flat array */
  index = 0L;
  for (r = 0; r < height; r++) {
    for (c = 0; c < width; c++) {
      row_pointers[r][c] = p[index++];
    }
  }
  
  /* write bytes */
  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "write_png(): Error during writing bytes.\n");
    return -1;
  }
  png_write_image(png_ptr, row_pointers);
  
  /* end write */
  if (setjmp(png_jmpbuf(png_ptr)))
    fprintf(stderr, "write_png(): Error during end of write.\n");
  
  png_write_end(png_ptr, NULL);

  /* cleanup heap allocation */
  for (r=0; r<height; r++)
    free(row_pointers[r]);
  free(row_pointers);

  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

  return 0;
}
