/*
 * regioninfo.c
 * adapted from regBuild.c (Elke Braun, Franz Kummert, Gernot Fink)
 *
 * contains functions for computing the following region information
 * - number of pixels
 * - center of region
 * - moments m20,m02,m11 of region
 * - contour pixels of region
 * - min and max coordinates of region pixels
 *
 * function dependencies are:
 * region_info_[create|init|clear]()
 * -> region_countpixel() -> region_center() 
 *                        -> region_moments()
 *                        -> region_contour() -> region_minmax()
 *                                            -> region_contour2indexMap()
 *                                            -> region_contour2image()
 *
 * Sven Wachsmuth, 26.11.2002 (V0.1)
 * Sven Wachsmuth, 24.02.2003 (V0.2 - re-organization)
 * Sven Wachsmuth, 11.07.2003 (V0.3 - added region_contourByMask,
 *                                    added region_contour2ppm
 *                                    minor corrections)
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "regioninfo.h"

#define MAXLOOP    50000

#define _STRCAT(s,t,n,max) \
(strcat(((n+=strlen(t)) >= max) ? s=realloc(s, max=n+1) : s, t))


void *_region_memcpyPtr(int *max_copy, void *copy, int _max_copy, 
			void *mem, int max_mem, int n_mem, int memsize)
{
  /** if mem internal memory, realloc and copy it */ 
  if (max_mem > 0) {
    if (_max_copy >= n_mem) {
      (*max_copy) = _max_copy;
    } else if (_max_copy > 0) {
      copy = (void *) realloc(copy, n_mem * memsize);
      (*max_copy) = n_mem;
    } else {
      copy = (void *) malloc(n_mem * memsize);
      (*max_copy) = n_mem;
    }
    if (mem) memcpy(copy, mem, n_mem * memsize);
    return (copy);
  }
  /** else, free copy (if internal memory) and return mem */
  else {
    if (copy && _max_copy > 0) free(copy);
    return (mem);
  }
}

void region_normalizeRegionMap(int *regionMap, int cols, int rows,
			       int max_label, int *_table)
{
  int i,j,n=cols*rows;
  int *table = (_table) ? table : (int *) malloc(max_label * sizeof(int));
  for (i=0; i < max_label; i++) table[i] = -1;

  for (j=0,i=0; i < n; i++) {
    int label = regionMap[i];
    if (table[label] < 0) table[label] = j++;
    regionMap[i] = table[label];
  }
  if (!_table) free(table);
}

int region_backgroundIntLabel(int *labelMap, int cols, int rows, int offset, 
			      int max_label, int *_count)
{
  int i,j, n, l, m;
  int max = 0, i_max = -1;
  int *count = (_count) ? _count : (int *) malloc(max_label * sizeof(int));
  memset(count,0,max_label * sizeof(int));

  if (offset < 0) offset = 0; /* default */
  if ((2 * offset) >= cols || (2 * offset) >= rows) return (-1);

  labelMap += cols * offset + offset;  /* adjust starting point */
  l = rows - 2 * offset;      /* number of remaining rows */
  m = cols - 2 * offset;      /* last pixel in a row (=m-1) */
  n = cols * l - 2 * offset;  /* last pixel in the image (=n-1) */
  
  { /* count first and last row */
    int *labelMap_ptr1 = labelMap;
    int *labelMap_ptrN = &labelMap[n-m];
    for (i=0; i < m; i++) {
      count[*(labelMap_ptr1++)]++;
      count[*(labelMap_ptrN++)]++;
    }
    /* count first and last column */
    l-= 2; /* first row and last row were already checked */
    labelMap_ptr1 = &labelMap[cols];
    labelMap_ptrN = &labelMap[cols+m-1];
    for (i=0; i < l; i++) {
      count[*labelMap_ptr1]++; labelMap_ptr1+=cols;
      count[*labelMap_ptrN]++; labelMap_ptrN+=cols;
    }
  }
  /* search for maximum count */
  for (i=0; i < max_label; i++)
    if (count[i] > max) {
      max = count[i]; i_max = i;
    }

  if (!_count) free(count);
  return (i_max);
}

int region_backgroundCharLabel(unsigned char *labelMap, int cols, int rows, 
			       int offset, int max_label, int *_count)
{
  int i,j, n, l, m;
  int max = 0, i_max = -1;
  int *count = (_count) ? _count : (int *) malloc(max_label * sizeof(int));
  memset(count,0,max_label * sizeof(int));

  if (offset < 0) offset = 0; /* default */
  if ((2 * offset) >= cols || (2 * offset) >= rows) return (-1);

  labelMap += cols * offset + offset;  /* adjust starting point */
  l = rows - 2 * offset;      /* number of remaining rows */
  m = cols - 2 * offset;      /* last pixel in a row (=m-1) */
  n = cols * l - 2 * offset;  /* last pixel in the image (=n-1) */
  
  { /* count first and last row */
    unsigned char *labelMap_ptr1 = labelMap;
    unsigned char *labelMap_ptrN = &labelMap[n-m];
    for (i=0; i < m; i++) {
      count[*(labelMap_ptr1++)]++;
      count[*(labelMap_ptrN++)]++;
    }
    /* count first and last column */
    l-= 2; /* first row and last row were already checked */
    labelMap_ptr1 = &labelMap[cols];
    labelMap_ptrN = &labelMap[cols+m-1];
    for (i=0; i < l; i++) {
      count[*labelMap_ptr1]++; labelMap_ptr1+=cols;
      count[*labelMap_ptrN]++; labelMap_ptrN+=cols;
    }
  }
  /* search for maximum count */
  for (i=0; i < max_label; i++)
    if (count[i] > max) {
      max = count[i]; i_max = i;
    }

  if (!_count) free(count);
  return (i_max);
}

region_info_t *region_info_init(region_info_t *info)
{
  if (!info)
    info = (region_info_t *) malloc(sizeof(region_info_t));

  memset(info, 0, sizeof(region_info_t));
  return (info);
}

region_info_t *region_info_create(void)
{
  return region_info_init(NULL);
}

void region_info_free(region_info_t *info)
{
  if (!info) return;
  if (info->contour && info->max_contour > 0)
    free(info->contour);
  if (info->subLabels && info->max_subLabels > 0)
    free(info->subLabels);
}

void region_info_destroy(region_info_t *info)
{
  if (!info) return;
  region_info_free(info);
  free(info);
}

region_info_t *region_info_dup(region_info_t *info)
{
  return (region_info_cpy(NULL, info));
}

region_info_t *region_info_cpy(region_info_t *copy, region_info_t *info)
{
  contour_point_t *_contour = NULL;
  int              max_contour = 0;
  int *            _subLabels = NULL;
  int              max_subLabels = 0;

  if (!info) return (copy);
  if (!copy)
    copy = region_info_create();

  _contour = copy->contour;
  max_contour = copy->max_contour;
  _subLabels = copy->subLabels;
  max_subLabels = copy->max_subLabels;

  memcpy(copy, info, sizeof(region_info_t));

  copy->contour = (contour_point_t *)
    _region_memcpyPtr(&copy->max_contour,
		      _contour, max_contour,
		      info->contour, info->max_contour,
		      info->n_contour, sizeof(contour_point_t));
  copy->subLabels = (int *)
    _region_memcpyPtr(&copy->max_subLabels,
		      _subLabels, max_subLabels,
		      info->subLabels, info->max_subLabels,
		      info->n_subLabels, sizeof(int));

  return (copy);
}

region_info_t *region_info_clear(region_info_t *info)
{
  int max_contour = 0;
  contour_point_t *_contour = NULL;
  int max_subLabels = 0;
  int *_subLabels = NULL;

  if (!info) return (NULL);

  max_contour = info->max_contour;
  _contour    = info->contour;
  max_subLabels = 0;
  _subLabels  = info->subLabels;

  region_info_init(info);

  info->max_contour = max_contour;
  info->contour = _contour;
  info->max_subLabels = max_subLabels;
  info->subLabels = _subLabels;

  return (info);
}

void region_countpixel(region_info_t **region_info,
		       int *regionMap, int xlen, int ylen,
		       unsigned char *colorMap)
{ 
 /* Image scanned in rows (left-right, top-down) in order to find
  * starting points for regions (used in region_contour()). 
  * Stores pixelcount, sum_x, sum_y for each region.
  * Starts at pixel (1,1) and stops at pixel (xlen-2,ylen-2)
  * leaving a one pixel frame unchecked.
  * Pixels with negative values are ignored.
  */
  int i,k;
  int *imagpntr;
  region_info_t *infopntr;
  int cols = xlen-1;
  int rows = ylen-1;

  imagpntr = regionMap+xlen+1;	/* start at pixel (1,1) */
  for (i=1; i<rows; ++i)
    {
      for (k=1; k<cols; ++k, imagpntr++) 
	{
	  if (*imagpntr >= 0)
	    {
	      infopntr = region_info[*imagpntr];
	      (infopntr->pixelcount)++;
	      (infopntr->sum_x) += k;
	      (infopntr->sum_y) += i;
	
	      /* if region did not occure before, then ... */
	      if (infopntr->started == 0)
		{
		  infopntr->color = (colorMap) ? colorMap[i*xlen+k]: *imagpntr;
		  infopntr->label = *imagpntr;
		  infopntr->start_x = k;
		  infopntr->start_y = i;
		  infopntr->started = 1;
		}
	    }
	}
      /* at the end of a row the last pixel of this row and the first
       * pixel of the next row are not checked */
      imagpntr+=2; 
    }
}

void region_countpixelOLD(region_info_t *region_info,
			  int *regionMap, int xlen, int ylen,
			  unsigned char *colorMap)
{ 
 /* Image scanned in rows (left-right, top-down) in order to find
  * starting points for regions (used in region_contour()). 
  * Stores pixelcount, sum_x, sum_y for each region.
  * Starts at pixel (1,1) and stops at pixel (xlen-2,ylen-2)
  * leaving a one pixel frame unchecked.
  * Pixels with negative values are ignored.
  */
  int i,k;
  int *imagpntr;
  region_info_t *infopntr;
  int cols = xlen-1;
  int rows = ylen-1;

  imagpntr = regionMap+xlen+1;	/* start at pixel (1,1) */
  for (i=1; i<rows; ++i)
    {
      for (k=1; k<cols; ++k, imagpntr++) 
	{
	  if (*imagpntr >= 0)
	    {
	      infopntr = &(region_info[*imagpntr]);
	      (infopntr->pixelcount)++;
	      (infopntr->sum_x) += k;
	      (infopntr->sum_y) += i;
	
	      /* if region did not occure before, then ... */
	      if (infopntr->started == 0)
		{
		  infopntr->color = (colorMap) ? colorMap[i*xlen+k]: *imagpntr;
		  infopntr->label = *imagpntr;
		  infopntr->start_x = k;
		  infopntr->start_y = i;
		  infopntr->started = 1;
		}
	    }
	}
      /* at the end of a row the last pixel of this row and the first
       * pixel of the next row are not checked */
      imagpntr+=2; 
    }
}

void region_center(region_info_t *info)
{
  if (!info || info->pixelcount == 0) return;

  info->center_x = ((float) info->sum_x) / ((float) info->pixelcount);
  info->center_y = ((float) info->sum_y) / ((float) info->pixelcount);
}

int  region_cmpMoments(region_info_t *region1,
		       region_info_t *region2)
{
  if (!region1 && !region2) return 0;
  if (!region1) return (-1);
  if (!region2) return ( 1);

  if (region1->m11 < region2->m11)
    return (-1);
  else if (region1->m11 > region2->m11)
    return (1);
  else
    return (0);
}

void region_moments(region_info_t **region_info, int n_regions,
		    int *regionMap, int xlen, int ylen, int minNbPix)
{
  int i,j,k;
  int *imagpntr;
  region_info_t *infopntr;

  for (i=0; i < n_regions; i++) {
    region_center(region_info[i]);
  }
  /* Ueber das Bild laufen und aus Punkten mit Hilfe der Schwerpunkte
     Momente berechnen */
  for (i=0, imagpntr = regionMap; i<ylen; ++i) 
    {
      for (k=0; k<xlen; ++k, ++imagpntr)
	{
	  if (*imagpntr < 0) 
	    continue;
	  infopntr = region_info[*imagpntr];
	  if (infopntr->pixelcount > minNbPix)
	    {
	      infopntr->m02 += (i - infopntr->center_y) * 
		(i - infopntr->center_y);
	      infopntr->m20 += (k - infopntr->center_x) * 
		(k - infopntr->center_x);
	      infopntr->m11 += (k - infopntr->center_x) * 
		(i - infopntr->center_y);
	    }
	}
    }
}

void region_momentsOLD(region_info_t *region_info, int n_regions,
		       int *regionMap, int xlen, int ylen, int minNbPix)
{
  int i,j,k;
  int *imagpntr;
  region_info_t *infopntr;

  for (i=0; i < n_regions; i++) {
    region_center(&region_info[i]);
  }
  /* Ueber das Bild laufen und aus Punkten mit Hilfe der Schwerpunkte
     Momente berechnen */
  for (i=0, imagpntr = regionMap; i<ylen; ++i) 
    {
      for (k=0; k<xlen; ++k, ++imagpntr)
	{
	  if (*imagpntr < 0) 
	    continue;
	  infopntr = &(region_info[*imagpntr]);
	  if (infopntr->pixelcount > minNbPix)
	    {
	      infopntr->m02 += (i - infopntr->center_y) * 
		(i - infopntr->center_y);
	      infopntr->m20 += (k - infopntr->center_x) * 
		(k - infopntr->center_x);
	      infopntr->m11 += (k - infopntr->center_x) * 
		(i - infopntr->center_y);
	    }
	}
    }
}

void region_minmax(region_info_t *info)
{
  int i;
  int min_x,min_y,max_x,max_y;

  if (!info || info->n_contour <= 0) return;

  min_x = max_x = info->contour[0].x;
  min_y = max_y = info->contour[0].y;

  for (i=1; i < info->n_contour; i++) {
    int x = info->contour[i].x;
    int y = info->contour[i].y;
    if (x < min_x) min_x = x;
    if (x > max_x) max_x = x;
    if (y < min_y) min_y = y;
    if (y > max_y) max_y = y;
  }
  info->min_x = min_x; info->min_y = min_y;
  info->max_x = max_x; info->max_y = max_y; 
}

void region_contour(region_info_t *region_info, int *regionMap,
		    int xlen, int ylen, int i_region,
		    contour_point_t *contour)
{
  /*
   * Assumes a regionMap with all marginal pixels being set to an 
   * undefine label that corresponds to no <i_region> value.
   * This prevents the algorithm from testing special marginal conditions.
   */
  int n_points;
  if (!region_info || region_info->pixelcount == 0)
    return;

  if (!contour) {
    /* assume that the longest possible contour consists of all image pixels
     */
    contour = (contour_point_t *)
      malloc((xlen*ylen) * sizeof(contour_point_t));
    region_info->max_contour = (xlen * ylen);
  } else {
    /* code in max_contour that memory space is externally allocated */
    region_info->max_contour = -1;
  }
  /* start contour finding at region starting point */
  n_points =
    _region_contour(contour, regionMap, xlen, ylen, region_info->start_x,
		    region_info->start_y, i_region);

  /* reallocate memory space for contour pixels */
  if (region_info->max_contour > 0) {
    contour = (contour_point_t *)
      realloc(contour, n_points * sizeof(contour_point_t));
    region_info->max_contour = n_points;
  }
  region_info->contour = contour;
  region_info->n_contour = n_points;
}

int _region_contour(contour_point_t *contour,
		    int *regionMap, int xlen, int ylen,
		    int start_x, int start_y,
		    int label)
{
  /* list of x- and y- offsets, that determines the next point to check
   * (depends on search direction)
   * if first test (0) does not succeed a second point (1) is checked.
   *                       
   * 010   ..0   01.   ...   0..   ...   .10   ...   ...
   * 1x1 1:.x1 2:.x. 3:.x. 4:1x. 5:.x1 6:.x. 7:.x. 8:1x.
   * 010   ...   ...   01.   ...   ..0   ...   .10   0..
   *
   */
  int xoffset[9][2] = {
    {0,0}, { 1, 1}, {-1, 0}, {-1,0}, {-1,-1}, {1,1}, { 1, 0}, {1,0}, {-1,-1}};
  int yoffset[9][2] = {
    {0.0}, {-1, 0}, {-1,-1}, { 1,1}, {-1, 0}, {1,0}, {-1,-1}, {1,1}, { 1, 0}};

  /* table of next search direction 
   * (depends on previous search direction and if the contour pixel
   *  checked first was found (0) or no contour pixel was not (1),
   *  otherwise the search direction is kept)
   *
   * [0]:                          [1]:
   * (left)  1 -> 2 -> 8 -> 7 -> 1 (right) 1 -> 7 -> 8 -> 2 -> 1
   * (right) 3 -> 4 -> 6 -> 5 -> 3 (left)  3 -> 5 -> 6 -> 4 -> 3
   */
  int change_direction[9][2] = {
    {0,0}, { 2, 7}, { 8, 1}, { 4,5}, { 6, 3}, {3,6}, { 5, 4}, {1,8}, { 7, 2}};

  int act_x, act_y;
  int direction;
  /* start direction is always 4 */
  int start_direction = 4;
  int loopcount = 0;
  int searchCount;
  int n_points;
  contour_point_t *point;

  /* put starting point into contour */
  point = contour;
  point->x = start_x;
  point->y = start_y;
  n_points = 1;

  /* remember actual point */
  act_x = (int) start_x;
  act_y = (int) start_y;
  
  direction = start_direction;
  searchCount = 0;

  /* Loop, until contour closed ... */
  do {
    /* Check next image pixel ... */
    if (regionMap[(act_x + xoffset[direction][0]) +
		 (act_y + yoffset[direction][0]) * xlen] == label) {
      
      /* put point into contour and remember this point */
      act_x += xoffset[direction][0];
      act_y += yoffset[direction][0];
      
      point++; n_points++;
      point->x = act_x;
      point->y = act_y;
      
      /* new search direction */
      direction = change_direction[direction][0];
      searchCount = 0;
    }
    /* Check next image pixel ... */
    else if (regionMap[(act_x + xoffset[direction][1]) +
		      (act_y + yoffset[direction][1]) * xlen] == label) {
      
      /* put point into contour and remember actual point */
      act_x += xoffset[direction][1];
      act_y += yoffset[direction][1];
      
      point++; n_points++;
      point->x = act_x;
      point->y = act_y;
      
      /* keep search direction!! */
      searchCount = 0;
    }
    /* if no checked image pixel equals label ...
       --> change search direction ... */
    else 
      {
	direction = change_direction[direction][1];
	searchCount ++;
      }
    ++loopcount;
    /* Search until starting point was found or (error case) loopcount
       exceeds MAXLOOP */
  } while (searchCount < 4 &&       /* untested directions remain */
	   (n_points == 1           /* AND (search just started */
	    || act_x != start_x     /*      OR actual pt neq starting pt */ 
	    || act_y != start_y     /*      OR other search direction) */
	    || direction != start_direction) 
	   && loopcount < MAXLOOP   /* AND loopcount not exceeded */
	   );
  
  if (searchCount >= 4)
    {
#ifdef TRACE
      fprintf(stderr,"region_contour: isolated point: (%d/%d)\n", 
	      start_x, start_y);
#endif
      return(1);
    }

  if (loopcount >= MAXLOOP)
    {
#ifdef TRACE
      fprintf(stderr,
	      "region_contour: Regionenlabel %d with starting point at (%d, %d) not closed\n",
	      label, start_x, start_y);
#endif
      return(0);
    }

  return(n_points - 1); /* start-point was assigned as last point */
}

void region_contour2indexMap(int *indexMap, int cols,
			     int n_contour, contour_point_t *contour)
{
  int i;
  contour_point_t *q;

  if (!indexMap || !contour) return;

  q = &contour[n_contour-1];
  for (i=0; i < n_contour; i++) {
    contour_point_t *p = &contour[i];
    *region_indexMapP(indexMap,p->x,p->y,q->x,q->y,cols) = i+1;
    q = p;
  }
}

int *region_indexMapP(int *indexMap, int px, int py, int qx, int qy, int cols)
{
  /* the same pixel can be visited from 4 different directions */
  int _index[3][3] = { { 0,3,3 },
		       { 0,0,2 },
		       { 1,1,2 } };
  return (&indexMap[(py*cols+px)*4+_index[py-qy+1][px-qx+1]]);
}

int region_indexMapN(int *indexMap, int nx, int ny,
		     int px, int py, int qx, int qy, int cols)
{
  /* the same pixel can be visited from 4 different directions */
  int _index[3][3] = { { 0,3,3 },
		       { 0,0,2 },
		       { 1,1,2 } };
  int __index = _index[py-qy+1][px-qx+1];
  int *_indexMap = &indexMap[(ny*cols+nx)*4];
  int index=0;
  int i;

  /* return the next defined value (turn direction clockwise) 
   * . in case of 4 values       xq  x
   *                              xpx          x
   *                               *     ->   * 
   *                              x x
   *                             x   x
   */
  for (i=0; i < 4; i++) {
    if ((--__index) < 0) __index = 3;
    if ((index=_indexMap[__index]) != 0) break;
  }
  return (index);
}
  
int *region_indexMap_realloc(int *indexMap, int cols, int rows)
{
  if (indexMap)
    return realloc(indexMap, cols*rows*4 * sizeof(int));
  else
    return malloc(cols*rows*4 * sizeof(int));
}

int *region_indexMap_clear(int *indexMap, int cols, int rows)
{
  if (!indexMap)
    indexMap = region_indexMap_realloc(NULL,cols,rows);
  memset(indexMap,0, cols*rows*4 * sizeof(int));
  return (indexMap);
}

void region_contour2image(int *image, int cols,
			  int n_contour, contour_point_t *contour,
			  int value)
{
  int i;
  if (!image || !contour) return;
  for (i=0; i < n_contour; i++) {
    int index = cols * contour[i].y + contour[i].x;
    image[index] = value;
  }
}

void region_contour2ppm(pixel **pixels, int cols,
			int n_contour, contour_point_t *contour,
			int value)
{
  int i;
  if (!pixels || !contour) return;
  for (i=0; i < n_contour; i++) {
    int index = cols * contour[i].y + contour[i].x;
    PPM_ASSIGN(pixels[contour[i].y][contour[i].x],value,value,value);
  }
}

void region_contour2charImage(char *image, int cols,
			      int n_contour, contour_point_t *contour,
			      char value)
{
  int i;
  if (!image || !contour) return;
  for (i=0; i < n_contour; i++) {
    int index = cols * contour[i].y + contour[i].x;
    image[index] = value;
  }
}

region_info_t *region_setSubLabel(region_info_t *region)
{
  if (!region) return (NULL);

  return region_addSubLabel(region_clearSubLabels(region),region->label);
}

region_info_t *region_clearSubLabels(region_info_t *region)
{
  if (region) 
    region->n_subLabels = 0;
  return (region);
}

region_info_t *region_addSubLabel(region_info_t *region, int subLabel)
{
  int n;
  if (!region) return (NULL);

  n = ++region->n_subLabels;

  region->subLabels = (int *)
    _region_memcpyPtr(&region->max_subLabels, 
		      region->subLabels, region->max_subLabels,
		      NULL, n, n, sizeof(int));
  region->subLabels[n-1] = subLabel;
  return (region);
}

region_info_t *region_addSubLabels(region_info_t *region, 
				   int *subLabels, int n_subLabels)
{
  int n;
  if (!region) return (NULL);

  n = (region->n_subLabels += n_subLabels);

  region->subLabels = (int *)
    _region_memcpyPtr(&region->max_subLabels, 
		      region->subLabels, region->max_subLabels,
		      NULL, n, n, sizeof(int));
  memcpy(region->subLabels + n - n_subLabels, subLabels,
	 n_subLabels * sizeof(int));

  return (region);
}

int region_areDisjunct(region_info_t *region1,
		       region_info_t *region2)
{
  int i,j;
  if (!region1 || !region2) return (1);
  
  for (i=0; i < region1->n_subLabels; i++)
    for (j=0; j < region2->n_subLabels; j++)
      if (region1->subLabels[i] == region2->subLabels[j]) 
	return (0);
  return (1);
}

char *region_info_sprint(char *s, region_info_t *region)
{
  char tmp[256];
  int  i,n,max;
  if (!region) return (s);

  sprintf(tmp,"region %d: C%d #%d X%d Y%d ^(%d,%d) *(%g,%g) [%d,%d..%d,%d] m(%g,%g,%g) [#%d",
	  region->label,
	  region->color,
	  region->pixelcount,
	  region->sum_x,
	  region->sum_y,
	  region->start_x, region->start_y,
	  region->center_x, region->center_y,
	  region->min_x, region->min_y, region->max_x, region->max_y,
	  region->m20, region->m02, region->m11,
	  region->n_subLabels);
  n   = strlen(tmp);
  max = n + region->n_subLabels * 3 + region->n_contour * 10 + 10;
  strcpy(s = realloc(s, max),tmp);

  for (i=0; i < region->n_subLabels; i++) {
    sprintf(tmp, " %d", region->subLabels[i]);
    s = _STRCAT(s,tmp,n,max);
  }
  sprintf(tmp, "] [#%d", region->n_contour);
  _STRCAT(s,tmp,n,max);

  for (i=0; i < region->n_contour; i++) {
    sprintf(tmp, " (%d,%d)", region->contour[i].x, region->contour[i].y);
    _STRCAT(s, tmp, n, max);
  }
  sprintf(tmp, "]");
  _STRCAT(s, tmp, n ,max);

  return (s);
}

int region_info_sscan(region_info_t *region, char *s)
{
  int i,j;
  int n_subLabels, n_contour;
  if (!region || !s) return (0);
  if ((i=sscanf(s,"region %d: C%d #%d X%d Y%d ^(%d,%d) *(%g,%g) [%d,%d..%d,%d] m(%g,%g,%g) [#%d", 
		&region->label,
		&region->color,
		&region->pixelcount,
		&region->sum_x,
		&region->sum_y,
		&region->start_x, &region->start_y,
		&region->center_x, &region->center_y,
		&region->min_x, &region->min_y, &region->max_x, &region->max_y,
		&region->m20, &region->m02, &region->m11,
		&n_subLabels))
      != 17) {
    fprintf(stderr,"region_info_sscan: illegal format (%d/15 values read).\n",i);
    return (0);
  }
  s = strstr(s,"[#");
  for (j=0; strchr(++s,' ') && j < n_subLabels; j++) {
    int subLabel;
    s = strchr(s,' ');
    if ((i=sscanf(s," %d", &subLabel)) != 1) {
      fprintf(stderr,"region_sscanf: illegal format (subLabels).\n");
      return (0);
    }
    region_addSubLabel(region, subLabel);
  }
  s = strstr(s,"[#");
  if (!s || sscanf(s,"[#%d", &n_contour) != 1) {
    fprintf(stderr,"region_sscanf: illegal format (#contour).\n");
    return (0);
  }
  if (region->max_contour < n_contour) {
    if (region->max_contour > 0) {
      region->contour = (contour_point_t *)
	realloc(region->contour, n_contour * sizeof(contour_point_t));
    } else {
      region->contour = (contour_point_t *)
	malloc(n_contour * sizeof(contour_point_t));
    }
    region->max_contour = n_contour;
  }
  for (j=0; strchr(++s,' ') && j < n_contour; j++) {
    int x,y;
    s = strchr(s,' ');
    if (sscanf(s," (%d,%d)", &x,&y) != 2) {
      fprintf(stderr,"region_info_sscan: illegal format (contour).\n");
      return (0);
    }
    if (region->n_contour > region->max_contour) {
      fprintf(stderr,"REGION CONTOUR ABORTED at %d!\n",j);
      break;
    }
    region->contour[j].x = x;
    region->contour[j].y = y;
    region->n_contour++;
  }
  return (1);
}

int _region_contourTurnMask(int xoffset[][9],
			    int yoffset[][9],
			    int xlookup[][9],
			    int ylookup[][9],
			    int change_direction[][9],
			    int n,
			    int d_from, int d_to, int d_next)
{
  int k;
  for (k=0; k <= n; k++) {
    xoffset[k][d_to] = -yoffset[k][d_from];
    yoffset[k][d_to] =  xoffset[k][d_from];
    xlookup[k][d_to] = -ylookup[k][d_from];
    ylookup[k][d_to] =  xlookup[k][d_from];
    change_direction[k][d_to] =
      (k == 0) ? d_from : 
      (change_direction[k][d_from] == d_from) ? d_to : d_next;
  }
  return (0);
}

int _region_contourSwitchMask(int xoffset[][9],
			      int yoffset[][9],
			      int xlookup[][9],
			      int ylookup[][9],
			      int change_direction[][9],
			      int n,
			      int d_from, int d_to, 
			      int d_next, int d_prev)
{
  int k;
  for (k=0; k <= n; k++) {
    xoffset[k][d_to] = -xoffset[k][d_from];
    yoffset[k][d_to] =  yoffset[k][d_from];
    xlookup[k][d_to] = -xlookup[k][d_from];
    ylookup[k][d_to] =  ylookup[k][d_from];
    change_direction[k][d_to] =
      (k == 0) ? d_prev : 
      (change_direction[k][d_from] == d_from) ? d_to : d_next;
  }
  return (0);
}



int _region_contourDefineMask(int xoffset[][9],
			      int yoffset[][9],
			      int xlookup[][9],
			      int ylookup[][9],
			      int change_direction[][9],
			      int *start_x, int *start_y,
			      int n_gap, int n_offset)
{
  int _xoffset[8][8]   = { { 0, 1, 1, 1, 1, 1, 1, 1 },
			   { 0, 0, 1, 1, 1, 1, 1, 1 },
		  	   { 0, 0, 0, 0, 0, 0, 0, 0 },
			   { 0, 0, 0, 0, 0, 0, 0, 0 },
			   { 0, 0, 0, 0, 0, 0, 0, 0 },
			   { 0, 0, 0, 0, 0, 0, 0, 0 },
			   { 0, 0, 0, 0, 0, 0, 0, 0 },
			   { 0, 0, 0, 0, 0, 0, 0, 0 } };
  int _yoffset[8][8]   = { {-1,-1, 0, 0, 0, 0, 0, 0 },
			   {-1,-1,-1,-1,-1,-1,-1,-1 },
			   {-1,-1,-1,-1,-1,-1,-1,-1 },
			   {-1,-1,-1,-1,-1,-1,-1,-1 },
			   {-1,-1,-1,-1,-1,-1,-1,-1 },
			   {-1,-1,-1,-1,-1,-1,-1,-1 },
			   {-1,-1,-1,-1,-1,-1,-1,-1 },
			   {-1,-1,-1,-1,-1,-1,-1,-1 } };
  int _direction[8][8] = { { 6, 5, 5, 5, 5, 5, 5, 5 },
			   { 6, 6, 5, 5, 5, 5, 5, 5 },
			   { 6, 6, 6, 6, 6, 6, 6, 6 },
			   { 6, 6, 6, 6, 6, 6, 6, 6 },
			   { 6, 6, 6, 6, 6, 6, 6, 6 },
			   { 6, 6, 6, 6, 6, 6, 6, 6 },
			   { 6, 6, 6, 6, 6, 6, 6, 6 },
			   { 6, 6, 6, 6, 6, 6, 6, 6 } };
  int _xlookup[8][7]   = { { 1, 2, 3, 4, 5, 6, 7 },     /* take n_gap+1 */
			   { 0, 1, 2, 3, 4, 5, 6 },     /* take n_gap+1 */
			   { 0, 1, 2, 3, 4, 5, 6 },     /* take n_gap   */
			   { 0, 1, 2, 3, 4, 5, 6 },     /* take n_gap-1 */
			   { 0, 1, 2, 3, 4, 5, 6 },     /* take n_gap-2 */
			   { 0, 1, 2, 3, 4, 5, 6 },     /* take n_gap-3 */
			   { 0, 1, 2, 3, 4, 5, 6 },     /* take n_gap-4 */
			   { 0, 0, 1, 2, 3, 4, 5 } };   /* take n_gap-5 */
  int _ylookup[8][7]   = { { 0, 0, 0, 0, 0, 0, 0 },     /* take n_gap+1 */
			   { 0, 1, 1, 1, 1, 1, 1 },     /* take n_gap+1 */
			   { 1, 2, 2, 2, 2, 2, 2 },     /* take n_gap   */
			   { 2, 3, 3, 3, 3, 3, 3 },     /* take n_gap-1 */
			   { 3, 4, 4, 4, 4, 4, 4 },     /* take n_gap-2 */
			   { 4, 5, 5, 5, 5, 5, 5 },     /* take n_gap-3 */
			   { 5, 6, 6, 6, 6, 6, 6 },     /* take n_gap-4 */
			   { 6, 7, 7, 7, 7, 7, 7 } };   /* take n_gap-5 */

  int i,j,k;
  *start_y -= n_offset;

  if (n_gap < n_offset) n_gap = n_offset;
  
  /* first define it for direction 6  (up) */

  /* ... zero case if no pixel is found */
  xlookup[0][6] = 0;
  ylookup[0][6] = 0;
  xoffset[0][6] = 0;
  yoffset[0][6] = 0;
  change_direction[0][6] = 4;

  /* ... first case (only relevant for n_offset > 0) */
  xlookup[1][6] = 0 - n_offset;
  ylookup[1][6] = 0;
  xoffset[1][6] = -1;
  yoffset[1][6] = -1;
  change_direction[1][6] = 6;  /* if it would be changed to 4 (left)
				* there are always loop cases */

  for (k=1,i=n_gap+(n_offset==0)?1:0,j=1; i >= 0; i--, j=n_gap+((i>0)?2:1)) {
    int h;
    for (h=0; h < j; h++) {
      int x = _xlookup[i][h];
      int y = _ylookup[i][h];
      xlookup[++k][6] = x - n_offset;
      ylookup[k][6] = -y - 1; 
      xoffset[k][6] = _xoffset[y][x];
      yoffset[k][6] = _yoffset[y][x];
      change_direction[k][6] = _direction[y][x];
    }
  }

  _region_contourTurnMask(xoffset,yoffset,xlookup,ylookup,change_direction,
			  k,6,5,3);
  _region_contourTurnMask(xoffset,yoffset,xlookup,ylookup,change_direction,
			  k,5,3,4);
  _region_contourTurnMask(xoffset,yoffset,xlookup,ylookup,change_direction,
			  k,3,4,6);
  _region_contourSwitchMask(xoffset,yoffset,xlookup,ylookup,change_direction,
			    k,6,2,8,1);
  _region_contourTurnMask(xoffset,yoffset,xlookup,ylookup,change_direction,
			  k,2,8,7);
  _region_contourTurnMask(xoffset,yoffset,xlookup,ylookup,change_direction,
			  k,8,7,1);
  _region_contourTurnMask(xoffset,yoffset,xlookup,ylookup,change_direction,
			  k,7,1,2);
  return (k);
}

int _region_contourByMaskOp(contour_point_t *contour,
			    int *regionMap, int cols,
			    int start_x, int start_y,
			    int n_check, int i_check,
			    int label,
			    int xoffset[][9],
			    int yoffset[][9],
			    int xlookup[][9],
			    int ylookup[][9],
			    int change_direction[][9])
{
  int act_x, act_y;
  int direction;
  /* start direction is always 4 */
  int start_direction = 4;
  int loopcount = 0;
  int searchCount;
  int n_points;
  contour_point_t *point;

  /* put starting point into contour */
  point = contour;
  point->x = start_x;
  point->y = start_y;
  n_points = 1;

  /* remember actual point */
  act_x = (int) start_x;
  act_y = (int) start_y;

  /*fprintf(stderr," PUT[%d]:%d,%d\n",label,act_x,act_y);*/

  direction = start_direction;
  searchCount = 0;

  /* Loop, until contour closed ... */
  do {
    int i;
    /* Check next image pixel ... */
    for (i=n_check; i >= i_check; i--) {
      
      /*fprintf(stderr," CHECK[%d,%d]%d,%d",direction,i,act_x + xlookup[i][direction],
       *      act_y + ylookup[i][direction]);
       */
      if (regionMap[act_x + xlookup[i][direction] +
		   (act_y + ylookup[i][direction]) * cols] == label) {
	
	/* put point into contour and remember this point */
	act_x += xoffset[i][direction];
	act_y += yoffset[i][direction];

	/*fprintf(stderr," CHECK[%d][%d,+%d,+%d]:\n",
	 *	direction,
	 *	i,xlookup[i][direction],ylookup[i][direction]);
	 *fprintf(stderr," PUT[%d][%d,+%d,+%d]:%d,%d\n",
	 *	label,
	 *	i,xoffset[i][direction],yoffset[i][direction],act_x,act_y);
	 */
	point++; n_points++;
	point->x = act_x;
	point->y = act_y;

	/* new search direction */
	direction = change_direction[i][direction];
	searchCount = 0;
	break;
      }
    } 
    /* if no checked image pixel equals label ...
       --> change search direction ... */
    if (i < i_check) {
      direction = change_direction[0][direction];
      searchCount++;
    }
    ++loopcount;

    /* Search until starting point was found or (error case) loopcount
       exceeds MAXLOOP */
  } while (searchCount < 4 &&       /* untested directions remain */
	   (n_points == 1           /* AND (search just started */
	    || act_x != start_x     /*      OR actual pt neq starting pt */ 
	    || act_y != start_y     /*      OR other search direction) */
	    || direction != start_direction) 
	   && loopcount < MAXLOOP   /* AND loopcount not exceeded */
	   );

  if (loopcount >= MAXLOOP)
    {
#ifdef TRACE
      fprintf(stderr,
	      "region_contour: Regionenlabel %d with starting point at (%d, %d) not closed\n",
	      label, start_x, start_y);
#endif
      return(0);
    }

  return(n_points - 1); /* start-point was assigned as last point */
}
  
void region_contourByMaskOp(region_info_t *region_info, int *regionMap,
			    int xlen, int ylen, int i_region,
			    contour_point_t *contour,
			    int n_gap, int n_offset)
{
  /*
   * Assumes a regionMap with all marginal pixels being set to an 
   * undefine label that corresponds to no <i_region> value.
   * This prevents the algorithm from testing special marginal conditions.
   */
  int n_points;
  if (!region_info || region_info->pixelcount == 0)
    return;

  if (!contour) {
    /* assume that the longest possible contour consists of all image pixels
     */
    contour = (contour_point_t *)
      malloc((xlen*ylen) * sizeof(contour_point_t));
    region_info->max_contour = (xlen * ylen);
  } else {
    /* code in max_contour that memory space is externally allocated */
    region_info->max_contour = -1;
  }
  {
    int xoffset[64][9];
    int yoffset[64][9];
    int xlookup[64][9];
    int ylookup[64][9];
    int change_direction[64][9];
    int n_mask;

    /* get starting point */
    int start_x = region_info->start_x;
    int start_y = region_info->start_y;
    
    /* define masks for contour searching */
    n_mask = _region_contourDefineMask(xoffset,
				       yoffset,
				       xlookup,
				       ylookup,
				       change_direction,
				       &start_x,
				       &start_y,
				       n_gap, n_offset);
    /*{
      int i,j;
      fprintf(stderr,"MASKS: \n");
      fprintf(stderr,"  (XLOOKUP,YLOOKUP;XOFFSET,YOFFSET;DIRECTION): \n");
      for (j=1; j < 9; j++) {
	fprintf(stderr,"%d:\t",j);
	for (i=0; i <= (n_gap+1)*(n_gap+2); i++) {
	  fprintf(stderr," (%d,%d;%d,%d;%d)", 
		  xlookup[i][j],ylookup[i][j],xoffset[i][j],yoffset[i][j],
		  change_direction[i][j]);
	}
	fprintf(stderr,"\n");
      }
      }*/

    /* start contour finding at region starting point */
    n_points =
      _region_contourByMaskOp(contour, regionMap, xlen, start_x, start_y, 
			      n_mask,(n_offset > 0) ? 1:2,
			      i_region,
			      xoffset, yoffset, xlookup, ylookup,
			      change_direction);
  }
  /* reallocate memory space for contour pixels */
  if (region_info->max_contour > 0) {
    contour = (contour_point_t *)
      realloc(contour, n_points * sizeof(contour_point_t));
    region_info->max_contour = n_points;
  }
  region_info->contour = contour;
  region_info->n_contour = n_points;
}

region_info_t *region_contourExpByMaskOp(region_info_t *region_exp,
					 region_info_t *region_info,
					 contour_point_t *contour,
					 int n_gap, int n_offset)
{
  if (!region_info) return (region_exp);

  region_exp = region_info_cpy(region_exp,region_info);
  {
    int x = region_info->min_x - n_gap - 1;
    int y = region_info->min_y - n_gap - 1;
    int cols = region_info->max_x - x + 2 * n_gap + 2;
    int rows = region_info->max_y - y + 2 * n_gap + 2;
    int *map = malloc(cols*rows * sizeof(int));
    int *map_base = map - y * cols - x;
    memset(map,0,cols*rows*sizeof(int));
    region_contour2image(map_base, cols, region_info->n_contour,
			 region_info->contour, region_info->label);
    region_contourByMaskOp(region_exp, map_base, cols, rows,
			   region_info->label, contour, n_gap, n_offset);
    free(map);
  }
  return (region_exp);
}

region_info_t *region_contourDilation(region_info_t *region_exp,
				      region_info_t *region_info,
				      contour_point_t *contour,
				      int n_gap)
{
  if (!region_info) return (region_exp);

  region_exp = region_info_cpy(region_exp,region_info);
  {
    int i,j;
    int x = region_info->min_x - n_gap - 1;
    int y = region_info->min_y - n_gap - 1;
    int cols = region_info->max_x - x + 2 * n_gap + 2;
    int rows = region_info->max_y - y + 2 * n_gap + 2;
    int *map = malloc(cols*rows * sizeof(int));
    int *map_base = map - y * cols - x;
    memset(map,0,cols*rows*sizeof(int));
    for (i=-n_gap; i <= n_gap; i++)
      for (j=-n_gap; j <= n_gap; j++) {
	int *_map_base = map_base + cols * i + j;
	region_contour2image(_map_base, cols, region_info->n_contour,
			     region_info->contour, region_info->label);
      }
    region_exp->start_x-=n_gap;
    region_exp->start_y-=n_gap;

    region_contour(region_exp, map_base, cols, rows,
		   region_info->label, contour);
    free(map);
  }
  return (region_exp);
}
  

