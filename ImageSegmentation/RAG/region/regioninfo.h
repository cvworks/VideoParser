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
 * Sven Wachsmuth, 03.06.2003 (V0.3 - added region_contourByMaskOp)
 */
#ifndef _REGION_INFO_H
#define _REGION_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ppm.h>

typedef struct contour_point_t {
  int x;
  int y;
} contour_point_t;

typedef struct region_info_t {
  int pixelcount;
  int sum_x;
  int sum_y;
  int start_x;
  int start_y;
  int started;

  int color;
  int label;

  float center_x;
  float center_y;
  float m20, m02, m11;

  int              max_contour;
  int              n_contour;
  contour_point_t *contour;

  int min_x, min_y;
  int max_x, max_y;

  int  max_subLabels;
  int  n_subLabels;
  int *subLabels;

} region_info_t;

  /* region_normalizeRegionMap():
   * Re-assigns the regionMap by consecutive labels [0,1,...].
   * Scans the image and assigns labels in left-right, top-down order.
   * - takes a regionMap
   * - takes the maximum label in regionMap
   * - optionally takes memory space for assignment table of size max_label
   * -> stores new labels in regionMap
   */
void region_normalizeRegionMap(int *regionMap, int cols, int rows,
			       int max_label, int *_table);

  /* region_backgroundXLabel():
   * Detect background label by calculating a histogram over all marginal
   * Pixels and taking the maximum component
   * - takes labelMap (size cols x rows)
   * - takes maximum label in labelMap
   * - optionally takes memory space for label histogram (size max_label)
   * => returns most frequent label on all marginal pixels in labelMap
   */
int region_backgroundIntLabel(int *labelMap, int cols, int rows, int offset,
			      int max_label, int *_count);
int region_backgroundCharLabel(unsigned char *labelMap, int cols, int rows,
			       int offset, int max_label, int *_count);

  /* region_info_...():
   * region_info_... initialization and memory functions for the
   * basic information about regions
   */
region_info_t *region_info_init(region_info_t *info);
region_info_t *region_info_create(void);
void           region_info_free(region_info_t *info);
void           region_info_destroy(region_info_t *info);
region_info_t *region_info_dup(region_info_t *info);
region_info_t *region_info_cpy(region_info_t *copy, region_info_t *info);
region_info_t *region_info_clear(region_info_t *info);

  /* region_countpixel():
   * basic function to start filling the region_info-structure.
   * Checks all non-marginal pixels of an image.
   * - takes an initialized array of regioninfo-structures
   * - takes a regionMap
   * - optionally takes a colorMap  
   * -> counts pixels of each region
   * -> stores starting pixel for each region
   * -> stores the color index of the starting pixel (if a colorMap exists)
   * -> stores the label of the region
   * -> computes sum_x and sum_y for each region (for calculating center)
   */
void region_countpixel(region_info_t **region_info,
		       int *regionMap, int xlen, int ylen,
		       unsigned char *colorMap);
void region_countpixelOLD(region_info_t *region_info,
			  int *regionMap, int xlen, int ylen,
			  unsigned char *colorMap);
  /* region_center():
   * - takes regioninfo-structure filled by region_pixelcount()
   * -> computes the center of the region
   */
void region_center(region_info_t *info);

  /* region_cmpMoments():
   * - takes two regioninfo-structures
   * - returns comparison of m11-moments (-1,0,1)
   */
int  region_cmpMoments(region_info_t *region1, region_info_t *region2);

  /* region_moments():
   * - takes regioninfo-structure of all regions filled by region_center()
   * - takes regionMap
   * - takes minNbPix (minimal pixelcount of a region)
   * -> computes and stores moments m02, m20, m11
   */
void region_moments(region_info_t **region_info, int n_regions,
		    int *regionMap, int xlen, int ylen, int minNbPix);
void region_momentsOLD(region_info_t *region_info, int n_regions,
		       int *regionMap, int xlen, int ylen, int minNbPix);

  /* region_contour():
   * Assumes a regionMap with all marginal pixels being set to an 
   * undefine label that corresponds to no <i_region> value.
   * This prevents the algorithm from testing special marginal conditions.
   * - takes region_info-structure filled by region_countpixel()
   * - takes regionMap
   * - takes label of region
   * - optionally takes memory-space for contour-pixels
   * -> computes and stores contour-pixels (8-neighborhood)
   */
void region_contour(region_info_t *region_info, int *regionMap,
		    int xlen, int ylen, int i_region,
		    contour_point_t *contour);

  /* region_minmax():
   * - takes regioninfo-structure filled by region_contour()
   * -> stores the minimal and maximal x and y coordinates of the pixels
   */
void region_minmax(region_info_t *info);

  /* region_contour2indexMap():
   * - takes initialized memory space for the indexMap (size of image)
   * - takes array of contour-pixels
   * -> stores array-index of contour-pixel in indexMap
   */
void region_contour2indexMap(int *indexMap, int cols,
			     int n_contour, contour_point_t *contour);
  /* region_indexMapP():
   * - takes query-pixel <px,py> and previous contour pixel <qx,qy>
   *   and queries an index-map with <cols> columns
   * -> returns pointer on indexMap-entry
   */
int *region_indexMapP(int *indexMap, int px, int py, int qx, int qy, int cols);

int *region_indexMap_realloc(int *indexMap, int cols, int rows);
int *region_indexMap_clear(int *indexMap, int cols, int rows);

  /* region_contour2image():
   * - takes initialized memory space <image> (size of image)
   * - takes array of contour-pixels
   * -> stores <value> at position of contour-pixel in <image>
   */
void region_contour2image(int *image, int cols, 
			  int n_contour, contour_point_t *contour, int value);
void region_contour2charImage(char *image, int cols, 
			      int n_contour, contour_point_t *contour,
			      char value);
void region_contour2ppm(pixel **pixels, int cols,
			int n_contour, contour_point_t *contour, int value);

  /* region_setSubLabel():
   * - set list of subLabels to [region->label]
   */
region_info_t *region_setSubLabel(region_info_t *region);

  /* region_clearSubLabels():
   * - set list of subLabels to []
   */
region_info_t *region_clearSubLabels(region_info_t *region);

  /* region_addSubLabel():
   * - adds <subLabel> to list of subLabels
   */
region_info_t *region_addSubLabel(region_info_t *region, int subLabel);
  
  /* region_addSubLabels():
   * - adds <subLabels> to list of subLabels
   */
region_info_t *region_addSubLabels(region_info_t *region, 
				   int *subLabels, int n_subLabels);

  /* region_areDisjunct():
   * - checks if the two subLabel sets are disjunct
   */
int region_areDisjunct(region_info_t *region1,
		       region_info_t *region2);


char *region_info_sprint(char *s, region_info_t *region);
int   region_info_sscan(region_info_t *region, char *s);

void region_contourByMaskOp(region_info_t *region_info, int *regionMap,
			    int xlen, int ylen, int i_region,
			    contour_point_t *contour,
			    int n_gap, int n_offset);

region_info_t *region_contourExpByMaskOp(region_info_t *region_exp,
					 region_info_t *region_info,
					 contour_point_t *contour,
					 int n_gap, int n_offset);

region_info_t *region_contourDilation(region_info_t *region_exp,
				      region_info_t *region_info,
				      contour_point_t *contour,
				      int n_gap);

#ifdef __cplusplus
}
#endif

#endif

