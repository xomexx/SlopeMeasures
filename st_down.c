#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include "mypng.h"
		
/**
 * A transform finding the gradient of the steepest upwards tangent at each pixel. 
 * <p>
 * CODER WARNING: Many of the internal methods use the square of the slope. 
 */

double* inGrey ; 
	
int fullWidth ; 
int fullHeight ; 
double** mins ; 
double** maxs ; 
int* widths ; 
int* heights ; 
int* quadLengths ;
int layerCount ; 

double getPixel(double* pixels, int x, int y, int width) {
  return pixels[x+y*width] ;
}

void setPixel(double* pixels, int x, int y, double value, int width) {
  pixels[x+y*width] = value ;
}

double searchWholeQuad(int inX, int inY, double inValue, int quadX, int quadY, int depth, double steepestYet);

/**
 * Returns the least possible distance (along one axis) from a pixel in the finest-grained image to any pixel in the given quad. 
 * @param inCoord Coordinate of source pixel in finest-grained image. 
 * @param otherLayerCoord Coordinate of other 'pixel' in the layer-image. 
 */
int minCoordDistance(int inCoord, int otherLayerCoord, int quadLength) { 
  /* Note: we can never be to the right of a truncated-width quad. */ 
  int otherFinestLowCoord = otherLayerCoord * quadLength ; 
  int otherFinestHighCoord = otherFinestLowCoord + quadLength - 1 ; // Inside the quad, not immediately-after. 
  if (inCoord<otherFinestLowCoord) return otherFinestLowCoord - inCoord ; 
  if (inCoord>otherFinestHighCoord) return inCoord - otherFinestHighCoord ; 
  return 0 ; 
}
	
double computeSteepestPossibleSlopeToOther(int inX, int inY, double inValue, int quadX, int quadY, int depth, int quadLength) {
  int layerWidth = widths[depth] ; 
  int minDistanceX = minCoordDistance(inX, quadX, quadLength); 
  int minDistanceY = minCoordDistance(inY, quadY, quadLength); 
  int minDistanceSqr = minDistanceX*minDistanceX+minDistanceY*minDistanceY;
  double minQuadValue =  getPixel(mins[depth], quadX, quadY, layerWidth); 
  double valueDiff = inValue-minQuadValue;
  double steepestPossibleSlopeSqr = valueDiff*fabs(valueDiff) / minDistanceSqr ;
  return steepestPossibleSlopeSqr ;
}
	
/**
 * Returns the delta to the sibling quad along this axis. 
 * <p>
 * Each quad has three siblings in the one-coarser quad. 
 * This method returns <code>-1</code> or <code>1</code>, according to whether the neighbour along this axis is to the right or left of this <code>coord</code>. 
 * If there is no neighbour (ie, we're in a truncated quad), <code>zero</code> is returned.  
 */
int getDeltaCoord(int coord, int layerLength) { 
  int/*boolean*/ isOdd = (coord%2) == 1 ; 
  if (isOdd) { 
    /* Here we know: we are in the right half of the quad. */
    return -1 ; 
  } else {
    /* Here we know: we are in the left half of the quad. */
    if (coord==layerLength-1) { 
      /* Here we know: this is the last coordinate. There is no sibling along this axis. */
      return 0 ; 
    } else { 
      return 1 ; 
    }
  }
}
	
/**
 * Returns the coordinate the quad in the next coarser layer, that holds this coordinate in this layer. 
 */
int toCoarserCoord(int coord) { 
  return coord / 2 ; 
}

int imin(int i, int j) {
  if (i<j) return i ; else return j ;
}

int toCoarserLength(int length) {
  return (length+1)/2;
}

int getQuadDepth_n(int length) { 
  if (length==1) return 1 ; else return getQuadDepth_n(toCoarserLength(length)) + 1 ; 
}

int getQuadDepth(int fullWidth, int fullHeight) { 
  return getQuadDepth_n(imin(fullWidth, fullHeight)); 
}

double* makeDoubles(int width, int height) {
  return (double*)calloc(width*height, sizeof(double)) ;
}
  
/**
 * Fills the quad image, and related fields, for the given depth. 
 * <p>
 * This is an initialization method. 
 */
void fillQuadDepth(int targetDepth) { 
  int sourceDepth = targetDepth - 1 ; 
  double* sourceMins = mins[sourceDepth] ; 
  double* sourceMaxs = maxs[sourceDepth] ; 
  int sourceWidth = widths[sourceDepth] ; 
  int sourceHeight = heights[sourceDepth] ; 
  int sourceLength = quadLengths[sourceDepth] ; 
  int targetWidth = toCoarserLength(sourceWidth) ; 
  int targetHeight = toCoarserLength(sourceHeight) ; 
  int targetLength = 2 * sourceLength ; 
  double* targetMins = makeDoubles(targetWidth, targetHeight); 
  double* targetMaxs = makeDoubles(targetWidth, targetHeight); 
  mins[targetDepth] = targetMins ; 
  maxs[targetDepth] = targetMaxs ; 
  widths[targetDepth] = targetWidth ; 
  heights[targetDepth] = targetHeight ; 
  quadLengths[targetDepth] = targetLength ; 
  /************* OMP ***************/
#pragma omp parallel for  collapse(2)
  for (int targetY=0 ; targetY<targetHeight ; targetY++) { 
    for (int targetX=0 ; targetX<targetWidth ; targetX++) {  /* PJ moved this up 2 lines - for better OMP! */
      int sourceYStart = targetY * 2 ; 
      int sourceYFinish = fmin(sourceYStart+sourceLength+1, sourceHeight); 
      // TODO We're extracting min & max from 2x2, 2x1 or 1x1 squares here. It could be more efficient! 
      int sourceXStart = targetX * 2 ; 
      int sourceXFinish = fmin(sourceXStart+sourceLength+1, sourceWidth); 
      double quadMin = getPixel(sourceMins, sourceXStart, sourceYStart, sourceWidth); // ->pixels[sourceYStart][sourceXStart] ; 
      double quadMax = getPixel(sourceMaxs, sourceXStart, sourceYStart, sourceWidth); // ->pixels[sourceYStart][sourceXStart] ; 
      for (int sourceY=sourceYStart ; sourceY<sourceYFinish ; sourceY++) { 
	for (int sourceX=sourceXStart ; sourceX<sourceXFinish ; sourceX++) { 
	  quadMin = fmin(quadMin, getPixel(sourceMins, sourceX, sourceY, sourceWidth)) ; 
	  quadMax = fmax(quadMax, getPixel(sourceMaxs, sourceX, sourceY, sourceWidth)) ; 
	}
      }
      setPixel(targetMins, targetX, targetY, quadMin, targetWidth); 
      setPixel(targetMaxs, targetX, targetY, quadMax, targetWidth); 
    }
  }
}

/**
 * Searches a sibling quad for the steepest slope. 
 * <p>
 * <em>Assumes</em>: The pixel <code>inX,inY</code> is in the finest-grain image (ie, <code>inGrey</code>), 
 * and the quad <code>baseQuadX,baseQuadY</code> at layer <code>depth</code> contains that pixel. 
 * 
 * @param deltaX X-delta to a sibling quad. 
 * @param deltaY Y-delta to a sibling quad. 
 * @return The steepest slope found within the sibling quad, or <code>steepestYet</code>, which ever is steeper. 
 */
double searchSibling(int inX, int inY, double inValue, int baseQuadX, int baseQuadY, int depth, int quadLength, int deltaX, int deltaY, double steepestYet) { 
  int quadX = baseQuadX + deltaX ; 
  int quadY = baseQuadY + deltaY ; 
  double steepestPossibleSlope = computeSteepestPossibleSlopeToOther(inX, inY, inValue, quadX, quadY, depth, quadLength); 
  if (steepestPossibleSlope>steepestYet) { 
    steepestYet = searchWholeQuad(inX, inY, inValue, quadX, quadY, depth, steepestYet); 
  }
  return steepestYet ;
}
	
/**
 * Recursively searches for the (square of) steepest slope from pixel <code>inX,inY</code> to any other pixel. 
 * The arguments specify a quad that has already been searched. 
 * This method will search the three sibling quads at this level, then search the coarser levels. 
 * 
 * @param inX x-coordinate of source pixel in the finest-grained image (ie, the <code>inGrey</code> image). 
 * @param inY y-coordinate of source pixel in the finest-grained image (ie, the <code>inGrey</code> image). 
 * @param inValue the pixel value of the source pixel. 
 * @param layerX x-coordinate of quad that has been explored so far. It should contain the fine-grain pixel <code>inX,inY</code>. 
 * @param layerY y-coordinate of quad that has been explored so far. It should contain the fine-grain pixel <code>inX,inY</code>. 
 * @param depth depth of quad that has been explored so far. 
 * @param steepestYet
 * @return 
 */
double searchSiblingAndCoarserQuads(int inX, int inY, double inValue, int layerX, int layerY, int depth, double steepestYet) { 
  if (depth==layerCount) return steepestYet ; 
  int layerWidth = widths[depth] ; 
  int layerHeight = heights[depth] ; 
  int quadLength = quadLengths[depth] ; 
  int deltaX = getDeltaCoord(layerX, layerWidth); 
  int deltaY = getDeltaCoord(layerY, layerHeight); 
  //////  Explore the three sibling quads at this level. 
  if (deltaX!=0) { 
    if (deltaY!=0) { 
      steepestYet = fmax(steepestYet, searchSibling(inX, inY, inValue, layerX, layerY, depth, quadLength, 0, deltaY, steepestYet));
      steepestYet = fmax(steepestYet, searchSibling(inX, inY, inValue, layerX, layerY, depth, quadLength, deltaX, 0, steepestYet));
      steepestYet = fmax(steepestYet, searchSibling(inX, inY, inValue, layerX, layerY, depth, quadLength, deltaX, deltaY, steepestYet));
    } else { 
      steepestYet = fmax(steepestYet, searchSibling(inX, inY, inValue, layerX, layerY, depth, quadLength, deltaX, 0, steepestYet));
    }
  } else { 
    if (deltaY!=0) { 
      steepestYet = fmax(steepestYet, searchSibling(inX, inY, inValue, layerX, layerY, depth, quadLength, 0, deltaY, steepestYet));
    } else { 
      // Nothing required. 
    }
  }
  /* Here we know: The quad at the next coarsest layer, that contains this pixel, has been fully searched. */
  //////  Explore the coarser levels. 
  steepestYet = searchSiblingAndCoarserQuads(inX, inY, inValue, toCoarserCoord(layerX), toCoarserCoord(layerY), depth+1, steepestYet); 
  /* Here we know: The entire image has been searched. */
  //////  Bye bye 
  return steepestYet ; 
}

/**
 * Searches all pixels in a quad for a steep slope to the finest-grain pixel <code>inX,inY</code>. 
 * The pixel <code>inX,inY</code> must be outside the quad. 
 * 
 * @return The steepest slope (squared) found within the quad, or <code>steepestYet</code>, which ever is steeper. 
 */
double searchWholeQuad(int inX, int inY, double inValue, int quadX, int quadY, int depth, double steepestYet) { 
  //double* minValues = mins[depth] ; 
  if (depth>0) { 
    /* Here we know: We're checking a summary-layer image. */
    int quadLength = quadLengths[depth];
    //////  Check quad as a whole
    double steepestPossibleWholeQuad = computeSteepestPossibleSlopeToOther(inX, inY, inValue, quadX, quadY, depth, quadLength); 
    if (steepestYet>steepestPossibleWholeQuad) return steepestYet ; 
    //////  Check sub-quads
    int fineDepth = depth - 1 ; 
    int fineWidth = widths[fineDepth];
    int fineHeight = heights[fineDepth];
    int fineX0 = quadX * 2 ; 
    int fineY0 = quadY * 2 ; 
    /* Implementation note: the code would run faster if the quads nearest inX,inY were searched first. 
       However, the code is clearer as it is below. */
    if (fineX0+1<fineWidth) { 
      if (fineY0+1<fineHeight) { 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0, fineY0, fineDepth, steepestYet); 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0+1, fineY0, fineDepth, steepestYet); 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0, fineY0+1, fineDepth, steepestYet); 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0+1, fineY0+1, fineDepth, steepestYet); 
      } else { 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0, fineY0, fineDepth, steepestYet); 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0+1, fineY0, fineDepth, steepestYet); 
      }
    } else { 
      if (fineY0+1<fineHeight) { 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0, fineY0, fineDepth, steepestYet); 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0, fineY0+1, fineDepth, steepestYet); 
      } else { 
	steepestYet = searchWholeQuad(inX, inY, inValue, fineX0, fineY0, fineDepth, steepestYet); 
      }
    }
    return steepestYet ; 
  } else { 
    /* Here we know: We've reached the finest-grained image, ie, the inGrey image. */
    int distanceX = inX - quadX ; // Might be negative 
    int distanceY = inY - quadY ; // Might be negative 
    double distanceSqr = distanceX*distanceX + distanceY*distanceY ; 
    double otherValue = getPixel(inGrey, quadX, quadY, fullWidth); 
    double valueDiff = inValue - otherValue;
    double slopeSqr = valueDiff*fabs(valueDiff) / distanceSqr ; 
    return fmax(steepestYet, slopeSqr); 
  }
}
	
double* runSteepestTangent(double* inGrey_arg, int width, int height) { 
  inGrey = inGrey_arg ; 
  fullWidth = width ; 
  fullHeight = height ; 
  double* slopes = makeDoubles(fullWidth, fullHeight); 
  //////  Compute quad images
  layerCount = getQuadDepth(fullWidth, fullHeight); 
  mins = (double**)calloc(layerCount, sizeof(double*)); 
  maxs = (double**)calloc(layerCount, sizeof(double*)); 
  widths = (int*)calloc(layerCount, sizeof(int)); 
  heights = (int*)calloc(layerCount, sizeof(int)); 
  quadLengths = (int*)calloc(layerCount, sizeof(int)); 
  mins[0] = inGrey_arg ;
  maxs[0] = inGrey_arg ; 
  widths[0] = fullWidth ; 
  heights[0] = fullHeight ; 
  quadLengths[0] = 1 ;
  
  for (int depth=1 ; depth<layerCount ; depth++) { 
    fillQuadDepth(depth);  /* OMP speedup is put inside this function */
  }
  /* Here we know: The 'mins' and 'maxs' quad images are filled with the minimum and maximum pixel 
     values from the area in inGrey they cover. */
  //////  Find steepest (down) slopes

  /************* OMP ***************/
#pragma omp parallel for collapse(2)
  for (int y=0 ; y<fullHeight ; y++) { 
    for (int x=0 ; x<fullWidth ; x++) { 
      double sqrSlope = searchSiblingAndCoarserQuads(x, y, getPixel(inGrey, x, y, fullWidth), x, y, 0, 0.0f);
      setPixel(slopes, x, y, sqrt(sqrSlope), fullWidth); 
    }
  }
  //////  Free allocated memory
  for (int depth=1 ; depth<layerCount ; depth++) { 
    free(mins[depth]);
    free(maxs[depth]);
  }
  free(mins);
  free(maxs);
  free(widths);
  free(heights);
  free(quadLengths); 
  //////  Bye bye
  return slopes ; 
}


/***********************************************************************/
double* SteepestTangent(const uint8 *Image, const size_t  Nr, const size_t  Nc) {
  /* 
     This implements the recursive Steepest Tangent Alg, as described in the DICTA-18 paper. 
  */
/***********************************************************************/

  size_t  N = Nr*Nc; /* total number of pixels */
  double* grey = makeDoubles(Nc, Nr); 
  for (int i=0 ; i<N ; i++) grey[i] = Image[i] ; 
  double* slopes = runSteepestTangent(grey, Nc, Nr); 
  free(grey);
  return slopes ;
} /* SteepestTangent() */



