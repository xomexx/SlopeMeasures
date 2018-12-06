#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h> /* multiprocessor support */
//#include "../PaulSwissPNG/png/mypng.h"
#include "mypng.h"


/***********************************************************************/
double* SteepestTangent(const uint8 *Image, const size_t  Nr, const size_t  Nc) {
  /* 
     This implements the basic Steepest Tangent Algorithm.
     Loops over all other pixels and remembers the greatest absolute slope.
     Paul @ 09 July 2018.
     Allocates memory for a double precision image, fills it with the
     Steepest Tangent and returns a pointer to it.
  */
/***********************************************************************/

  double *SteepestTangentImage = (double *)calloc(Nr*Nc, sizeof(double));
  if (!SteepestTangentImage) {
    fprintf(stderr, "SteepestTangent(): cannot allocate memory for return image.\n");
    return NULL;
  }
#pragma omp parallel for collapse(2)
  for (long r=0L; r<Nr; r++) {
    for (long c=0L; c<Nc; c++) {
      const long   index       = r*Nc + c;  
      const double centerPixel = (double)Image[index]; 
      double MaxSlopeSq = 0.0D;
      for (long r2=0L; r2<Nr; r2++) {
	for (long c2=0L; c2<Nc; c2++) {
	  const double pixelDiff = (double)Image[r2*Nc + c2] - centerPixel;
	  if (pixelDiff != 0.0D) {
	    const double slopeSq = (pixelDiff * pixelDiff) / (double)((c2-c)*(c2-c) + (r2-r)*(r2-r));
	    if (slopeSq > MaxSlopeSq) MaxSlopeSq = slopeSq;
	  } 
	} /* c2 */
      } /* r2 */
      SteepestTangentImage[index] = sqrt(MaxSlopeSq);
    } /* c */
  } /* r */
  
  return SteepestTangentImage;
} /* SteepestTangent() */
