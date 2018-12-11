#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h> /* multiprocessor support */
//#include "../PaulSwissPNG/png/mypng.h"
#include "mypng.h"

extern double* SteepestTangent(const uint8 *Image, const size_t  Nr, const size_t  Nc);
uint8* DoubleToUINT8Image(const double *DImage, const size_t  Nr, const size_t  Nc);
double get_time_ms();

/***********************************************************************/
int main(int argc, char *argv[]) {
  /* 
     NB: uint8 is a 8-bit byte. Typecast from "unsigned char" in mypng.h
     Paul @ 07 JUly 2018.
  */
/***********************************************************************/
  
  FILE   *fp, *fpo; /* input and output file pointers */
  uint8  *Image;    /* input image to be read from file */
  uint8  *OutImage; /* output image to be written to file */
  double *DImage;   /* Double precision image holds the result of steepest tangennt */
  size_t  Nr, Nc;   /* all images are this size, rows and cols */
  double  start_time, end_time; /* hold high precision times */
 
  if (argc != 3) {
    printf("Usage: %s in.png out.png\n", argv[0]);
    printf("Hint: try something like: %s lena.png test_lena.png\n", argv[0]);
    return 0;
  }
  
  fp = fopen(argv[1], "rb");
  if (!fp) {
    fprintf(stderr, "Cannot open the input image: %s.\n", argv[1]);
    return -1;
  }
  fpo = fopen(argv[2], "wb");
  if (!fpo) {
    fprintf(stderr, "Cannot open the output image: %s.\n", argv[2]);
    return -1;
  }
  
  Image = read_png(fp, &Nr, &Nc);
  fclose(fp);

  start_time = get_time_ms();
  DImage = SteepestTangent(Image, Nr, Nc); // Which algorithm is run depends on which swisxxxx.c file is in the Makefile. 
  end_time = get_time_ms();
  printf("Time taken: %.3lf seconds\n", (end_time-start_time)/1000.0);
  
  free(Image);  
  OutImage =  DoubleToUINT8Image(DImage, Nr, Nc);

#ifdef DEBUG
  /* print the first 32 values of DImage */
  size_t nPrint = Nc*Nr < 32 ? Nc*Nr : 32;
  for (size_t i=0; i<nPrint; i++) printf("%f ", DImage[i]);
  printf("\n");
#endif

  write_png(fpo, OutImage, Nr, Nc);
  fclose(fpo);

  free(DImage);  
  free(OutImage);

  return 0;
}



/***********************************************************************/
uint8* DoubleToUINT8Image(const double *DImage, const size_t  Nr, const size_t  Nc) {
  /* 
     Converts a double image to a uint8 image after normalizing. Use this for image
     storage and display.
     Paul @ 09 July 2018.
  */
/***********************************************************************/
  size_t index;
  uint8 *OutImage;
  double Z, MaxVal, MinVal, Multiplier;

  const size_t N = Nr*Nc; /* total number of pixels */
  OutImage = (uint8  *)calloc(N, sizeof(*OutImage));
  if (!OutImage) {
    fprintf(stderr, "DoubleToUINT8Image(): cannot allocate memory for return image.\n");
    return NULL;
  }
  /* find image maximum and minimum values */
  MaxVal = MinVal =  DImage[0L];  
  for (index=1L; index<N; index++) {
    Z = DImage[index];
    if (Z > MaxVal)  MaxVal = Z;
    if (Z < MinVal)  MinVal = Z;
  }
  /* Normalize to [0-255.999] then truncate to uint8 = [0-255] */
  if (MaxVal == MinVal) {
    Multiplier = 1.0D; /* cant stretch a constant image! */
  } else {
    Multiplier = 255.999D / ( MaxVal - MinVal);
  }
  for (index=0L; index<N; index++) {
    Z = Multiplier * (DImage[index] - MinVal);
    OutImage[index] = (uint8)floor(Z);
  }

  return OutImage;
} /* DoubleToUINT8Image() */


/**************************************************************/
double get_time_ms() {
/**************************************************************/
  /*  
      example code from: timmurphy.org 
  */
  
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + (t.tv_usec/1000000.0)) * 1000.0;
} /* get_time_ms() */


