/*
 * region/rag_mem.h
 *
 * Sven Wachsmuth, 08.04.2003
 *
 * maintenance of memory for regions, adjacencies, and merging
 */
#ifndef _REGION_RAGMEM_H
#define _REGION_RAGMEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <pnmadd/pnmadd.h>

#include "regioninfo.h"
#include "adjacency.h"
#include "regionmerge.h"

typedef struct {
  float r,g,b;
} rag_color_t;

#define RAG_TMP_ADJLISTS (4)
#define RAG_TMP_REGLISTS (2)

typedef struct rag_t {

  ppm_t         *image;

  unsigned char *colorMap;
  int            max_colorMap;

  int           *regionMap;
  int            max_regionMap;

  int           *contourIndexMap;
  int            max_contourIndexMap;

  int            cols, rows;

  rag_color_t  *colorTab;
  int           max_colorTab;
  int           n_colors;

  contour_point_t *contour_points;
  int              max_contour_points;
  int              n_contour_points;

  region_info_t       *region;
  int                  max_regions;
  int                  n_regions;
  
  int                 *region_index;
  int                  max_region_index;
  int                  n_region_index;

  region_adjacency_t  *adjacency_list;
  int                  max_adjacency_list;
  int                  n_adjacency_list;

  int                 *adjacency_index;
  int                  max_adjacency_index;
  int                  n_adjacency_index;

  // for merging and processing ...
  region_info_t **tmp_regions[RAG_TMP_REGLISTS];
  int             max_tmp_regions[RAG_TMP_REGLISTS];

  region_adjacency_t **tmp_adjacencies[RAG_TMP_ADJLISTS];
  int                max_tmp_adjacencies[RAG_TMP_ADJLISTS];

} rag_t;

rag_t *rag_init(rag_t *rag);
rag_t *rag_initN(rag_t *rag, 
		 int n,             /* number of regions */ 
		 int m              /* number of adjacencies */
		 );
rag_t *rag_create(void);
rag_t *rag_createN(int n,           /* number of regions */  
		   int m            /* number of adjacencies */
		   );
rag_t *rag_realloc(rag_t *rag, 
		   int n,           /* number of regions */ 
		   int m            /* number of adjacencies */
		   );

void   rag_free(rag_t *rag);
void   rag_destroy(rag_t *rag);

rag_t *rag_cpy(rag_t *copy, const rag_t *rag);
rag_t *rag_cpyPt(rag_t *copy, const rag_t *rag);
  /* rag_cpyPt: copies only the references on region and adjacency memory */
rag_t *rag_dup(const rag_t *rag);
rag_t *rag_dupPt(const rag_t *rag);
  /* rag_dupPt: calls rag_cpyPt(NULL,rag) */
rag_t *rag_movPt(rag_t *copy, rag_t *rag);
  /* rag_movPt: moves the region and adjacency memory to copy and leaves
   *   rag with references on the memory */
  
rag_t *rag_clear(rag_t *rag);
rag_t *rag_clearRegions(rag_t *rag);

ppm_t *                           /* returns the previously stored image */
rag_setImage(rag_t *rag, 
	     ppm_t *image      /* sets reference on this image */
	     );

unsigned char *   /* returns previous colorMap, if stored by reference */
rag_setColorMap(rag_t *rag, 
		unsigned char *colorMap,  /* new colorMap */
		int max_colorMap,    /* if (-1) set reference on colorMap
				      * if (-2) copy colorMap
				      * if (>0) keep colorMap memory */
		int cols, int rows
		);
  
int *             /* returns previous regionMap, if stored by reference */
rag_setRegionMap(rag_t *rag, 
		 int *regionMap,    /* new regionMap */
		 int max_regionMap, /* if (-1) set reference on regionMap
				     * if (-2) copy regionMap
				     * if (>0) keep regionMap memory */
		 int cols, int rows
		 );

int *             /* returns previous regionMap, if stored by reference */
rag_setContourIndexMap(rag_t *rag, 
		       int *contourIndexMap, /* new contourIndexMap */
		       int max_contourIndexMap, /* if (-1) set reference
						 * if (-2) copy mem
						 * if (>0) keep mem */
		       int cols, int rows
		       );

rag_color_t *     /* returns previous colorTab, if stored by reference */
rag_setColorTab(rag_t *rag, 
		rag_color_t *colorTab, /* new colorTab */
		int max_colorTab,         /* if (-1) set reference
					   * if (-2) copy mem
					   * if (>0) keep mem */
		int n_colors
		);
rag_color_t *
rag_setColorTabByPixel(rag_t *rag,
		       pixel *colorTab, /* new colorTab */
		       int n_colors
		       );

rag_color_t *rag_newColor(rag_t *rag, int *i_color);


int rag_getColorTabPixels(pixel **colors, /* result */
			  rag_t *rag
                          );

region_info_t *   /* returns previous regions, if stored by reference */ 
rag_setRegions(rag_t *rag, 
	       region_info_t *region, /* new region mem */
	       int max_regions,       /* if (-1) set reference
				       * if (-2) copy mem
				       * if (>0) keep mem */
	       int n_regions
	       );

contour_point_t * /* returns previous points, if stored by reference */ 
rag_setContourPoints(rag_t *rag, 
		     contour_point_t *points, /* new point mem */
		     int max_points,          /* if (-1) set reference
					       * if (-2) copy mem
					       * if (>0) keep mem */
		     int n_points
		     );

contour_point_t * /* returns previous points, if stored by reference */ 
rag_reallocContourPoints(rag_t *rag, int n_points);

region_adjacency_t * /* returns previous adjacencies, if stored by reference */
rag_setAdjacencyList(rag_t *rag, 
		     region_adjacency_t *adj, /* new adjacency mem */
		     int max_adj,             /* if (-1) set reference
					       * if (-2) copy mem
					       * if (>0) keep mem */
		     int n_adj
		     );

region_adjacency_t **     /* return cleared adjacency list */
rag_clearUnusedAdjacencies(rag_t *rag, 
			   region_adjacency_t **adjacencies,
			   int n_adjacencies);

  /* get-functions for regions */
region_info_t *rag_getRegionByLabel(rag_t *rag, int label,
				    int *index);
region_info_t *rag_getRegionByIndex(rag_t *rag, int index);
region_info_t *rag_getFirstRegion(rag_t *rag, int *index);
region_info_t *rag_getLastRegion(rag_t *rag, int *index);
region_info_t *rag_getNextRegion(rag_t *rag, int *index);
region_info_t *rag_getPrevRegion(rag_t *rag, int *index);
region_info_t **rag_getRegionArray(region_info_t **regions,
				   int *max_regions, int *n_regions,
				   rag_t *rag);

#define RAG_N_REGIONS(rag) ((rag)->n_region_index-1)
#define RAG_FORALL_REGIONS(rag,i) \
  if (rag) for (i=1; i < (rag)->n_region_index; i++)
#define RAG_GET_REGION(rag,i) \
  (&(rag)->region[(rag)->region_index[i]])

  /* get-functions for adjacencies */
region_adjacency_t *rag_getAdjacencyByLabel(rag_t *rag, int label,
					    int *index);
region_adjacency_t *rag_getAdjacencyByIndex(rag_t *rag, int index);
region_adjacency_t *rag_getFirstAdjacency(rag_t *rag, int *index);
region_adjacency_t *rag_getLastAdjacency(rag_t *rag, int *index);
region_adjacency_t *rag_getNextAdjacency(rag_t *rag, int *index);
region_adjacency_t *rag_getPrevAdjacency(rag_t *rag, int *index);
region_adjacency_t **rag_getAdjacencyArray(region_adjacency_t **adj,
					   int *max_adj, int *n_adj,
					   rag_t *rag);

#define RAG_N_ADJACENCIES(rag) ((rag)->n_adjacency_index)
#define RAG_FORALL_ADJACENCIES(rag,i) \
  if (rag) for (i=0; i < RAG_N_ADJACENCIES(rag); i++)
#define RAG_GET_ADJACENCY(rag,i) \
  (&rag->adjacency_list[rag->adjacency_index[i]])

  /* set/getnew/del-functions for regions */
region_info_t *rag_getRegion4Label(rag_t *rag, int label, int *index);
region_info_t *rag_newRegion(rag_t *rag, int *index);
region_info_t *rag_addRegion(rag_t *rag, region_info_t *region,
			     int *index);
rag_t *rag_delRegionByLabel(rag_t *rag, int label);
rag_t *rag_delRegionByIndex(rag_t *rag, int index);
rag_t *rag_delLastRegion(rag_t *rag);

rag_t *rag_blockRegionMapLabels(rag_t *rag);

region_info_t **
rag_reallocTmpRegions(rag_t *rag, int tmp_index, int n);

region_info_t **
rag_getRegions(region_info_t **regions, int *max_regions, rag_t *rag);

region_info_t **
rag_getTmpRegions(rag_t *rag, int tmp_index);

region_info_t **
rag_newTmpRegions(rag_t *rag, int tmp_index, 
		  int min_label, int max_label);


  /* set/getnew/del-functions for adjacencies */
region_adjacency_t *rag_getAdjacency4Label(rag_t *rag, int label,
					   int *index);
region_adjacency_t *rag_newAdjacency(rag_t *rag, int *index);
region_adjacency_t *rag_addAdjacency(rag_t *rag, 
				     region_adjacency_t *adjacency,
				     int *index);

region_adjacency_t **
rag_reallocTmpAdjacencies(rag_t *rag, int tmp_index, int n);

region_adjacency_t **
rag_getAdjacencies(region_adjacency_t **adjs,
		   int *max_adjs,
		   rag_t *rag, 
		   int label
		   );

region_adjacency_t **
rag_getTmpRevAdjacencies(rag_t *rag, 
			 int tmp_index,
			 int label);

region_adjacency_t **
rag_getTmpAdjacencies(rag_t *rag, 
		      int tmp_index,
		      int label);

region_adjacency_t **
rag_getRevAdjacencies(region_adjacency_t **rev_adjs,
		      int *max_adjs, 
		      rag_t *rag, int label);

region_adjacency_t **
rag_newTmpAdjacencies(rag_t *rag, 
		      int tmp_index,
		      int label);
region_adjacency_t **
rag_newTmpRevAdjacencies(rag_t *rag, 
			 int tmp_index,
			 int label);


#define RAG_GET_SUBLABELS(rag,i) \
  ((rag)->region[(rag)->region_index[i]].subLabels)
#define RAG_N_SUBLABELS(rag,i) \
  ((rag)->region[(rag)->region_index[i]].n_subLabels)

int *rag_getSubLabelsByIndex(rag_t *rag, int index, int *n_subLabels);
int *rag_getSubLabelsByLabel(rag_t *rag, int label, int *n_subLabels);
int *rag_setSubLabelByLabel(rag_t *rag, int label);
int *rag_setSubLabelByIndex(rag_t *rag, int index);
int *rag_clearSubLabelsByLabel(rag_t *rag, int label);
int *rag_clearSubLabelsByIndex(rag_t *rag, int index);
int *rag_addSubLabelsByLabel(rag_t *rag, int label, 
			     int *subLabels, int n_subLabels);
int *rag_addSubLabelsByIndex(rag_t *rag, int index,
			     int *subLabels, int n_subLabels);
int *rag_addSubLabelByLabel(rag_t *rag, int label, int subLabel);
int *rag_addSubLabelByIndex(rag_t *rag, int index, int subLabel);

int rag_consistent(rag_t *rag, const char *s);
  
#ifdef __cplusplus
}
#endif

#endif
