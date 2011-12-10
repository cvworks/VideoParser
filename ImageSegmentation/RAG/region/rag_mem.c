/*
 * region/rag_mem.c
 *
 * Sven Wachsmuth, 08.04.2003
 *
 * maintenance of memory for regions, adjacencies, and merging
 *
 * Sven Wachsmuth, 28.07.2003 (error in rag_adjacencyLabel2Index corrected)
 *
 */
#include <stdlib.h>
#include <stdio.h>

#include "_mem.h"
#include "rag_mem.h"

#define RAG_ADDITEMS (16)

int *_rag_initIndex(int *index, int *max_index, int *n_index,
		    int start, int end)
{
  int i,j, n= (start < end) ? end - start : start - end;
  if (n_index) (*n_index) = n;

  if (n == 0) return (index);

  index = (int *) 
    _rag_realloc_memory_space(index, max_index, n,
			      sizeof(int), NULL, NULL, NULL);
  if (start < end)
    for (j=0,i=start; i < end; i++,j++) index[j] = i;
  else
    for (j=0,i=start; i > end; i--,j++) index[j] = i;
    
  return (index);
}

int _rag_label2index(int label, int min_label, int max_label,
			int *_index, int n_index)
{
  int i;
  if (!_index || label < min_label || label >= max_label) return (-1);

  /* the most probable index for label is index==label */
  for (i=label; i < n_index && _index[i] != label; i++);
  if (i >= n_index)
    for (i=label-1; i >= 0 && _index[i] != label; i--);

  return (i);
}

int _rag_index2label(int index, int *_index, int n_index)
{
  int n;
  if (!_index || index < 0 || index >= n_index) return (index);
  
  return (_index[index]);
}

int rag_regionLabel2Index(rag_t *rag, int label)
{
  int n,m;
  if (!rag) return (-1);

  n = ((rag->max_regions < 0) ? rag->n_regions : rag->max_regions);
  m = ((rag->max_region_index < 0) ? 
       rag->n_region_index : rag->max_region_index);
    
  return (_rag_label2index(label, 1, n, rag->region_index, m));
}

int rag_regionIndex2Label(rag_t *rag, int index)
{
  int n,m;
  if (!rag) return (-1);

  m = ((rag->max_region_index < 0) ? 
       rag->n_region_index : rag->max_region_index);

  return (_rag_index2label(index, rag->region_index, m));
}

int rag_adjacencyLabel2Index(rag_t *rag, int label)
{
  int n,m;
  if (!rag) return (-1);

  n = ((rag->max_adjacency_list < 0) ? 
       rag->n_adjacency_list : rag->max_adjacency_list);
  m = ((rag->max_adjacency_index < 0) ? 
       rag->n_adjacency_index : rag->max_adjacency_index);
    
  /*return (_rag_label2index(label, 1, n, rag->adjacency_index, m));*/
  /* CHANGE because of error in testVISTRAM */
  return (_rag_label2index(label, 0, n, rag->adjacency_index, m));
}

int rag_adjacencyIndex2Label(rag_t *rag, int index)
{
  int n,m;
  if (!rag) return (-1);

  m = ((rag->max_adjacency_index < 0) ? 
       rag->n_adjacency_index : rag->max_adjacency_index);

  return (_rag_index2label(index, rag->adjacency_index, m));
}


/* ----------------------------------------------------------------- */

rag_t *rag_init(rag_t *rag)
{
  if (!rag) 
    rag = (rag_t *) malloc(sizeof(rag_t));

  memset(rag, 0, sizeof(rag_t));
  rag->n_regions = 1;
  rag->n_region_index = 1;

  return (rag);
}

rag_t *rag_initN(rag_t *rag, 
		       int n,             /* number of regions */ 
		       int m              /* number of adjacencies */
		       )
{
  rag_init(rag);
  return (rag_realloc(rag,n,m));
}
  
rag_t *rag_create(void)
{
  return (rag_init(NULL));
}

rag_t *rag_createN(int n,           /* number of regions */  
		   int m            /* number of adjacencies */
		   )
{
  return (rag_realloc(NULL,n,m));
}

rag_t *rag_realloc(rag_t *rag, 
		   int n,           /* number of regions */ 
		   int m            /* number of adjacencies */
		   )
{
  int _n_regions;
  int _n_adjacencies;
  region_info_t *_region;

  if (!rag)
    rag = rag_create();

  _region    = rag->region;
  _n_regions = (rag->max_regions > 0) ? rag->max_regions : rag->n_regions;

  rag->region = (region_info_t *)
    _rag_realloc_memory_space(rag->region, &rag->max_regions,
			      n+1, sizeof(region_info_t),
			      (rag_copyFunc *) region_info_cpy,
			      (rag_initFunc *) region_info_init,
			      (rag_freeFunc *) NULL);
  rag->region_index = (int *)
    _rag_realloc_memory_space(rag->region_index, &rag->max_region_index,
			      n+1, sizeof(int), NULL, NULL, NULL);

  /** set new references to rag->region */
  _rag_initIndex(rag->region_index+_n_regions,
		 &rag->max_region_index, /* make sure that no reallocation
					  * occures */
		 NULL, _n_regions, rag->max_regions);

  if (_region != rag->region) {
    int i;
    RAG_FORALL_ADJACENCIES(rag,i) {
      region_adjacency_t *adj = RAG_GET_ADJACENCY(rag,i);
      if (!adj) continue;
      if (adj->region1) adj->region1 += rag->region - _region;
      if (adj->region2) adj->region2 += rag->region - _region;
    }
  }

  _n_adjacencies = ((rag->max_adjacency_list > 0) ? 
		    rag->max_adjacency_list : rag->n_adjacency_list);

  rag->adjacency_list = (region_adjacency_t *)
    _rag_realloc_memory_space(rag->adjacency_list, &rag->max_adjacency_list,
			      m, sizeof(region_adjacency_t),
			      (rag_copyFunc *) region_adjacency_cpy,
			      (rag_initFunc *) region_adjacency_init,
			      (rag_freeFunc *) NULL);
  rag->adjacency_index = (int *)
    _rag_realloc_memory_space(rag->adjacency_index, &rag->max_adjacency_index,
			      m, sizeof(int), NULL, NULL, NULL);

  /** set new references to rag->adjacency_list */
  _rag_initIndex(rag->adjacency_index+_n_adjacencies,
		 &rag->max_adjacency_index, /* make sure that no 
					     * reallocation occures */
		 NULL, _n_adjacencies, rag->max_adjacency_list);

  /*fprintf(stderr,"rag_realloc: nodes %d of %d ;  edges %d of %d\n",
	  rag->n_regions, rag->max_regions, rag->n_adjacency_list,
	  rag->max_adjacency_list);
  */
  return (rag);
}

void   rag_free(rag_t *rag)
{
  if (!rag) return;

  _rag_free_memory_space(rag->colorMap, &rag->max_colorMap, 
			 sizeof(unsigned char), NULL);
  _rag_free_memory_space(rag->regionMap, &rag->max_regionMap,
			 sizeof(int), NULL);
  _rag_free_memory_space(rag->contourIndexMap, &rag->max_contourIndexMap,
			 sizeof(int), NULL);
  _rag_free_memory_space(rag->colorTab, &rag->max_colorTab,
			 sizeof(rag_color_t), NULL);
  _rag_free_memory_space(rag->contour_points, &rag->max_contour_points,
			 sizeof(contour_point_t), NULL);
  _rag_free_memory_space(rag->region, &rag->max_regions,
			 sizeof(region_info_t),
			 (rag_freeFunc *) region_info_free);
  _rag_free_memory_space(rag->region_index, &rag->max_region_index,
			 sizeof(int), NULL);
  _rag_free_memory_space(rag->adjacency_list, &rag->max_adjacency_list,
			 sizeof(region_adjacency_t),
			 (rag_freeFunc *) region_adjacency_free);
  _rag_free_memory_space(rag->adjacency_index, &rag->max_adjacency_index,
			 sizeof(int), NULL);
  {
    int i;
    for (i=0; i < RAG_TMP_ADJLISTS; i++)
      _rag_free_memory_space(rag->tmp_adjacencies[i],
			     &rag->max_tmp_adjacencies[i],
			     sizeof(region_adjacency_t *), NULL);
    for (i=0; i < RAG_TMP_REGLISTS; i++)
      _rag_free_memory_space(rag->tmp_regions[i],
			     &rag->max_tmp_regions[i],
			     sizeof(region_info_t *), NULL);
  }
}

void   rag_destroy(rag_t *rag)
{
  rag_free(rag);
  free(rag);
}

rag_t *rag_cpy(rag_t *copy, const rag_t *rag)
{
  if (!rag) return (copy);
  if (!copy) 
    copy = rag_create();
  
  copy->image = rag->image;
  
  copy->colorMap = (unsigned char *)
    _rag_cpy_memory_space(copy->colorMap, &copy->max_colorMap, NULL, 
			  rag->colorMap, rag->max_colorMap,
			  rag->cols * rag->rows, sizeof(unsigned char),
			  NULL, NULL, NULL);
  copy->regionMap = (int *)
    _rag_cpy_memory_space(copy->regionMap,&copy->max_regionMap, NULL,
			  rag->regionMap,rag->max_regionMap,
			  rag->cols * rag->rows, sizeof(int),
			  NULL, NULL, NULL);
  copy->contourIndexMap = (int *)
    _rag_cpy_memory_space(copy->contourIndexMap, &copy->max_contourIndexMap,
			  NULL,
			  rag->contourIndexMap, rag->max_contourIndexMap,
			  rag->cols * rag->rows, sizeof(int),
			  NULL, NULL, NULL);
  copy->cols = rag->cols;
  copy->rows = rag->rows;

  copy->colorTab = (rag_color_t *)
    _rag_cpy_memory_space(copy->colorTab, &copy->max_colorTab, &copy->n_colors,
			  rag->colorTab, rag->max_colorTab,
			  rag->n_colors, sizeof(rag_color_t),
			  NULL, NULL, NULL);

  copy->region = (region_info_t *)
    _rag_cpy_memory_space(copy->region, &copy->max_regions, &copy->n_regions,
			  rag->region, rag->max_regions, rag->n_regions,
			  sizeof(region_info_t),
			  (rag_copyFunc *) region_info_cpy,
			  (rag_initFunc *) region_info_init,
			  (rag_freeFunc *) region_info_free);
  
  copy->region_index = (int *)
    _rag_cpy_memory_space(copy->region_index, &copy->max_region_index, 
			  &copy->n_region_index,
			  rag->region, rag->max_region_index, 
			  rag->n_region_index,
			  sizeof(int), NULL, NULL, NULL);

  copy->adjacency_list = (region_adjacency_t *)
    _rag_cpy_memory_space(copy->adjacency_list, &copy->max_adjacency_list,
			  &copy->n_adjacency_list,
			  rag->adjacency_list, rag->max_adjacency_list,
			  rag->n_adjacency_list, sizeof(region_adjacency_t),
			  (rag_copyFunc *) region_adjacency_cpy,
			  (rag_initFunc *) region_adjacency_init,
			  (rag_freeFunc *) region_adjacency_free);

  copy->adjacency_index = (int *)
    _rag_cpy_memory_space(copy->adjacency_index, &copy->max_adjacency_index,
			  &copy->n_adjacency_index,
			  rag->adjacency_index, rag->max_adjacency_index,
			  rag->n_adjacency_index, sizeof(int), NULL,NULL,NULL);

  copy->contour_points = (contour_point_t *)
    _rag_cpy_memory_space(copy->contour_points, &copy->max_contour_points,
			  &copy->n_contour_points,
			  rag->contour_points, rag->max_contour_points,
			  rag->n_contour_points, sizeof(contour_point_t),
			  NULL, NULL, NULL);
  
  /** correct external contour-pointers */
  if (copy->contour_points != rag->contour_points) {
    int i;
    for (i=0; i < copy->n_regions; i++) {
      if (copy->region[i].max_contour < 0) {
	copy->region[i].contour 
	  = (copy->region[i].contour 
	     - rag->contour_points) + copy->contour_points;
      }
    }
  }

  /** correct region-pointers */
  if (copy->region != rag->region) {
    int i, n = copy->n_adjacency_list;
    for (i=0; i < n; i++) {
      copy->adjacency_list[i].region1 = (copy->adjacency_list[i].region1
					 - rag->region) + copy->region;
      copy->adjacency_list[i].region2 = (copy->adjacency_list[i].region2
					 - rag->region) + copy->region;
    }
  }
  return (copy);
}

/* rag_cpyPt: copies only the references on region and adjacency memory */
rag_t *rag_cpyPt(rag_t *copy, const rag_t *rag)
{
  if (copy) { 
    rag_free(copy); rag_init(copy); 
  }
  if (!rag) return (copy);
  if (!copy)
    copy = rag_create();
  
  memcpy(copy,rag,sizeof(rag_t));
  
  copy->max_colorMap = -1;
  copy->max_regionMap = -1;
  copy->max_contourIndexMap = -1;
  copy->max_colorTab = -1;
  copy->max_adjacency_list = -1;
  copy->max_adjacency_index = -1;
  copy->max_regions = -1;
  copy->max_region_index = -1;
  copy->max_contour_points = -1;
  {
    int i;
    for (i=0; i < RAG_TMP_ADJLISTS; i++)
      copy->max_tmp_adjacencies[i] = -1;
    for (i=0; i < RAG_TMP_REGLISTS; i++)
      copy->max_tmp_regions[i] = -1;
  }
  return (copy);
}

rag_t *rag_dup(const rag_t *rag)
{
  return (rag_cpy(NULL,rag));
}

/* rag_dupPt: calls rag_cpyPt(NULL,rag) */
rag_t *rag_dupPt(const rag_t *rag)
{
  return (rag_cpyPt(NULL,rag));
}

/* rag_movPt: moves the region and adjacency memory to copy and leaves
 *   rag with references on the memory */
rag_t *rag_movPt(rag_t *copy, rag_t *rag)
{
  if (copy) { 
    rag_free(copy); rag_init(copy); 
  }
  if (!rag) return (copy);
  if (!copy)
    copy = rag_create();
  
  memcpy(copy,rag,sizeof(rag_t));
  
  rag->max_colorMap = -1;
  rag->max_regionMap = -1;
  rag->max_contourIndexMap = -1;
  rag->max_colorTab = -1;
  rag->max_adjacency_list = -1;
  rag->max_adjacency_index = -1;
  rag->max_regions = -1;
  rag->max_region_index = -1;
  rag->max_contour_points = -1;
  {
    int i;
    for (i=0; i < RAG_TMP_ADJLISTS; i++)
      rag->max_tmp_adjacencies[i] = -1;
    for (i=0; i < RAG_TMP_REGLISTS; i++)
      rag->max_tmp_regions[i] = -1;
  }
  return (copy);
}
 
rag_t *rag_clear(rag_t *rag)
{
  int i;
  if (!rag) return (rag);

  rag->image = NULL;

  rag->cols = 0;
  rag->rows = 0;
  rag->n_colors = 0;

  rag_clearRegions(rag);

  return (rag);
}
  
rag_t *rag_clearRegions(rag_t *rag)
{
  int i;
  if (!rag) return (rag);

  if (rag->max_adjacency_list > 0)
    for (i=0; i < rag->max_adjacency_list; i++) {
      region_adjacency_free(&rag->adjacency_list[i]);
      region_adjacency_init(&rag->adjacency_list[i]);
    }
  rag->n_adjacency_list = 0;
  rag->n_adjacency_index = 0;

  if (rag->max_regions > 0)
    for (i=0; i < rag->max_regions; i++) {
      region_info_free(&rag->region[i]);
      region_info_init(&rag->region[i]);
    }
  rag->n_regions = 1;
  rag->n_region_index = 1;

  rag->n_contour_points = 0;

  return (rag);
}
  

ppm_t *                           /* returns the previously stored image */
rag_setImage(rag_t *rag, 
		ppm_t *image      /* sets reference on this image */
		)
{
  ppm_t *_image;
  if (!rag) return (NULL);

  _image = rag->image;
  rag->image = image;
  return (_image);
}


unsigned char *   /* returns previous colorMap, if stored by reference */
rag_setColorMap(rag_t *rag, 
		   unsigned char *colorMap,  /* new colorMap */
		   int max_colorMap,    /* if (-1) set reference on colorMap
					 * if (-2) copy colorMap
					 * if (>0) keep colorMap memory */
		   int cols, int rows
		   )
{
  unsigned char *_map;
  if (!rag) return (NULL);

  _map = (rag->max_colorMap < 0) ? rag->colorMap : NULL;
  rag->colorMap = (unsigned char *)
    _rag_set_memory_space(rag->colorMap, &rag->max_colorMap, NULL,
			  colorMap, max_colorMap, cols * rows,
			  sizeof(unsigned char), NULL, NULL, NULL);
  rag->cols = cols;
  rag->rows = rows;

  return (_map);
}


int *             /* returns previous regionMap, if stored by reference */
rag_setRegionMap(rag_t *rag, 
		    int *regionMap,    /* new regionMap */
		    int max_regionMap, /* if (-1) set reference on regionMap
					* if (-2) copy regionMap
					* if (>0) keep regionMap memory */
		    int cols, int rows
		    )
{
  int *_map;
  if (!rag) return (NULL);
 
  _map = (rag->max_regionMap < 0) ? rag->regionMap : NULL;
  rag->regionMap = (int *)
    _rag_set_memory_space(rag->regionMap, &rag->max_regionMap, NULL,
			  regionMap, max_regionMap, cols * rows,
			  sizeof(int), NULL, NULL, NULL);
  rag->cols = cols;
  rag->rows = rows;

  return (_map);
}


int *             /* returns previous regionMap, if stored by reference */
rag_setContourIndexMap(rag_t *rag, 
			  int *contourIndexMap, /* new contourIndexMap */
			  int max_contourIndexMap, /* if (-1) set reference
						    * if (-2) copy mem
						    * if (>0) keep mem */
			  int cols, int rows
			  )
{
  int *_map;
  if (!rag) return (NULL);
 
  _map = (rag->max_contourIndexMap < 0) ? rag->contourIndexMap : NULL;
  rag->contourIndexMap = (int *)
    _rag_set_memory_space(rag->contourIndexMap, &rag->max_contourIndexMap, 
			  NULL,
			  contourIndexMap, max_contourIndexMap, cols * rows,
			  sizeof(int), NULL, NULL, NULL);
  if (rag->cols != cols || rag->rows != rows)
    fprintf(stderr, "rag_setContourIndexMap: size (%d,%d) differs from regionMap (%d,%d)!\n",cols,rows,rag->cols,rag->rows);

  return (_map);
}


rag_color_t *        /* returns previous colorTab, if stored by reference */
rag_setColorTab(rag_t *rag, 
		rag_color_t *colorTab,    /* new colorTab */
		int max_colorTab,         /* if (-1) set reference
					   * if (-2) copy mem
					   * if (>0) keep mem */
		int n_colors
    )
{
  rag_color_t *_tab;
  if (!rag) return (NULL);

  _tab = (rag->max_colorTab < 0) ? rag->colorTab : NULL;
  rag->colorTab = (rag_color_t *)
    _rag_set_memory_space(rag->colorTab, &rag->max_colorTab, &rag->n_colors,
			  colorTab, max_colorTab, n_colors,
			  sizeof(rag_color_t), NULL, NULL, NULL);

  return (_tab);
}


rag_color_t *
rag_setColorTabByPixel(rag_t *rag,
		       pixel *colorTab, /* new colorTab */
		       int n_colors
		       )
{
  rag_color_t *_tab;
  if (!rag) return (NULL);

  _tab = (rag_color_t *) calloc(n_colors, sizeof(rag_color_t));
  {
    int i;
    /* in pseudo.c: pnm_ppmSegColor2charMap colors are defined
     * for 0 <= index < n_colors-1 */
    for (i=0; i < n_colors-1; i++) {
      pixel c = colorTab[i];
      _tab[i].r = PPM_GETR(c);
      _tab[i].g = PPM_GETG(c);
      _tab[i].b = PPM_GETB(c);
    }
  }
  return (rag_setColorTab(rag, _tab, n_colors, n_colors));
}

rag_color_t *rag_newColor(rag_t *rag, int *i_color)
{
  if (!rag) return (NULL);
  
  rag->colorTab = (rag_color_t *)
      _rag_realloc_memory_space(rag->colorTab, &rag->max_colorTab,
				rag->n_colors+1, sizeof(rag_color_t),
				NULL, NULL, NULL);
  if (i_color) (*i_color) = rag->n_colors+1;

  return (&rag->colorTab[rag->n_colors++]);
}

int rag_getColorTabPixels(pixel **colors, /* result */
			  rag_t *rag)
{
  int         n_colors;
  rag_color_t *_tab;
  if (!rag) return (0);

  _tab = rag->colorTab;
  n_colors = rag->n_colors;

  if (colors) {
    int i;
    if (*colors) 
      (*colors) = (pixel *) realloc(*colors, (n_colors) * sizeof(pixel));
    else
      (*colors) = (pixel *) calloc(n_colors, sizeof(pixel));

    /* in pseudo.c: pnm_ppmSegColor2charMap colors are defined
     * for 0 <= index < n_colors-1 */
    for (i=0; i < n_colors; i++) {
      PPM_ASSIGN((*colors)[i],_tab[i].r,_tab[i].g,_tab[i].b); 
    }
  }
  return (n_colors);
}

region_info_t *   /* returns previous regions, if stored by reference */ 
rag_setRegions(rag_t *rag, 
	       region_info_t *region, /* new region mem */
	       int max_regions,       /* if (-1) set reference
				       * if (-2) copy mem
				       * if (>0) keep mem */
	       int n_regions
	       )
{
  region_info_t *_region;
  if (!rag) return (NULL);

  _region = (rag->max_regions < 0) ? rag->region : NULL;
  rag->region = (region_info_t *)
    _rag_set_memory_space(rag->region, &rag->max_regions, &rag->n_regions,
			  region, max_regions, n_regions, 
			  sizeof(region_info_t),
			  (rag_copyFunc *) region_info_cpy,
			  (rag_initFunc *) region_info_init,
			  (rag_freeFunc *) region_info_free);

  /* if rag->region_index is only a reference, it is reallocated */
  rag->region_index = 
    _rag_initIndex(rag->region_index, &rag->max_region_index,
		   &rag->n_region_index, 0, n_regions);

  return (_region);
}

contour_point_t * /* returns previous points, if stored by reference */ 
rag_reallocContourPoints(rag_t *rag, int n_points)
{
  contour_point_t *_points;
  if (!rag) return (NULL);

  _points = (rag->max_contour_points < 0) ? rag->contour_points : NULL;
  rag->contour_points = (contour_point_t *)
    _rag_realloc_memory_space(rag->contour_points, &rag->max_contour_points,
			      n_points, sizeof(contour_point_t), 
			      NULL, NULL, NULL);

  return (_points);  
}
  

contour_point_t * /* returns previous points, if stored by reference */ 
rag_setContourPoints(rag_t *rag, 
			contour_point_t *points, /* new point mem */
			int max_points,          /* if (-1) set reference
						  * if (-2) copy mem
						  * if (>0) keep mem */
			int n_points
			)
{
  contour_point_t *_points;
  if (!rag) return (NULL);

  _points = (rag->max_contour_points < 0) ? rag->contour_points : NULL;
  rag->contour_points = (contour_point_t *)
    _rag_set_memory_space(rag->contour_points, &rag->max_contour_points,
			  &rag->n_contour_points,
			  points, max_points, n_points, 
			  sizeof(contour_point_t), NULL, NULL, NULL);

  return (_points);  
}

region_adjacency_t * /* returns previous adjacencies, if stored by reference */
rag_setAdjacencyList(rag_t *rag, 
			region_adjacency_t *adj, /* new adjacency mem */
			int max_adj,             /* if (-1) set reference
						  * if (-2) copy mem
						  * if (>0) keep mem */
			int n_adj
			)
{
  region_adjacency_t *_adj;
  if (!rag) return (NULL);
  
  _adj = (rag->max_adjacency_list < 0) ? rag->adjacency_list : NULL;
  rag->adjacency_list = (region_adjacency_t *)
    _rag_set_memory_space(rag->adjacency_list, &rag->max_adjacency_list,
			  &rag->n_adjacency_list,
			  adj, max_adj, n_adj, sizeof(region_adjacency_t),
			  (rag_copyFunc *) region_adjacency_cpy,
			  (rag_initFunc *) region_adjacency_init,
			  (rag_freeFunc *) region_adjacency_free);

  /* if rag->adjacency_index is only a reference, it is reallocated */
  rag->adjacency_index = 
    _rag_initIndex(rag->adjacency_index, &rag->max_adjacency_index,
		      &rag->n_adjacency_index, 0, n_adj);
  
  return (_adj);
}

/* ---------------------------------------------------------------------- */

region_info_t *rag_getRegionByLabel(rag_t *rag, int label,
				    int *index)
{
  if (!rag 
      || label <= 0 
      || label >= rag->n_regions) return (NULL);

  if (index) {
    /* search for index */
    int i;
    for (i=rag->n_region_index-1; 
	 i > 0 && rag->region_index[i] != label; 
	 i--);
    /* An index may not be found because the region[label] has been deleted.
     * In this case (*index) is set to (0) ) */
    (*index) = i;
  }
  /* If region[label] exists, but has been deleted, the label in
   * the region structure is set to 0 which is not equal to the questioned 
   * label. */
  return ((rag->region[label].label != label) ? NULL
	  : &rag->region[label]);
}

region_info_t *rag_getRegionByIndex(rag_t *rag, int index)
{
  if (!rag) return (NULL);

  return ((index > 0 && index < rag->n_region_index) ? 
	  RAG_GET_REGION(rag,index) : NULL);
}

region_info_t *rag_getFirstRegion(rag_t *rag, int *index)
{
  if (index) (*index) = 1;
  
  return ((rag && rag->n_region_index > 1) ?
	  RAG_GET_REGION(rag,1) : NULL);
}

region_info_t *rag_getLastRegion(rag_t *rag, int *index)
{
  int i;
  if (!rag) {
    if (index) (*index) = 0;
    return (NULL);
  }
  i = rag->n_region_index - 1;
  if (index) (*index) = i;

  return (i > 0) ? RAG_GET_REGION(rag,i) : NULL;
}

region_info_t *rag_getNextRegion(rag_t *rag, int *index)
{
  int i;
  if (!rag || !index) return (NULL);

  i = ++(*index);
  return ((i > 0 && i < rag->n_region_index) ?
	  RAG_GET_REGION(rag,i) : NULL);
}

region_info_t *rag_getPrevRegion(rag_t *rag, int *index)
{
  int i;
  if (!rag || !index) return (NULL);

  i = --(*index);
  return ((i > 0 && i < rag->n_region_index) ?
	  RAG_GET_REGION(rag,i) : NULL);
}

region_info_t **rag_getRegionArray(region_info_t **regions,
				      int *max_regions,
				      int *n_regions,
				      rag_t *rag)
{
  int i;
  if (n_regions) (*n_regions) = 0;
  if (!rag) return (regions);

  regions = (region_info_t **)
    _rag_realloc_memory_space(regions, max_regions, RAG_N_REGIONS(rag),
			      sizeof(region_info_t *), NULL, NULL, NULL);
  RAG_FORALL_REGIONS(rag,i) {
    regions[i] = RAG_GET_REGION(rag,i);
  }
  if (n_regions) (*n_regions) = RAG_N_REGIONS(rag);

  return (regions);
}

region_info_t *rag_getRegion4Label(rag_t *rag, int label, int *index)
{
  int i;
  region_info_t *region = NULL;

  if (!rag || label <= 0) return (NULL);

  /** check if label is a valid region */
  if (region = rag_getRegionByLabel(rag, label, index))
    return (region);

  /** check if new memory has to be (re)allocated */ 
  if (label >= rag->max_regions) {
    rag_realloc(rag, label+1, 0);
  }
  if (label >= rag->n_regions)
    rag->n_regions = label+1;

  region = &rag->region[label];
  i = rag_regionLabel2Index(rag, label);
  if (i < 0) {
    fprintf(stderr,"rag_getRegion4Label: rag inconsistent: no index for label %d.\n",label);
    exit(1);
  }
  rag->region_index[i] = rag->region_index[rag->n_region_index];
  rag->region_index[rag->n_region_index] = label;

  if (index) (*index) = rag->n_region_index;
  rag->n_region_index++;

  region->label = label;
  rag_setSubLabelByLabel(rag, label);

  return (region);
}

region_info_t *rag_newRegion(rag_t *rag, int *index)
{
  int i,label;
  region_info_t *region = NULL;

  if (!rag) return (NULL);
  
  i = rag->n_region_index;
  if (i >= rag->max_region_index)
    rag_realloc(rag, i+RAG_ADDITEMS,0);
  
  if (index) (*index) = i;
  rag->n_region_index++;

  label = rag->region_index[i];
  region = &rag->region[label];
  region->label = label;
  if (label >= rag->n_regions)
    rag->n_regions = label + 1;

  rag_setSubLabelByLabel(rag, label);

  return (region);
}

region_info_t *
rag_addRegion(rag_t *rag, region_info_t *region, int *index)
{
  int label;
  region_info_t *_region = NULL;

  if (!rag || !region) return (NULL);

  label = region->label;
  if (label <= 0) {
    _region = rag_newRegion(rag,index);
    if (_region) label = _region->label;
  } else
    _region = rag_getRegion4Label(rag, label, index);

  if (!_region) {
    fprintf(stderr, "rag_setRegion: cannot set region with label %d.\n",
	    label);
  }
  region_info_cpy(_region, region);
  _region->label = label;

  rag_setSubLabelByLabel(rag, label);

  return (_region);
}

rag_t *rag_delRegionByLabel(rag_t *rag, int label)
{
  int i;
  region_info_t *region = NULL;

  if (!rag) return (rag);

  if (region = rag_getRegionByLabel(rag, label, &i)) {
    int j = rag->n_region_index - 1;
    region_info_free(region);
    region_info_init(region);
    rag->region_index[i] = rag->region_index[j];
    rag->region_index[j] = label;
    rag->n_region_index--;
  }
  return (rag);
}
  
rag_t *rag_delRegionByIndex(rag_t *rag, int index)
{
  region_info_t *region = NULL;

  if (!rag) return (rag);

  if (region = rag_getRegionByIndex(rag, index)) {
    int j = rag->n_region_index - 1;
    int label = region->label;

    region_info_free(region);
    region_info_init(region);
    rag->region_index[index] = rag->region_index[j];
    rag->region_index[j] = label;
    rag->n_region_index--;
  }
  return (rag);
}
 
rag_t *rag_delLastRegion(rag_t *rag)
{
  region_info_t *region = NULL;

  if (!rag || rag->n_region_index <= 1) return (rag);

  region = RAG_GET_REGION(rag,--rag->n_region_index);
  region_info_free(region);
  region_info_init(region);

  return (rag);
}

rag_t *rag_blockRegionMapLabels(rag_t *rag)
{
  int *regionMap;
  int i,n,max=-1;
  if (!rag || !rag->regionMap) return (rag);

  regionMap = rag->regionMap;
  n=rag->cols*rag->rows;

  for (i=0; i < n; i++)
    if (regionMap[i] > max) max = regionMap[i];
  for (i=1; i <= max; i++)
    rag_getRegion4Label(rag,i,NULL);
  return (rag);
}

region_info_t **
rag_reallocTmpRegions(rag_t *rag, int tmp_index, int n)
{
  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_REGLISTS)
    return (NULL);

  rag->tmp_regions[tmp_index] = (region_info_t **)
    _rag_realloc_memory_space(rag->tmp_regions[tmp_index], 
			      &rag->max_tmp_regions[tmp_index], 
			      n, sizeof(region_info_t *), 
			      NULL,NULL,NULL);
  return (rag->tmp_regions[tmp_index]);
}

region_info_t **
rag_getRegions(region_info_t **regions, int *max_regions, rag_t *rag)
{
  int i;
  if (!rag) return (regions);

  regions = (region_info_t **)
    _rag_realloc_memory_space(regions, max_regions, rag->n_regions, 
			      sizeof(region_info_t *), NULL,NULL,NULL);
  memset(regions,0,rag->n_regions * sizeof(region_info_t *));

  RAG_FORALL_REGIONS(rag,i) {
    region_info_t *reg = RAG_GET_REGION(rag,i);
    if (!reg || reg->label <= 0) continue;
    
    regions[reg->label] = reg;
  }
  return (regions);
}

region_info_t **
rag_getTmpRegions(rag_t *rag, int tmp_index)
{
  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_REGLISTS)
    return (NULL);

  rag->tmp_regions[tmp_index] =
    rag_getRegions(rag->tmp_regions[tmp_index],
		   &rag->max_tmp_regions[tmp_index],
		   rag);
  return (rag->tmp_regions[tmp_index]);
}

region_info_t **
rag_newTmpRegions(rag_t *rag, int tmp_index, int min_label,
		     int max_label)
{
  int i;
  int n_regions = max_label+1;
  region_info_t **regions;

  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_REGLISTS) 
    return (NULL);
  
  if (min_label <= 0) min_label = 1;
  if (max_label < min_label) return (NULL);

  regions = rag_reallocTmpRegions(rag, tmp_index, n_regions);
  memset(regions,0, n_regions * sizeof(region_info_t *));
  rag_realloc(rag, n_regions, 0);

  for (i=min_label; i <= max_label; i++) {
    region_info_t *reg = rag_getRegion4Label(rag, i, NULL);
    regions[i] = reg;
  }
  return (regions);
}

/* ---------------------------------------------------------------------- */

region_adjacency_t *rag_getAdjacencyByLabel(rag_t *rag, int label,
					       int *index)
{
  if (!rag 
      || label < 0 
      || label >= rag->n_adjacency_list) return (NULL);

  if (index) {
    /* search for index */
    int i;
    for (i=rag->n_adjacency_index-1; 
	 i >= 0 && rag->adjacency_index[i] != label; 
	 i--);
    /* An index may not be found because the adjacency_list[label] has 
     * been deleted. In this case (*index) is set to (-1) */
    (*index) = i;
  }
  /* If adjacency_list[label] exists, but has been deleted, both
   * region pointers or region labels are 0. */
  return ((rag->adjacency_list[label].region1 == NULL
	   && rag->adjacency_list[label].region1_label <= 0) ? NULL
	  : &rag->adjacency_list[label]);
}

region_adjacency_t *rag_getAdjacencyByIndex(rag_t *rag, int index)
{
  if (!rag) return (NULL);

  return ((index >= 0 && index < rag->n_adjacency_index) ? 
	  RAG_GET_ADJACENCY(rag,index) : NULL);
}

region_adjacency_t *rag_getFirstAdjacency(rag_t *rag, int *index)
{
  if (index) (*index) = 0;
  
  return ((rag && rag->n_adjacency_index > 0) ?
	  RAG_GET_ADJACENCY(rag,0) : NULL);
}

region_adjacency_t *rag_getLastAdjacency(rag_t *rag, int *index)
{
  int i;
  if (!rag) {
    if (index) (*index) = -1;
    return (NULL);
  }
  i = rag->n_adjacency_index - 1;
  if (index) (*index) = i;

  return (i >= 0) ? RAG_GET_ADJACENCY(rag,i) : NULL;
}

region_adjacency_t *rag_getNextAdjacency(rag_t *rag, int *index)
{
  int i;
  if (!rag || !index) return (NULL);

  i = ++(*index);
  return ((i >= 0 && i < rag->n_adjacency_index) ?
	  RAG_GET_ADJACENCY(rag,i) : NULL);
}

region_adjacency_t *rag_getPrevAdjacency(rag_t *rag, int *index)
{
  int i;
  if (!rag || !index) return (NULL);

  i = --(*index);
  return ((i >= 0 && i < rag->n_adjacency_index) ?
	  RAG_GET_ADJACENCY(rag,i) : NULL);
}
  
region_adjacency_t **rag_getAdjacencyArray(region_adjacency_t **adjs,
					      int *max_adjs,
					      int *n_adjs,
					      rag_t *rag)
{
  int i;
  if (n_adjs) (*n_adjs) = 0;
  if (!rag) return (adjs);

  adjs = (region_adjacency_t **)
    _rag_realloc_memory_space(adjs, max_adjs, RAG_N_ADJACENCIES(rag),
			      sizeof(region_adjacency_t *), NULL, NULL, NULL);
  RAG_FORALL_ADJACENCIES(rag,i) {
    adjs[i] = RAG_GET_ADJACENCY(rag,i);
  }
  if (n_adjs) (*n_adjs) = RAG_N_ADJACENCIES(rag);

  return (adjs);
}

region_adjacency_t *
rag_getAdjacency4Label(rag_t *rag, int label, int *index)
{
  int i;
  region_adjacency_t *adjacency = NULL;

  if (!rag || label < 0) return (NULL);

  /** check if label is a valid adjacency */
  if (adjacency = rag_getAdjacencyByLabel(rag, label, index))
    return (adjacency);

  /** check if new memory has to be (re)allocated */ 
  if (label >= rag->max_adjacency_list) {
    rag_realloc(rag, 0, label+1);
  }
  adjacency = &rag->adjacency_list[label];
  
  if (label >= rag->n_adjacency_list)
    rag->n_adjacency_list = label + 1;

  i = rag_adjacencyLabel2Index(rag, label);
  if (i < 0) {
    fprintf(stderr,"rag_getAdjacency4Label: rag inconsistent: no index for label %d.\n",label);
    exit(1);
  }
  rag->adjacency_index[i] 
    = rag->adjacency_index[rag->n_adjacency_index];
  rag->adjacency_index[rag->n_adjacency_index] = label;

  if (index) (*index) = rag->n_adjacency_index;
  rag->n_adjacency_index++;

  adjacency->label = label;

  return (adjacency);
}

region_adjacency_t *rag_newAdjacency(rag_t *rag, int *index)
{
  int i,label;
  region_adjacency_t *adjacency = NULL;

  if (!rag) return (NULL);
  
  i = rag->n_adjacency_index;
  if (i >= rag->max_adjacency_index)
    rag_realloc(rag, 0, i+RAG_ADDITEMS);
  
  if (index) (*index) = i;
  rag->n_adjacency_index++;

  label = rag->adjacency_index[i];
  adjacency = &rag->adjacency_list[label];
  adjacency->label = label;

  if (label >= rag->n_adjacency_list)
    rag->n_adjacency_list = label+1;

  return (adjacency);
}

region_adjacency_t *
rag_addAdjacency(rag_t *rag,region_adjacency_t *adjacency,int *index)
{
  int label;
  region_adjacency_t *_adjacency = NULL;

  if (!rag || !adjacency) return (NULL);

  label = adjacency->label;
  if (label <= 0) {
    _adjacency = rag_newAdjacency(rag,index);
    if (_adjacency) label = _adjacency->label;
  } else
    _adjacency = rag_getAdjacency4Label(rag, label, index);

  if (!_adjacency) {
    fprintf(stderr, "rag_setAdjacency: cannot set adjacency with label %d.\n", label);
  }
  region_adjacency_cpy(_adjacency, adjacency);
  _adjacency->label = label;

  return (_adjacency);
}

rag_t *rag_delAdjacencyByLabel(rag_t *rag, int label)
{
  int i;
  region_adjacency_t *adjacency = NULL;

  if (!rag) return (rag);

  if (adjacency = rag_getAdjacencyByLabel(rag, label, &i)) {
    int j = rag->n_adjacency_index - 1;
    region_adjacency_free(adjacency);
    region_adjacency_init(adjacency);
    adjacency->label = label;
    rag->adjacency_index[i] = rag->adjacency_index[j];
    rag->adjacency_index[j] = label;
    rag->n_adjacency_index--;
  }
  return (rag);
}
  
rag_t *rag_delAdjacencyByIndex(rag_t *rag, int index)
{
  region_adjacency_t *adjacency = NULL;

  if (!rag) return (rag);

  if (adjacency = rag_getAdjacencyByIndex(rag, index)) {
    int j = rag->n_adjacency_index - 1;
    int label = adjacency->label;

    region_adjacency_free(adjacency);
    region_adjacency_init(adjacency);
    rag->adjacency_index[index] = rag->adjacency_index[j];
    rag->adjacency_index[j] = label;
    rag->n_adjacency_index--;
  }
  return (rag);
}
 
rag_t *rag_delLastAdjacency(rag_t *rag)
{
  region_adjacency_t *adjacency = NULL;

  if (!rag || rag->n_adjacency_index <= 0) return (rag);

  adjacency = RAG_GET_ADJACENCY(rag,--rag->n_adjacency_index);
  region_adjacency_free(adjacency);
  region_adjacency_init(adjacency);

  return (rag);
}

region_adjacency_t **
rag_reallocTmpAdjacencies(rag_t *rag, int tmp_index, int n)
{
  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_ADJLISTS)
    return (NULL);

  rag->tmp_adjacencies[tmp_index] = (region_adjacency_t **)
    _rag_realloc_memory_space(rag->tmp_adjacencies[tmp_index], 
			      &rag->max_tmp_adjacencies[tmp_index], 
			      n, sizeof(region_adjacency_t *), 
			      NULL,NULL,NULL);
  return (rag->tmp_adjacencies[tmp_index]);
}

region_adjacency_t **rag_getTmpAdjacencies(rag_t *rag, int tmp_index,
					      int label)
{
  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_ADJLISTS)
    return (NULL);

  rag->tmp_adjacencies[tmp_index] =
    rag_getAdjacencies(rag->tmp_adjacencies[tmp_index],
		       &rag->max_tmp_adjacencies[tmp_index],
		       rag, label);
  return (rag->tmp_adjacencies[tmp_index]);
}

region_adjacency_t **rag_getAdjacencies(region_adjacency_t **adjs,
					int *max_adjs,
					rag_t *rag, int label)
{
  int i;
  if (!rag || label <= 0 || label >= rag->n_regions) return (adjs);

  adjs = (region_adjacency_t **)
    _rag_realloc_memory_space(adjs, max_adjs, rag->n_regions, 
			      sizeof(region_adjacency_t *), NULL,NULL,NULL);
  memset(adjs,0,rag->n_regions * sizeof(region_adjacency_t *));

  RAG_FORALL_ADJACENCIES(rag,i) {
    region_adjacency_t *adj = RAG_GET_ADJACENCY(rag,i);
    if (!adj || !adj->region1 || adj->region1->label != label) continue;
    
    adjs[adj->region2->label] = adj;
  }
  return (adjs);
}

region_adjacency_t **rag_getTmpRevAdjacencies(rag_t *rag, int tmp_index,
						 int label)
{
  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_ADJLISTS)
    return (NULL);

  rag->tmp_adjacencies[tmp_index] =
    rag_getRevAdjacencies(rag->tmp_adjacencies[tmp_index],
			  &rag->max_tmp_adjacencies[tmp_index],
			  rag, label);
  return (rag->tmp_adjacencies[tmp_index]);
}

region_adjacency_t **rag_getRevAdjacencies(region_adjacency_t **rev_adjs,
					   int *max_adjs, 
					   rag_t *rag, int label)
{
  int i;
  if (!rag || label <= 0 || label >= rag->n_regions) return (NULL);

  rev_adjs = (region_adjacency_t **)
    _rag_realloc_memory_space(rev_adjs, max_adjs, rag->n_regions, 
			      sizeof(region_adjacency_t *), NULL,NULL,NULL);
  memset(rev_adjs,0,rag->n_regions * sizeof(region_adjacency_t *));

  RAG_FORALL_ADJACENCIES(rag,i) {
    region_adjacency_t *adj = RAG_GET_ADJACENCY(rag,i);
    if (!adj || !adj->region2 || adj->region2->label != label) continue;
    
    rev_adjs[adj->region1->label] = adj;
  }
  return (rev_adjs);
}

region_adjacency_t **rag_newTmpAdjacencies(rag_t *rag, 
					   int tmp_index,
					   int label)
{
  int i;
  region_info_t *region1;
  region_adjacency_t **adjs;

  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_ADJLISTS
      || label <= 0 || label >= rag->n_regions) return (NULL);

  region1 = rag_getRegionByLabel(rag,label,NULL);
  if (!region1) return (NULL);

  adjs = rag_reallocTmpAdjacencies(rag, tmp_index, rag->n_regions);
  memset(adjs,0, rag->n_regions * sizeof(region_adjacency_t *));
  rag_realloc(rag,0,rag->n_adjacency_index+rag->n_regions);

  rag_consistent(rag,"RAG_REALLOC");

  RAG_FORALL_REGIONS(rag,i) {
    region_info_t *region2 = RAG_GET_REGION(rag,i);
    region_adjacency_t *adj = rag_newAdjacency(rag,NULL);
    adj->region1 = region1;
    adj->region2 = region2;
    adj->region1_label = region1->label;
    adj->region2_label = region2->label;
    adjs[region2->label] = adj;
  }
  rag_consistent(rag,"new adjs");

  return (adjs);
}

region_adjacency_t **rag_newTmpRevAdjacencies(rag_t *rag, 
					      int tmp_index,
					      int label)
{
  int i;
  region_info_t *region2;
  region_adjacency_t **adjs;

  if (!rag || tmp_index < 0 || tmp_index >= RAG_TMP_ADJLISTS
      || label <= 0 || label >= rag->n_regions) return (NULL);

  region2 = rag_getRegionByLabel(rag,label,NULL);
  if (!region2) return (NULL);

  adjs = rag_reallocTmpAdjacencies(rag, tmp_index, rag->n_regions);
  memset(adjs,0, rag->n_regions * sizeof(region_adjacency_t *));
  rag_realloc(rag,0,rag->n_adjacency_index+rag->n_regions);

  RAG_FORALL_REGIONS(rag,i) {
    region_info_t *region1 = RAG_GET_REGION(rag,i);
    region_adjacency_t *adj = rag_newAdjacency(rag,NULL);
    adj->region1 = region1;
    adj->region2 = region2;
    adjs[region1->label] = adj;
  }
  return (adjs);
}

region_adjacency_t **
rag_clearUnusedAdjacencies(rag_t *rag, 
			   region_adjacency_t **adjacencies, 
			   int n_adjacencies)
{
  int i;
  if (!adjacencies) return (adjacencies);

  for (i=0; i < n_adjacencies; i++) {
    if (!adjacencies[i]) continue;
    
    if (!region_isAdjacent(adjacencies[i])) {
      if (rag)
	rag_delAdjacencyByLabel(rag, adjacencies[i]->label);
      else
	region_adjacency_destroy(adjacencies[i]);
      adjacencies[i] = NULL;
    }
  }
  return (adjacencies);
}

/* --------------------------------------------------------------------- */

int *rag_getSubLabelsByIndex(rag_t *rag, int index, int *n_subLabels)
{
  if (!rag 
      || index < 0 
      || index >= RAG_N_REGIONS(rag)) return (NULL);
  
  if (n_subLabels) 
    (*n_subLabels) = RAG_N_SUBLABELS(rag,index);
  return (RAG_GET_SUBLABELS(rag,index));
}

int *rag_getSubLabelsByLabel(rag_t *rag, int label, int *n_subLabels)
{
  if (!rag 
      || label <= 0 
      || label >= rag->n_regions) return (NULL);
  
  if (n_subLabels) 
    (*n_subLabels) = rag->region[label].n_subLabels;
  return (rag->region[label].subLabels);
}

/*************
int *rag_subLabels_realloc(rag_t *rag, int label, int subLabels_n)
{
  int n_labels = label+1;
  int _max_subLabels, _max_subLabels_max, _max_subLabels_n;
  if (!rag || label < 0) return (NULL);

  _max_subLabels = _max_subLabels_max = _max_subLabels_n 
    = rag->max_subLabels;

  rag->subLabels = (int **)
    _rag_realloc_memory_space(rag->subLabels, &rag->max_subLabels,
			      n_labels, sizeof(int *), NULL, NULL, NULL);
  rag->subLabels_max = (int *)
    _rag_realloc_memory_space(rag->subLabels_max, &_max_subLabels_max,
			      n_labels, sizeof(int), NULL, NULL, NULL);
  rag->subLabels_n = (int *)
    _rag_realloc_memory_space(rag->subLabels_n, &_max_subLabels_n,
			      n_labels, sizeof(int), NULL, NULL, NULL);

  /* if subLabels is only reference on memory (external control)
   * after a realloc all entries have to be newly allocated ... *%

  if (_max_subLabels < 0) {
    int i;
    for (i=0; i < rag->max_subLabels; i++)
      rag->subLabels_max[i] = -1;
      rag->subLabels[i] = (int *)
	_rag_realloc_memory_space(rag->subLabels[i], 
				  &rag->subLabels_max[i],
				  rag->subLabels_n[i], sizeof(int),
				  NULL,NULL,NULL);
  }
  rag->subLabels[label] = (int *)
    _rag_realloc_memory_space(rag->subLabels[label],
			      &rag->subLabels_max[label],
			      subLabels_n, sizeof(int),
			      NULL,NULL,NULL);

  return (rag->subLabels[label]);
}
  ******************/

int *rag_setSubLabelByLabel(rag_t *rag, int label)
{
  if (!rag || label <= 0 || label >= rag->n_regions) 
    return (NULL);

  region_setSubLabel(&rag->region[label]);

  return (rag->region[label].subLabels);
}

int *rag_setSubLabelByIndex(rag_t *rag, int index)
{
  int label = rag_regionIndex2Label(rag,index);
  return rag_setSubLabelByIndex(rag, label);
}

int *rag_clearSubLabelsByLabel(rag_t *rag, int label)
{
  if (!rag || label <= 0 || label >= rag->n_regions)
    return (NULL);
  
  region_clearSubLabels(&rag->region[label]);
  
  return (rag->region[label].subLabels);
}

int *rag_clearSubLabelsByIndex(rag_t *rag, int index)
{
  int label = rag_regionIndex2Label(rag,index);
  return (rag_clearSubLabelsByLabel(rag, label));
}

int *rag_addSubLabelsByLabel(rag_t *rag, int label, 
			     int *subLabels, int n_subLabels)
{
  if (!rag || label <= 0 || label >= rag->n_regions) 
    return (NULL);

  region_addSubLabels(&rag->region[label], subLabels, n_subLabels);
  
  return (rag->region[label].subLabels);
}
  
int *rag_addSubLabelsByIndex(rag_t *rag, int index,
				int *subLabels, int n_subLabels)
{
  int label = rag_regionIndex2Label(rag,index);
  return (rag_addSubLabelsByLabel(rag, label, subLabels, n_subLabels));
}

int *rag_addSubLabelByLabel(rag_t *rag, int label, int subLabel)
{
  if (!rag || label <= 0 || label >= rag->n_regions) 
    return (NULL);

  region_addSubLabel(&rag->region[label], subLabel);
  
  return (rag->region[label].subLabels);
}

int *rag_addSubLabelByIndex(rag_t *rag, int index, int subLabel)
{
  int label = rag_regionIndex2Label(rag,index);
  return (rag_addSubLabelByLabel(rag, label, subLabel));
}

int rag_consistent(rag_t *rag, const char *s)
{
  int i;
  if (!s) s = "";

  for (i=1; i < rag->n_regions; i++) {
    if (rag->region[i].max_contour > 0 && 
	rag->region[i].n_contour > rag->region[i].max_contour) {
      fprintf(stderr, "rag_consistent:%s: MEMORY ERROR in region %d contour (%d <= %d failed)!\n",s, i, rag->region[i].n_contour, rag->region[i].max_contour);
      exit(1);
    }
    if (/*rag->region[i].n_contour > 0 && */ rag->region[i].pixelcount > 0 &&
	(rag->region[i].start_x == 0 || rag->region[i].start_y == 0)) {
      fprintf(stderr, "rag_consistent:%s: BOUNDARY ERROR in region %d start (%d,%d)!\n",s, i, rag->region[i].start_x, rag->region[i].start_y);
      exit(1);
    }
  }
  for (i=0; i < rag->n_adjacency_list; i++) {
    if ((rag->adjacency_list[i].region1_label > 0
	 && (rag->adjacency_list[i].region1->label 
	     != rag->adjacency_list[i].region1_label))
	|| (rag->adjacency_list[i].region2_label > 0
	    && (rag->adjacency_list[i].region2->label 
		!= rag->adjacency_list[i].region2_label))) {
      fprintf(stderr,"rag_consistent:%s: ADJACENCY ERROR in adj %d (%d-%d) points to (%d-%d)!\n", s, i,
	      rag->adjacency_list[i].region1_label,
	      rag->adjacency_list[i].region2_label,
	      rag->adjacency_list[i].region1->label,
	      rag->adjacency_list[i].region2->label);
      exit(1);
    }
  }
  return (1);
}


