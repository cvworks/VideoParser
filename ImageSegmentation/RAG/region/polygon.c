#include <math.h>
#include "polygon.h"


void _region_polygon(contour_point_t *polygon, int *n_polygon,
		     contour_point_t *contour,
		     int i_start, int i_end, float max_d)
{
  int w;
  float dw = 0;

  /*
   * if start and end point identical, then the most distant point
   * defines the direction of the line between both points
   */
  if (contour[i_start].x == contour[i_end].x
      && contour[i_start].y == contour[i_end].y) {
    int i;
    float d;
    for (i=i_start+1; i < i_end; i++) {
      float dx = contour[i].x - contour[i_start].x;
      float dy = contour[i].y - contour[i_start].y;
      float d  = dx*dx + dy*dy;
      if (d > dw) {
	w = i;
	dw  = d;
      }
    }
    dw = sqrt((double) dw);
  }
  else {
    float dx = contour[i_end].x - contour[i_start].x;
    float dy = contour[i_start].y - contour[i_end].y;
    float a = sqrt((double) dx * dx + dy * dy);
    float sinus = dx / a;
    float cosinus = dy / a;
    
    /* search for the most distant point to the line between i_start..i_end */
    int i;
    float d;
    dw = 0;
    for (i = i_start + 1; i < i_end; i++) {
      // WHY is cosinus and sinus added ????
      float d = ((contour[i].x - contour[i_start].x) * cosinus +
		 (contour[i].y - contour[i_start].y) * sinus);
      if (fabs((double) d) > dw) {
	w = i;
	dw = fabs((double) d);
      }
    }
  }
  
  /* if distance is above a threshold, keep the point and search both
     intervals */
  if (dw > max_d) {
    _region_polygon(polygon, n_polygon, contour, i_start, w, max_d);
    _region_polygon(polygon, n_polygon, contour, w, i_end, max_d);
  }
  else {
    /* skip all points between i_start and i_end */
    polygon[(*n_polygon)++] = contour[i_end];
  }
}

int region_polygon(contour_point_t *polygon, 
		   contour_point_t *contour, int n_contour,
		   float max_d)
{
  int n_polygon=1;
  if (!polygon || !contour || n_contour <= 0) return 0;

  polygon[0] = contour[0];
  if (n_contour > 1)
    _region_polygon(polygon, &n_polygon, contour, 0, n_contour-1, max_d);

  return (n_polygon);
}

int region_filterContour(contour_point_t *contour, int n_contour)
{
  int i;

  int ip = n_contour-2;
  for (i=0; i < n_contour-1; i++) {
    contour_point_t *p = &contour[ip];
    contour_point_t *q = &contour[i];
    
    while (region_pixel_connected(p,q) && ip != i) {
      if ((--ip) < 0) ip = n_contour-1;
      p = &contour[ip];
    }
    if (ip >= i) {
      int j,k;
      if (ip < n_contour-1) n_contour = ip+2;
      for (k=(ip < n_contour-1) ? 0 : 1,j=i; j != k && j < n_contour; j++) {
	contour[k++] = contour[j];
      }
      if (j!=k) n_contour = k;
      ip = (i==0) ? n_contour - 1 : i-1;
    } else {
      int j,k;
      for (k=ip+2, j=i; j != k && j < n_contour; j++) {
	contour[k++] = contour[j]; 
      }
      if (j!=k) n_contour = k;
      ip = i-1;
    }
  }
  return (n_contour);
}

    

