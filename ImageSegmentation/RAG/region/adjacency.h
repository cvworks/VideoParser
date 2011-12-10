/*
 * adjecency.h
 *
 * compute adjacency representation of region pair
 * (based on contour pixels)
 *
 * Sven Wachsmuth, 27.11.2002
 */
#ifndef _CONNECT_ADJACENCY_H
#define _CONNECT_ADJACENCY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "regioninfo.h"

typedef struct contour_section_t {
  int start;
  int end;
  int length;
  contour_point_t checked_start_point;
  contour_point_t checked_end_point;
} contour_section_t;

typedef struct contour_sectionPair_t {
  contour_section_t sec1;
  contour_section_t sec2;
} contour_sectionPair_t;

typedef struct region_adjacency_t {
  region_info_t *region1;
  region_info_t *region2;
  int            region1_label; /* used for reading */
  int            region2_label; /* used for reading */

  int label;   /* identifier for adjacency */
  
  int contour_length1;
  int contour_length2;

  int max_sections;
  int n_sections;
  contour_sectionPair_t *sections;
} region_adjacency_t;

contour_section_t *contour_section_init(contour_section_t *sec);
contour_section_t *contour_section_create(void);
void               contour_section_free(contour_section_t *sec);
void               contour_section_destroy(contour_section_t *sec);

contour_section_t *contour_section_dup(contour_section_t *sec);
contour_section_t *contour_section_cpy(contour_section_t *copy, 
				       contour_section_t *sec);

contour_sectionPair_t *contour_sectionPair_init(contour_sectionPair_t *sec);
contour_sectionPair_t *contour_sectionPair_create(void);
void               contour_sectionPair_free(contour_sectionPair_t *sec);
void               contour_sectionPair_destroy(contour_sectionPair_t *sec);

contour_sectionPair_t *contour_sectionPair_dup(contour_sectionPair_t *sec);
contour_sectionPair_t *contour_sectionPair_cpy(contour_sectionPair_t *copy, 
					       contour_sectionPair_t *sec);

contour_sectionPair_t *contour_sectionPair_update(contour_sectionPair_t *sec,
						  region_info_t *reg1,
						  region_info_t *reg2,
						  int *indexMap, int cols);

contour_sectionPair_t *contour_sectionPair_reverse(contour_sectionPair_t *rev,
						   contour_sectionPair_t *sec);

region_adjacency_t *region_adjacency_init(region_adjacency_t *adj);
region_adjacency_t *region_adjacency_create(void);
void               region_adjacency_free(region_adjacency_t *adj);
void               region_adjacency_destroy(region_adjacency_t *adj);

region_adjacency_t *region_adjacency_dup(region_adjacency_t *adj);
region_adjacency_t *region_adjacency_cpy(region_adjacency_t *copy, 
					 region_adjacency_t *adj);

int region_adjacency_startSection(region_adjacency_t *adj,
				  int index,
				  int checked_x,
				  int checked_y);
region_adjacency_t *region_adjacency_endSection(region_adjacency_t *adj,
						int index,
						int checked_x,
						int checked_y);
region_adjacency_t **region_adjacency_set(region_adjacency_t **adj,
					  region_info_t **reg,
					  int i_region, int n_regions,
					  int *regionMap, int cols);
region_adjacency_t *region_adjacency_setOLD(region_adjacency_t *adj,
					    region_info_t *reg,
					    int i_region, int n_regions,
					    int *regionMap, int cols);

region_adjacency_t *region_adjacency_update(region_adjacency_t *adj,
					    int *indexMap, int cols);

region_adjacency_t *region_adjacency_revSection(region_adjacency_t *adj,
						contour_sectionPair_t *sec);
region_adjacency_t *region_adjacency_cpySection(region_adjacency_t *adj,
						contour_sectionPair_t *sec);
region_adjacency_t *region_adjacency_cpySec1(region_adjacency_t *adj,
					     contour_sectionPair_t *sec);

int                 region_isAdjacent(region_adjacency_t *adj);
int                 region_isBiAdjacent(region_adjacency_t *adj);

region_adjacency_t *region_adjacency_setValues(region_adjacency_t *adj,
					       region_info_t **regions,
					       int max_regions);
region_adjacency_t *region_adjacency_setRValues(region_adjacency_t *adj,
						region_info_t *regions1,
						region_info_t *regions2);
region_adjacency_t *region_adjacency_setAValues(region_adjacency_t *adj);

char *region_adjacency_sprint(char *s, region_adjacency_t *adj);
int   region_adjacency_sscan(region_adjacency_t *adj, char *s);
int   region_adjacency_sscanR(region_adjacency_t *adj, char *s, 
			      region_info_t **regions, int max);

#ifdef __cplusplus
}
#endif

#endif


