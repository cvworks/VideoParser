/*
 * regionmerge.h
 *
 * contains functions for merging regions
 *
 * Sven Wachsmuth, 5.2.2003, only planar graphs
 *
 * Sven Wachsmuth, 20.3.2003, un-restricted graphs
 */
#ifndef _REGION_MERGE_H
#define _REGION_MERGE_H

#include "regioninfo.h"
#include "adjacency.h"

#ifdef __cplusplus
extern "C" {
#endif

region_info_t *region_merge(region_info_t      *merged_region,
			    region_adjacency_t *merged_adjacencies,
			    region_adjacency_t *merged_rev_adjacencies,
			    region_adjacency_t *merge_adjacency1,
			    region_adjacency_t *merge_adjacency2,
			    region_adjacency_t *adjacencies1,
			    region_adjacency_t *adjacencies2,
			    int                n_adjacencies,
			    int                color,
			    int                label,
			    contour_point_t    *contour_points,
			    int                *n_contour_points);

int region_contour_merge(contour_point_t *contour, int *n_contour,
			 region_adjacency_t *merged_adjacencies,
			 region_adjacency_t *merged_rev_adjacencies,
			 region_adjacency_t *adj1,
			 region_adjacency_t *adjacencies1,
			 region_adjacency_t *adj2,
			 region_adjacency_t *adjacencies2,
			 int                n_adjacencies);

region_info_t *region_mergePt(region_info_t      *merged_region,
			      region_adjacency_t **merged_adjacencies,
			      region_adjacency_t **merged_rev_adjacencies,
			      region_adjacency_t *merge_adjacency1,
			      region_adjacency_t *merge_adjacency2,
			      region_adjacency_t **adjacencies1,
			      region_adjacency_t **adjacencies2,
			      int                n_adjacencies,
			      int                color,
			      int                label,
			      contour_point_t    *contour_points,
			      int                *n_contour_points);

int region_contour_mergePt(contour_point_t *contour, int *n_contour,
			   region_adjacency_t **merged_adjacencies,
			   region_adjacency_t **merged_rev_adjacencies,
			   region_adjacency_t *adj1,
			   region_adjacency_t **adjacencies1,
			   region_adjacency_t *adj2,
			   region_adjacency_t **adjacencies2,
			   int                n_adjacencies);

int region_merge_order(region_info_t *info1, region_info_t *info2);

void region_merge_adjustContour(int *end1, int *start2,
				region_info_t *region1,
				region_info_t *region2);

int region_pixel_connected(contour_point_t *p1, contour_point_t *p2);

region_info_t *region_mergeOutEdges2(region_info_t      *merged_region,
				     region_info_t      **regions,
				     int                n_regions,
				     region_adjacency_t **merged_adjacencies,
				     region_adjacency_t *merge_adjacency1,
				     region_adjacency_t *merge_adjacency2,
				     int                color,
				     int                label,
				     contour_point_t    *contour_points,
				     int                *n_contour_points,
				     int                *regionMap,
				     int                *indexMap,
				     int                cols, int rows);

int region_mergeInEdges2(region_adjacency_t **rev_adjacenciesM,
			 region_info_t **regions, int n_regions,
			 int merged_label, int label1, int label2,
			 region_adjacency_t **rev_adjacencies1);

#ifdef __cplusplus
}
#endif

#endif
