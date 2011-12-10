#ifndef REGION_POLYGON_H
#define REGION_POLYGON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "regioninfo.h"

int region_polygon(contour_point_t *polygon, 
		   contour_point_t *contour, int n_contour,
		   float max_d);

int region_filterContour(contour_point_t *contour, int n_contour);

#ifdef __cplusplus
}
#endif

#endif
