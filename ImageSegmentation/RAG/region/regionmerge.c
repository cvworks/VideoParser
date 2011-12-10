/*
 * regionmerge.c
 *
 * contains functions for merging regions
 *
 * Sven Wachsmuth, 5.2.2003, only planar graphs
 *
 * Sven Wachsmuth, 20.3.2003, un-restricted graphs
 */
#include <stdio.h>
#include <stdlib.h>
#include "regionmerge.h"

#define MAXLOOP    50000

int _region_contour_mergeSegment(contour_point_t *merged_contour, 
				 int *n_merged_contour,
				 region_adjacency_t *adj1,
				 int start1, int stop1, int *end1);

void 
_region_contour_mergeAdjacencies(region_adjacency_t *merged_adjacencies,
				 region_adjacency_t *merged_rev_adjacencies,
				 region_adjacency_t *adj1,
				 region_adjacency_t *adjacencies1,
				 int                n_adjacencies,
				 int start1, int end1,
				 int n_merged_contour);

void 
_region_contour_mergeAdjacenciesPt(region_adjacency_t**merged_adjacencies,
				   region_adjacency_t**merged_rev_adjacencies,
				   region_adjacency_t *adj1,
				   region_adjacency_t **adjacencies1,
				   int                n_adjacencies,
				   int start1, int end1,
				   int n_merged_contour);

void 
_region_contour_mergeEndAdjacencyPt(region_adjacency_t**merged_adjacencies,
				    region_adjacency_t**merged_rev_adjacencies,
				    region_adjacency_t *adj1,
				    region_adjacency_t **adjacencies1,
				    int                n_adjacencies,
				    int n_merged_contour);

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
			    int                *n_contour_points)
{
  region_info_t *info1, *info2;
  int order;

  if (!merge_adjacency1 && !merge_adjacency2) return (merged_region);
  
  if (merge_adjacency1) {
    info1 = merge_adjacency1->region1;
    info2 = merge_adjacency1->region2;
  } else {
    info1 = merge_adjacency2->region2;
    info2 = merge_adjacency2->region1;
  }    

  order = region_merge_order(info1, info2);
  if (order == 0) return (merged_region);

  if (order > 0) {
    region_info_t *_info;
    region_adjacency_t *_adj;
    _info = info1; info1 = info2; info2 = _info;
    _adj = merge_adjacency1; merge_adjacency1 = merge_adjacency2;
    merge_adjacency2 = _adj;
    _adj = adjacencies1; adjacencies1 = adjacencies2; adjacencies2 = _adj;
  }
  
  if (!info2) return (merged_region);

  if (!merged_region)
    merged_region = region_info_create();
  
  merged_region->pixelcount = info1->pixelcount + info2->pixelcount;
  merged_region->sum_x      = info1->sum_x + info2->sum_x;
  merged_region->sum_y      = info1->sum_y + info2->sum_y;
  merged_region->start_x    = info1->start_x;
  merged_region->start_y    = info1->start_y;
  merged_region->started    = 1;
  merged_region->color      = color;
  merged_region->label      = label;

  { /** alloc memory for contour points */
    
    int n_contour = info1->n_contour + info2->n_contour;
    if (!contour_points) {
      if (merged_region->max_contour < n_contour) {
	if (merged_region->max_contour > 0)
	  contour_points = (contour_point_t *)
	    realloc(merged_region->contour,
		    n_contour * sizeof(contour_point_t));
	else
	  contour_points = (contour_point_t *)
	    malloc(n_contour * sizeof(contour_point_t));
	merged_region->contour = contour_points;
      } else
	contour_points = merged_region->contour;
    } else {
      if (merged_region->max_contour > 0) free(merged_region->contour);
      merged_region->contour = contour_points + (*n_contour_points);
    }
  }
  merged_region->n_contour = 0;
  region_contour_merge(merged_region->contour, &merged_region->n_contour,
		       merged_adjacencies,
		       merged_rev_adjacencies,		       
		       merge_adjacency1,
		       adjacencies1,
		       merge_adjacency2,
		       adjacencies2,
		       n_adjacencies);

  return (merged_region);
}

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
			      int                *n_contour_points)
{
  region_info_t *info1, *info2;
  int order;

  if (!merge_adjacency1 && !merge_adjacency2) return (merged_region);
  
  if (merge_adjacency1) {
    info1 = merge_adjacency1->region1;
    info2 = merge_adjacency1->region2;
  } else {
    info1 = merge_adjacency2->region2;
    info2 = merge_adjacency2->region1;
  }    
  order = region_merge_order(info1, info2);
  if (order == 0) return (merged_region);

  if (order > 0) {
    region_info_t *_info;
    region_adjacency_t *_adj, **_adjs;
    _info = info1; info1 = info2; info2 = _info;
    _adj = merge_adjacency1; merge_adjacency1 = merge_adjacency2;
    merge_adjacency2 = _adj;
    _adjs = adjacencies1; adjacencies1 = adjacencies2; adjacencies2 = _adjs;
  }
  
  if (!info2) return (merged_region);

  if (!merged_region)
    merged_region = region_info_create();
  
  merged_region->pixelcount = info1->pixelcount + info2->pixelcount;
  merged_region->sum_x      = info1->sum_x + info2->sum_x;
  merged_region->sum_y      = info1->sum_y + info2->sum_y;
  merged_region->start_x    = info1->start_x;
  merged_region->start_y    = info1->start_y;
  merged_region->started    = 1;
  merged_region->color      = color;
  merged_region->label      = label;

  { /** alloc memory for contour points */
    
    int n_contour = info1->n_contour + info2->n_contour;
    if (!contour_points) {
      if (merged_region->max_contour < n_contour) {
	if (merged_region->max_contour > 0)
	  contour_points = (contour_point_t *)
	    realloc(merged_region->contour,
		    n_contour * sizeof(contour_point_t));
	else
	  contour_points = (contour_point_t *)
	    malloc(n_contour * sizeof(contour_point_t));
	merged_region->contour = contour_points;
	merged_region->max_contour = n_contour;
      } else
	contour_points = merged_region->contour;
    } else {
      if (merged_region->max_contour > 0) free(merged_region->contour);
      merged_region->contour = contour_points + (*n_contour_points);
      merged_region->max_contour = -1;
    }
  }
  merged_region->n_contour = 0;
  region_contour_mergePt(merged_region->contour, &merged_region->n_contour,
			 merged_adjacencies,
			 merged_rev_adjacencies,		       
			 merge_adjacency1,
			 adjacencies1,
			 merge_adjacency2,
			 adjacencies2,
			 n_adjacencies);

  return (merged_region);
}

int region_merge_order(region_info_t *info1, region_info_t *info2)
{
  if (!info1 && !info2) return 0;
  if (!info1)           return 1;
  if (!info2)           return -1;

  if (info1->start_y < info2->start_y) return -1;
  if (info1->start_y > info2->start_y) return  1;

  if (info1->start_x < info2->start_x) return -1;
  if (info1->start_x > info2->start_x) return  1;

  return 0;
}

int _region_contour_mergeSegment(contour_point_t *merged_contour,
				 int *n_merged_contour,
				 region_adjacency_t *adj1,
				 int start1, int stop1, int *end1)
     /*
      * adds next contour segment to merged contour
      */
{
  int i, start2, min_i, min, n;
  /** find first common segment in adj1 beginning behind start1 point 
      and ending before stop1 */
  n = adj1->region1->n_contour;
  min_i = -1; min = (stop1 - start1 + n) % n; 

  /*fprintf(stderr,"start1:%d stop1:%d min:%d.\n",start1,stop1,min);
   */

  for (i=0; i < adj1->n_sections; i++) {
    int start_diff = (adj1->sections[i].sec1.start - start1 + n) % n;
    int end_diff   = (adj1->sections[i].sec1.end   - start1 + n) % n;

    /*fprintf(stderr,"section [%d..%d] diff:%d.\n",
	    adj1->sections[i].sec1.start,adj1->sections[i].sec1.end,_diff);
    */

    /*if (start_diff > 0 && start_diff < min) {
      missing some sections that start on same pixel */
    if (start_diff >= 0 && end_diff > 0 && start_diff < min) {
      min_i = i; min = start_diff;
    }
  }
  i = min_i;

  /** define end index of current section and start index of next section */

  /* ... if no common segment was found ... */
  if (i < 0) {
    /* ... section to add is whole remaining contour ... */
    (*end1) = adj1->region1->n_contour - 1;
    /* ... no next section */
    start2 = -1;

    /*fprintf(stderr, "no section found, adding %d to %d\n", start1, *end1);*/

  } else {
    /* ... section to add stops at start of common segment ... */
    (*end1) = adj1->sections[i].sec1.start;
    /* ... next section starts on end pixel of common segment partner ... */
    start2 = adj1->sections[i].sec2.end;
    /* ... connection between this and next section must be checked and
     *     possibly adapted for 8-neighborhood property of contour */
    /*region_merge_adjustContour(end1, &start2, adj1->region1, adj1->region2);*/

    /*fprintf(stderr, "section found, adding %d to %d\n", start1, *end1);*/
  }

  /** copy next section to merged contour */

  { /** copy [start1..end1] to contour */
    contour_point_t *adj_contour1 = adj1->region1->contour;
    if (start1 <= (*end1)) {
      int n = (*end1) - start1 + 1;
      memcpy(merged_contour+(*n_merged_contour),
	     adj_contour1+start1, n*sizeof(contour_point_t));
      (*n_merged_contour)+=n;
    } else {
      int m = adj1->region1->n_contour;
      int n = m - start1;
      memcpy(merged_contour+(*n_merged_contour),
	     adj_contour1+start1, n*sizeof(contour_point_t));
      (*n_merged_contour)+=n;
      n = (*end1) + 1;
      memcpy(merged_contour+(*n_merged_contour),
	     adj_contour1, n*sizeof(contour_point_t));
      (*n_merged_contour)+=n;
    }
  }
  return (start2);
}

void 
_region_contour_mergeAdjacencies(region_adjacency_t *merged_adjacencies,
				 region_adjacency_t *merged_rev_adjacencies,
				 region_adjacency_t *adj1,
				 region_adjacency_t *adjacencies1,
				 int                n_adjacencies,
				 int start1, int end1,
				 int n_merged_contour)
{
  int m,n1,n0,j;

  /** copy all adjacencies of region1 in section [start1..end1] to
      the merged adjacencies */
  
  if (!merged_adjacencies) return;

  /* normalize [start1..end1] to [0..n1] */
  m  = adj1->region1->n_contour;
  n1 = (end1 - start1 + m) % m;

  /* define start of added section in merged contour */
  n0 = n_merged_contour - (n1 + 1);

  /* forall adjacencies of region1 ... */
  for (j = 0; j < n_adjacencies; j++) {
    region_adjacency_t *adj = &adjacencies1[j];
    int k;
    if (!adj) continue;
    for (k=0; k < adj->n_sections; k++) {
      contour_sectionPair_t *secPair = &adj->sections[k];
      
      /* ... normalize section to start index start1 */
      int sec1_start = (secPair->sec1.start - start1 + m) % m;
      int sec1_end   = (secPair->sec1.end - start1 + m) % m;

      /** if section is in [start1..end1] range ... */
      if (0 <= sec1_start && sec1_end <= n1) {
	
	/* ... adjust start and end index to merged contour ... */
	int start_merge = n0 + sec1_start;
	int end_merge = n0 + sec1_end;

	/* ... add section to merged adjacency ... */
	int isec = region_adjacency_startSection(&merged_adjacencies[j],
						 start_merge,
						 secPair->sec1.checked_start_point.x,
						 secPair->sec1.checked_start_point.y);
	region_adjacency_endSection(&merged_adjacencies[j],
				    end_merge,
				    secPair->sec1.checked_end_point.x,
				    secPair->sec1.checked_end_point.y);
	
	/* ... copy start and end index from secPair */
	merged_adjacencies[j].sections[isec].sec2 = secPair->sec2;
	
	/*
	fprintf(stderr, "Section added: %d-%d [%d..%d,%d..%d]\n",
		merged_adjacencies[j].region1->label,
		merged_adjacencies[j].region2->label,
		merged_adjacencies[j].sections[isec].sec1.start,
		merged_adjacencies[j].sections[isec].sec1.end,
		merged_adjacencies[j].sections[isec].sec2.start,
		merged_adjacencies[j].sections[isec].sec2.end);
	*/

	/* ... set reverse adjacency */
	if (secPair->sec2.start >= 0)
	  /* ... this function copies the newly generated section with
	   * exchange of sec1 and sec2. This may be not the exact
	   * solution (may vary by one contour-pixel).
	   * The exact solution would have to check rev_adjacencies1 and
	   * find the matching section in there. 
	   */
	  region_adjacency_revSection(&merged_rev_adjacencies[j],
				      &merged_adjacencies[j].sections[isec]);
	
      }
    }
  }
}

void 
_region_contour_mergeAdjacenciesPt(region_adjacency_t **merged_adjacencies,
				   region_adjacency_t **merged_rev_adjacencies,
				   region_adjacency_t *adj1,
				   region_adjacency_t **adjacencies1,
				   int                n_adjacencies,
				   int start1, int end1,
				   int n_merged_contour)
{
  int m,n1,n0,j;

  /** copy all adjacencies of region1 in section [start1..end1] to
      the merged adjacencies */
  
  if (!merged_adjacencies) return;

  /* normalize [start1..end1] to [0..n1] */
  m  = adj1->region1->n_contour;
  n1 = (end1 - start1 + m) % m;

  /* define start of added section in merged contour */
  n0 = n_merged_contour - (n1 + 1);

  /* forall adjacencies of region1 ... */
  for (j = 0; j < n_adjacencies; j++) {
    region_adjacency_t *adj = adjacencies1[j];
    int k;
    if (!adj || adj == adj1) continue;
    for (k=0; k < adj->n_sections; k++) {
      contour_sectionPair_t *secPair = &adj->sections[k];
      
      /* ... normalize section to start index start1 */
      int sec1_start = (secPair->sec1.start - start1 + m) % m;
      int sec1_end   = (secPair->sec1.end - start1 + m) % m;

      /** if section is in [start1..end1] range ... */
      if (0 <= sec1_start && sec1_start <= n1
	  && 0 <= sec1_end && sec1_end  <= n1) {
	
	/* ... adjust start and end index to merged contour ... */
	int start_merge = n0 + sec1_start;
	int end_merge = n0 + sec1_end;

	/* ... add section to merged adjacency ... */
	int isec = region_adjacency_startSection(merged_adjacencies[j],
						 start_merge,
						 secPair->sec1.checked_start_point.x,
						 secPair->sec1.checked_start_point.y);
	region_adjacency_endSection(merged_adjacencies[j],
				    end_merge,
				    secPair->sec1.checked_end_point.x,
				    secPair->sec1.checked_end_point.y);
	
	/* ... copy start and end index from secPair */
	merged_adjacencies[j]->sections[isec].sec2 = secPair->sec2;
	
	/*
	fprintf(stderr, "Section added: %d-%d [%d..%d,%d..%d]\n",
		merged_adjacencies[j]->region1->label,
		merged_adjacencies[j]->region2->label,
		merged_adjacencies[j]->sections[isec].sec1.start,
		merged_adjacencies[j]->sections[isec].sec1.end,
		merged_adjacencies[j]->sections[isec].sec2.start,
		merged_adjacencies[j]->sections[isec].sec2.end);
	*/
	/* ... set reverse adjacency */
	if (secPair->sec2.start >= 0)
	  /* ... this function copies the newly generated section with
	   * exchange of sec1 and sec2. This may be not the exact
	   * solution (may vary by one contour-pixel).
	   * The exact solution would have to check rev_adjacencies1 and
	   * find the matching section in there. 
	   */
	  region_adjacency_revSection(merged_rev_adjacencies[j],
				      &merged_adjacencies[j]->sections[isec]);
	
      }
    }
  }
}

void 
_region_contour_mergeEndAdjacencyPt(region_adjacency_t**merged_adjacencies,
				    region_adjacency_t**merged_rev_adjacencies,
				    region_adjacency_t *adj1,
				    region_adjacency_t **adjacencies1,
				    int                n_adjacencies,
				    int n_merged_contour)
{
  int j;
  /* forall adjacencies of region1 ... */
  for (j = 0; j < n_adjacencies; j++) {
    region_adjacency_t *adj = adjacencies1[j];
    int k;
    if (!adj || adj == adj1) continue;
    for (k=0; k < adj->n_sections; k++) {
      contour_sectionPair_t *secPair = &adj->sections[k];
      
      int sec1_start = secPair->sec1.start;
      int sec1_end   = secPair->sec1.end;
      
      /*fprintf(stderr, "EndAdjacency[%d]: [%d..%d]\n", j,sec1_start, sec1_end);
       */

      /** if section includes end and start point ... */
      if (sec1_start > sec1_end) {
	
	/* ... adjust start and end index to merged contour ... */
	int start_merge = (sec1_start - adj->region1->n_contour
			   + n_merged_contour);
	int end_merge = sec1_end;

	/* ... add section to merged adjacency ... */
	int isec = region_adjacency_startSection(merged_adjacencies[j],
						 start_merge,
						 secPair->sec1.checked_start_point.x,
						 secPair->sec1.checked_start_point.y);
	region_adjacency_endSection(merged_adjacencies[j],
				    end_merge,
				    secPair->sec1.checked_end_point.x,
				    secPair->sec1.checked_end_point.y);
	
	/* ... copy start and end index from secPair */
	merged_adjacencies[j]->sections[isec].sec2 = secPair->sec2;
	
	/*
	fprintf(stderr, "Section added: %d-%d [%d..%d,%d..%d]\n",
		merged_adjacencies[j]->region1->label,
		merged_adjacencies[j]->region2->label,
		merged_adjacencies[j]->sections[isec].sec1.start,
		merged_adjacencies[j]->sections[isec].sec1.end,
		merged_adjacencies[j]->sections[isec].sec2.start,
		merged_adjacencies[j]->sections[isec].sec2.end);
	*/
	/* ... set reverse adjacency */
	if (secPair->sec2.start >= 0)
	  /* ... this function copies the newly generated section with
	   * exchange of sec1 and sec2. This may be not the exact
	   * solution (may vary by one contour-pixel).
	   * The exact solution would have to check rev_adjacencies1 and
	   * find the matching section in there. 
	   */
	  region_adjacency_revSection(merged_rev_adjacencies[j],
				      &merged_adjacencies[j]->sections[isec]);
	
	return;
      }
    }
  }  
}

int region_contour_merge(contour_point_t *contour, int *n_contour,
			 region_adjacency_t *merged_adjacencies,
			 region_adjacency_t *merged_rev_adjacencies,
			 region_adjacency_t *adj1,
			 region_adjacency_t *adjacencies1,
			 region_adjacency_t *adj2,
			 region_adjacency_t *adjacencies2,
			 int                n_adjacencies)
{
  int i,swap, end1, start1;
  int n = adj1->region1->n_contour;

  for (i=0,swap=0; i >= 0; swap = 1 - swap) {
    start1 = i;
    if (swap) {
      i = _region_contour_mergeSegment(contour, n_contour, 
				       adj2, start1, i-1, &end1);
      _region_contour_mergeAdjacencies(merged_adjacencies,
				       merged_rev_adjacencies,
				       adj2, adjacencies2,
				       n_adjacencies, start1, end1,
				       *n_contour);
    } else {
      i = _region_contour_mergeSegment(contour, n_contour, 
				       adj1, start1, n-1, &end1);
      _region_contour_mergeAdjacencies(merged_adjacencies,
				       merged_rev_adjacencies,
				       adj1, adjacencies1,
				       n_adjacencies, start1, end1,
				       *n_contour);
    }
  }
  /*_region_contour_mergeEndAdjacency(merged_adjacencies,
   *			      merged_rev_adjacencies,
   *			      adj1, adjacencies1,
   *			      n_adjacencies,
   *			      *n_contour);
   */
  return 0;
}  

int region_contour_mergePt(contour_point_t *contour, int *n_contour,
			   region_adjacency_t **merged_adjacencies,
			   region_adjacency_t **merged_rev_adjacencies,
			   region_adjacency_t *adj1,
			   region_adjacency_t **adjacencies1,
			   region_adjacency_t *adj2,
			   region_adjacency_t **adjacencies2,
			   int                n_adjacencies)
{
  int i,swap, end1, start1;
  int n = (adj1) ? adj1->region1->n_contour : adj2->region2->n_contour;

  if (!adj1) {
    /** if no edge from region1 to region2 (region1 includes region2),
	copy all information from region1 to the merged region */

    memcpy(contour, adj2->region2->contour, n * sizeof(contour_point_t));
    (*n_contour) = n;

    for (i=0; i < n_adjacencies; i++) {
      region_info_t *reg;
      int j;
      if (!adjacencies1[i] || adjacencies1[i]->n_sections<=0) continue;
      
      reg = merged_adjacencies[i]->region2;
      region_adjacency_cpy(merged_adjacencies[i],adjacencies1[i]);
      merged_adjacencies[i]->region2 = reg;

      for (j=0; j < merged_adjacencies[i]->n_sections; j++) {
	contour_sectionPair_t *secPair = &merged_adjacencies[i]->sections[j];

	/* ... set reverse adjacency */
	if (secPair->sec2.start >= 0)
	  /* ... this function copies the newly generated section with
	   * exchange of sec1 and sec2. This may be not the exact
	   * solution (may vary by one contour-pixel).
	   * The exact solution would have to check rev_adjacencies1 and
	   * find the matching section in there. 
	   */
	  region_adjacency_revSection(merged_rev_adjacencies[i], secPair);
      }
    }
    return (0);
  }
      
  for (i=0,swap=0; i >= 0; swap = 1 - swap) {
    start1 = i;
    if (swap) {
      i = _region_contour_mergeSegment(contour, n_contour, 
				       adj2, start1, i-1, &end1);
      _region_contour_mergeAdjacenciesPt(merged_adjacencies,
					 merged_rev_adjacencies,
					 adj2, adjacencies2,
					 n_adjacencies, start1, end1,
					 *n_contour);
    } else {
      i = _region_contour_mergeSegment(contour, n_contour, 
				       adj1, start1, n-1, &end1);
      _region_contour_mergeAdjacenciesPt(merged_adjacencies,
					 merged_rev_adjacencies,
					 adj1, adjacencies1,
					 n_adjacencies, start1, end1,
					 *n_contour);
    }
  }
  _region_contour_mergeEndAdjacencyPt(merged_adjacencies,
				      merged_rev_adjacencies,
				      adj1, adjacencies1,
				      n_adjacencies,
				      *n_contour);
  return 0;
}  

int region_pixel_connected(contour_point_t *p1, contour_point_t *p2)
{
  int dx,dy;
  dx = (p1->x > p2->x) ? p1->x - p2->x : p2->x - p1->x;
  dy = (p1->y > p2->y) ? p1->y - p2->y : p2->y - p1->y;
  
  return (dx <= 1 && dy <= 1);
}

void region_merge_adjustContour(int *end1, int *start2,
				region_info_t *region1,
				region_info_t *region2)
{ /* check if the previous contour-pixel in region1 is still connected to
   * to the pixel of region2 and the same about the next region2 pixel */
  int j,k;
  if (!region1 || !region2) return;

  j = (*end1 > 0) ? *end1-1 : region1->n_contour;
  k = (*start2 < region2->n_contour-1) ? *start2+1 : 0;
  
  if (region_pixel_connected(&region1->contour[j],
			     &region2->contour[*start2]))
    *end1 = j;
  if (region_pixel_connected(&region1->contour[*end1],
			     &region2->contour[k]))
    *start2 = k;
}

/****** new merge op based on regionMap *********************/

int _region_mcontour(contour_point_t *contour,
		     int *regionMap, int xlen, int ylen,
		     int start_x, int start_y,
		     int max_subLabel, int *subLabel_flags)
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
    int label = regionMap[(act_x + xoffset[direction][0]) +
			 (act_y + yoffset[direction][0]) * xlen];
    if (label >= 0 && label <= max_subLabel && subLabel_flags[label]) {
      
      /* put point into contour and remember this point */
      act_x += xoffset[direction][0];
      act_y += yoffset[direction][0];
      
      point++; n_points++;
      point->x = act_x;
      point->y = act_y;
      
      /* new search direction */
      direction = change_direction[direction][0];
      searchCount = 0;
    } else {
      /* Check next image pixel ... */
      label = regionMap[(act_x + xoffset[direction][1]) +
		       (act_y + yoffset[direction][1]) * xlen];
      if (label >= 0 && label <= max_subLabel && subLabel_flags[label]) {
      
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
      else {
	direction = change_direction[direction][1];
	searchCount ++;
      }
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

void _region_contour_merge2(region_info_t *region_info, int *regionMap,
			    int xlen, int ylen, contour_point_t *contour)
{
  /*
   * Assumes a regionMap with all marginal pixels being set to an 
   * undefine label that corresponds to any other label.
   * This prevents the algorithm from testing special marginal conditions.
   * Furthermore, assumes that all sublabels are stored in region_info
   * and the starting point is already defined as well as the contour-
   * points are allocated (maximum is sum of sublabel-contours)
   */
  int n_points;
  int i,max_subLabel = -1;
  int *subLabel_flags=NULL;

  if (!region_info || region_info->pixelcount == 0)
    return;

  for (i=0; i < region_info->n_subLabels; i++) {
    int _subLabel = region_info->subLabels[i];
    if (_subLabel > max_subLabel) {
      max_subLabel = _subLabel;
    }
  }
  subLabel_flags = (int *) malloc((max_subLabel+1) * sizeof(int));
  memset(subLabel_flags, 0, (max_subLabel+1) * sizeof(int));
  for (i=0; i < region_info->n_subLabels; i++) {
    subLabel_flags[region_info->subLabels[i]] = 1;
  }

  /* start contour finding at region starting point */
  n_points =
    _region_mcontour(contour, regionMap, xlen, ylen, region_info->start_x,
		     region_info->start_y, max_subLabel, subLabel_flags);

  if (n_points > region_info->max_contour) {
    fprintf(stderr,"MEMORY OVERFLOW!!!!\n");
    free(0x0);
  }

  /* reallocate memory space for contour pixels */
  if (region_info->max_contour > 0) {
    contour = (contour_point_t *)
      realloc(contour, n_points * sizeof(contour_point_t));
    region_info->max_contour = n_points;
  }
  region_info->contour = contour;
  region_info->n_contour = n_points;

  if (subLabel_flags) free(subLabel_flags);
}

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
				     int                cols, int rows)
{
  region_info_t *info1, *info2;
  int i,j,order;

  if (!merge_adjacency1 && !merge_adjacency2) return (merged_region);
  
  if (merge_adjacency1) {
    info1 = merge_adjacency1->region1;
    info2 = merge_adjacency1->region2;
  } else {
    info1 = merge_adjacency2->region2;
    info2 = merge_adjacency2->region1;
  }    
  order = region_merge_order(info1, info2);
  if (order == 0) return (merged_region);

  if (order > 0) {
    region_info_t *_info;
    region_adjacency_t *_adj, **_adjs;
    _info = info1; info1 = info2; info2 = _info;
    _adj = merge_adjacency1; merge_adjacency1 = merge_adjacency2;
    merge_adjacency2 = _adj;
  }
  
  if (!info2) return (merged_region);

  if (!merged_region)
    merged_region = region_info_create();
  
  merged_region->pixelcount = info1->pixelcount + info2->pixelcount;
  merged_region->sum_x      = info1->sum_x + info2->sum_x;
  merged_region->sum_y      = info1->sum_y + info2->sum_y;
  merged_region->start_x    = info1->start_x;
  merged_region->start_y    = info1->start_y;
  merged_region->started    = 1;
  merged_region->color      = color;
  merged_region->label      = label;
  
  /** set subLabels */
  region_setSubLabel(merged_region);
  region_addSubLabels(merged_region, info1->subLabels, info1->n_subLabels);
  region_addSubLabels(merged_region, info2->subLabels, info2->n_subLabels);
    
  { /** alloc memory for contour points */
    
    int n_contour = info1->n_contour + info2->n_contour + 3;
    // 3 extra points: 
    // 2 for additionally needed pixels if there is
    //   a one-pixel connection between regions
    // 1 for the temporarily added last point which is the same as the
    //   starting points

    if (!contour_points) {
      if (merged_region->max_contour < n_contour) {
	if (merged_region->max_contour > 0)
	  contour_points = (contour_point_t *)
	    realloc(merged_region->contour,
		    n_contour * sizeof(contour_point_t));
	else
	  contour_points = (contour_point_t *)
	    malloc(n_contour * sizeof(contour_point_t));
	merged_region->contour = contour_points;
	merged_region->max_contour = n_contour;
      } else
	contour_points = merged_region->contour;
    } else {
      if (merged_region->max_contour > 0) free(merged_region->contour);
      merged_region->contour = contour_points + (*n_contour_points);
      merged_region->max_contour = -1;
    }
  }
  merged_region->n_contour = 0;

  _region_contour_merge2(merged_region, regionMap, cols, rows, contour_points);
  region_adjacency_set(merged_adjacencies, regions, label, n_regions,
		       regionMap, cols);
  for (i=1; i < n_regions; i++)
    region_adjacency_update(merged_adjacencies[i], indexMap, cols);
  
  /** adjacencies to other merged regions are only copied and not 
      adjusted to the new contour indices of the merged contour **/
  for (i=1; i < n_regions; i++) {
    int j;
    if (regions[i]->n_subLabels <= 1
	|| !region_areDisjunct(merged_region, regions[i]))
      continue;

    /** if a disjunct merged region has an adjacent sublabel, ... */
    for (j=0; j < regions[i]->n_subLabels; j++)
      if (region_isAdjacent(merged_adjacencies[regions[i]->subLabels[j]])
	  && regions[regions[i]->subLabels[j]]->n_subLabels <= 1) {
	
	/** then, add sections of sublabel ... */
	region_adjacency_t *adj = merged_adjacencies[regions[i]->subLabels[j]];
	int k;
	for (k=0; k < adj->n_sections; k++) {
	  region_adjacency_cpySection(merged_adjacencies[regions[i]->label],
				      &adj->sections[k]);
	}
      }
  }
  return (merged_region);
}

int region_mergeInEdges2(region_adjacency_t **rev_adjacenciesM,
			 region_info_t **regions, int n_regions,
			 int merged_label, int label1, int label2,
			 region_adjacency_t **rev_adjacencies1)
{
  int i,j;
  
  /** all regions that are adjacent to the regions that are merged
      (except those that are not disjunct to the merge partner) 
      are adjacent to the merged one */

  region_info_t *region2 = regions[label2];

  for (i=0; i < n_regions; i++) {
    region_info_t *regionI;

    region_adjacency_t *adj = rev_adjacencies1[i];
    if (!region_isAdjacent(adj)) continue;

    regionI = regions[i];
    if (!region_areDisjunct(region2, regionI)) continue;

    /** ... add sections from i to label1 ... */
    for (j=0; j < adj->n_sections; j++) {
      region_adjacency_cpySection(rev_adjacenciesM[i], &adj->sections[j]);
    }
  }
  return (0);
}
 






