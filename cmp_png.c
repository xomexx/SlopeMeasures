#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include "mypng.h"


int CompareImages(const uint8 *Image1, const size_t  Nr1, const size_t  Nc1,
		  const uint8 *Image2, const size_t  Nr2, const size_t  Nc2) {
  size_t r, c;
  size_t r_fd, c_fd; /* location of the first differences */
  size_t r_md, c_md; /* location of the maximum differences */
  int    diff, size_fd=0, size_md=0;  /* size of the first and maximum differences */
  uint8  p1, p2;

  printf("Are the images the same size?            ");
  if (Nr1 == Nr2 && Nc1 == Nc2) {
    printf("[YES]:   %ldx%ld\n", Nr1, Nc1);
  } else {
    printf("[ NO]:   %ldx%ld vs %ldx%ld\n", Nr1, Nc1, Nr2, Nc2);
    return EXIT_SUCCESS;
  }

  
  for (r=0; r <Nr1; r++) {
    for (c=0; c <Nc1; c++) {
      p1 = Image1[r*Nc1+c];
      p2 = Image2[r*Nc2+c];
      diff = (int)p1 - (int)p2;
      if (diff) {
	if (size_fd==0) {
	  size_fd = diff;
	  r_fd = r; c_fd = c;
	}
	if (abs(diff) > abs(size_md)) {
	  size_md = diff;
	  r_md = r; c_md = c;	  
	}
      }
    }
  }
  printf("Are the images exactly the same?         ");
  if (size_fd == 0) {
    printf("[YES]\n");
  } else {
    printf("[ NO]\n");
    printf("The first difference is:    %d at (%ld,%ld)\n", size_fd, r_fd, c_fd);
    printf("The greatest difference is: %d at (%ld,%ld)\n", size_md, r_md, c_md);
  }

  return EXIT_SUCCESS;
} /* CompareIImages() */



int main(int argc, char *argv[]) {
  /* 
     Opens 2 .png files supplied on the command line and compares them as 
     many ways as possible.  Prints out the results on stdout.
     NB: uint8 is a 8-bit byte. Typecast from "unsigned char" in mypng.h
     Paul Jackway @ 13 October 2018.
   */
  
  FILE *fp1, *fp2;
  uint8 *Image1, *Image2;
  size_t Nr1, Nc1, Nr2, Nc2;

  if (argc != 3) {
    printf("Usage: cmp_png file1.png file2.png\n");
    printf("Compares the two .png files and reports on stdout.\n");
    return EXIT_SUCCESS;
  }
  fp1 = fopen(argv[1], "rb");
  if (!fp1) {
    printf("Could not open image1: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  fp2 = fopen(argv[2], "rb");
  if (!fp2) {
    printf("Could not open image2: %s\n", argv[2]);
    return EXIT_FAILURE;
  }
  Image1 = read_png(fp1, &Nr1, &Nc1);
  fclose(fp1);
  if (Image1==NULL) {
    printf("Could not read image1: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  Image2 = read_png(fp2, &Nr2, &Nc2);
  fclose(fp2);
  if (Image2==NULL) {
    printf("Could not read image2: %s\n", argv[2]);
    free(Image1);
    return EXIT_FAILURE;
  }
  
  CompareImages(Image1, Nr1, Nc1, Image2, Nr2, Nc2);

  free(Image1);
  free(Image2);
  return EXIT_SUCCESS;
}
