/*
 * region/rag_gen.c
 *
 * Sven Wachsmuth, 10.04.2003
 *
 * computation of regions, adjacencies, and merges
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rag_gen.h"
#include "_mem.h"

int rag_computeRegionMap(rag_t *rag)
{
  int n_regions;
  if (!rag) return (0);

  rag->regionMap = (int *)
    _rag_realloc_memory_space(rag->regionMap, &rag->max_regionMap,
			      rag->cols * rag->rows, sizeof(int),
			      NULL, NULL, NULL);
  memset(rag->regionMap, 0, rag->cols * rag->rows * sizeof(int));
  
  n_regions = regionlab(rag->cols, rag->rows, rag->colorMap, rag->n_colors,
			rag->regionMap);

  return (n_regions);
}

int rag_computeContourIndexMap(rag_t *rag)
{
  int i;
  if (!rag) return (0);

  rag->contourIndexMap = (int *)
    _rag_realloc_memory_space(rag->contourIndexMap, &rag->max_contourIndexMap,
			      rag->cols * rag->rows * 4, sizeof(int),
			      NULL, NULL, NULL);
  memset(rag->contourIndexMap, 0, rag->cols * rag->rows * 4 * sizeof(int));

  RAG_FORALL_REGIONS(rag,i) {
    region_info_t *region = RAG_GET_REGION(rag,i);

    region_contour2indexMap(rag->contourIndexMap, rag->cols,
			    region->n_contour,
			    region->contour);
  }
  return (rag->n_regions);
}

int  rag_computeRegions(rag_t *rag, int n_regions)
{
  int i;
  region_info_t **regions;

  if (!rag) return (0);

  /** ensure that marginal pixels have no region label */
  if (rag->regionMap[0] >= 0)
    pnm_setIntMargin(rag->regionMap, rag->cols, rag->rows, -1);

  
  rag_realloc(rag, n_regions, 0);
  rag_clearRegions(rag);
  regions = rag_newTmpRegions(rag,0,1,n_regions-1);

  /** look for starting points */
  region_countpixel(regions, 
		    rag->regionMap, rag->cols, rag->rows, 
		    rag->colorMap);

  rag_reallocContourPoints(rag, rag->cols * rag->rows);

  /** follow contour */
  for (i=1; i < rag->n_regions; i++) {
    if (regions[i]->pixelcount > 0) {
      region_contour(regions[i], rag->regionMap, rag->cols, rag->rows,
		     i, rag->contour_points+rag->n_contour_points);
      rag->n_contour_points += rag->region[i].n_contour;
      
      rag_setSubLabelByLabel(rag, i);
    } else {
      /* if the region pixels are eliminated by frame-operation
	 (pnm_setIntFrame) then no pixels are found anymore and
	 the region is deleted */
      rag_delRegionByLabel(rag,regions[i]->label);
    }
  }
  return (rag->n_contour_points);
}

int  rag_computeAdjacencies(rag_t *rag,
			    int region_label,
			    region_adjacency_t **adj /* indexed by region */
			    )
{
  int j;
  region_info_t **regions;
  int             n_regions;

  if (!rag || !adj || region_label <= 0 || region_label >= rag->n_regions) 
    return (0);

  regions = rag_getTmpRegions(rag, 0);

  /** compute adjacencies */
  region_adjacency_set(adj, regions, region_label, rag->n_regions, 
		       rag->regionMap, rag->cols);

  for (j=1; j < rag->n_regions; j++)
    region_adjacency_update(adj[j], rag->contourIndexMap, rag->cols);

  return (rag->n_regions);
}

int rag_computeFromColorMap(rag_t *rag)
{
  int n_regions;

  if (!rag || !rag->colorMap || rag->cols <= 0 || rag->rows <= 0)
    return (0);

  n_regions = rag_computeRegionMap(rag);

  return (rag_computeFromRegionMap(rag, n_regions));
}

int rag_computeFromRegionMap(rag_t *rag, int n_regions)
{
  if (!rag || !rag->regionMap || rag->cols <= 0 || rag->rows <= 0
      || n_regions <= 1)
    return (0);

  rag_computeRegions(rag, n_regions);

  return (rag_computeFromRegions(rag));
}

int rag_computeFromRegions(rag_t *rag)
{
  int i;

  if (!rag || !rag->regionMap || rag->cols <= 0 || rag->rows <= 0
      || rag->n_regions <= 1)
    return (0);

  rag_computeContourIndexMap(rag);
  
  RAG_FORALL_REGIONS(rag,i) {
    int j;
    region_info_t *region = RAG_GET_REGION(rag,i);
    int label = region->label;

    region_adjacency_t **adjs = rag_newTmpAdjacencies(rag,0,label);
    
    rag_computeAdjacencies(rag, label, adjs);

    rag_clearUnusedAdjacencies(rag, adjs, rag->n_regions);
  }
  return (rag->n_regions);
}

int  rag_computeMerge(rag_t *rag, int label1, int label2)
{
  int indexM, labelM;
  region_info_t *merged_region;
  region_adjacency_t **adjacencies1;
  region_adjacency_t **adjacencies2;
  region_adjacency_t **adjacenciesM;
  region_adjacency_t **rev_adjacenciesM;

  if (!(merged_region = rag_newRegion(rag, &indexM)))
    return (-1);

  labelM = merged_region->label;

  adjacenciesM = rag_newTmpAdjacencies(rag,0,labelM);
  rev_adjacenciesM = rag_newTmpRevAdjacencies(rag,1,labelM);

  adjacencies1 = rag_getTmpAdjacencies(rag, 2, label1);
  adjacencies2 = rag_getTmpAdjacencies(rag, 3, label2);

  rag_computeMergeOutEdges(merged_region, 
			   adjacenciesM, rev_adjacenciesM,
			   rag, label1, label2,
			   adjacencies1, adjacencies2);

  adjacencies1 = rag_getTmpRevAdjacencies(rag, 2, label1);
  rag_computeMergeInEdges(adjacenciesM, rev_adjacenciesM,
			  rag, labelM, label1, label2,
			  adjacencies1);

  adjacencies2 = rag_getTmpRevAdjacencies(rag, 3, label2);
  rag_computeMergeInEdges(adjacenciesM, rev_adjacenciesM,
			  rag, labelM, label2, label1,
			  adjacencies2);

  rag_clearUnusedAdjacencies(rag, adjacenciesM, rag->n_regions);
  rag_clearUnusedAdjacencies(rag, rev_adjacenciesM, rag->n_regions);

  return (indexM);
}
  
int  rag_computeMerge2(rag_t *rag, int label1, int label2)
{
  int indexM, labelM;
  region_info_t *merged_region;
  region_info_t      **regions;
  region_adjacency_t **adjacencies1;
  region_adjacency_t **adjacencies2;
  region_adjacency_t **adjacenciesM;
  region_adjacency_t **rev_adjacenciesM;

  if (!(merged_region = rag_newRegion(rag, &indexM)))
    return (-1);

  labelM = merged_region->label;

  adjacenciesM = rag_newTmpAdjacencies(rag,0,labelM);
  rev_adjacenciesM = rag_newTmpRevAdjacencies(rag,1,labelM);

  adjacencies1 = rag_getTmpAdjacencies(rag, 2, label1);
  adjacencies2 = rag_getTmpAdjacencies(rag, 3, label2);

  regions = rag_getTmpRegions(rag, 0);

  if (!rag->contourIndexMap)
    rag_computeContourIndexMap(rag);

  region_mergeOutEdges2(merged_region,
			regions, rag->n_regions,
			adjacenciesM,
			adjacencies1[label2],
			adjacencies2[label1],
			labelM, labelM,
			NULL, NULL,
			rag->regionMap,
			rag->contourIndexMap,
			rag->cols, rag->rows);

  adjacencies1 = rag_getTmpRevAdjacencies(rag, 2, label1);
  region_mergeInEdges2(rev_adjacenciesM,
		       regions, rag->n_regions,
		       labelM, label1, label2,
		       adjacencies1);
  adjacencies2 = rag_getTmpRevAdjacencies(rag, 3, label2);
  region_mergeInEdges2(rev_adjacenciesM,
		       regions, rag->n_regions,
		       labelM, label2, label1,
		       adjacencies2);

  rag_clearUnusedAdjacencies(rag, adjacenciesM, rag->n_regions);
  rag_clearUnusedAdjacencies(rag, rev_adjacenciesM, rag->n_regions);

  return (indexM);
}
  

region_info_t *  
rag_computeMergeOutEdges(region_info_t *merged_region,
			 region_adjacency_t **adjacenciesM,
			 region_adjacency_t **rev_adjacenciesM,
			 rag_t *rag, int label1, int label2,
			 region_adjacency_t **adjacencies1,
			 region_adjacency_t **adjacencies2)
{
  region_adjacency_t *adj;
  region_adjacency_t *rev_adj;
  int label;

  if (!rag || !merged_region) return (merged_region);

  label = merged_region->label;

  adj = adjacencies1[label2];     /*rag_getAdjacency(rag,label1,label2),*/
  rev_adj = adjacencies2[label1]; /*rag_getAdjacency(rag,label2,label1),*/

  /** the merged region will have no adjacencies its parts */
  adjacencies1[label2] = NULL;
  adjacencies2[label1] = NULL;

  /** compute merge */
  region_mergePt(merged_region, 
		 adjacenciesM, 
		 rev_adjacenciesM,
		 adj, rev_adj, 
		 adjacencies1,
		 adjacencies2,
		 rag->n_regions,
		 label, label,
		 NULL, NULL);

  adjacencies1[label2] = adj;
  adjacencies2[label1] = rev_adj;
  {
    int n_subLabels;
    int *subLabels = rag_getSubLabelsByLabel(rag,label1, &n_subLabels);
    rag_addSubLabelsByLabel(rag, label, subLabels, n_subLabels);
    subLabels = rag_getSubLabelsByLabel(rag, label2, &n_subLabels);
    rag_addSubLabelsByLabel(rag, label, subLabels, n_subLabels);
  }

  return (merged_region);
}

int rag_computeMergeInEdges(region_adjacency_t **adjacenciesM,
			    region_adjacency_t **rev_adjacenciesM,
			    rag_t *rag, 
			    int merged_label, int label1, int label2,
			    region_adjacency_t **rev_adjacencies1)
{
  int i,j;
  region_info_t *region2 = rag_getRegionByLabel(rag, label2, NULL);

  if (!rag) return (-1);

  for (i=0; i < rag->n_regions; i++) {
    region_info_t *regionI;

    region_adjacency_t *adj = rev_adjacencies1[i];
    if (!region_isAdjacent(adj)
	|| region_isAdjacent(adjacenciesM[i])) continue;

    regionI = rag_getRegionByLabel(rag, i, NULL);

    if (!region_areDisjunct(region2, regionI))
      continue;

    /** ... add sections from i to label1 ... */
    for (j=0; j < adj->n_sections; j++) {
      region_adjacency_cpySec1(rev_adjacenciesM[i], &adj->sections[j]);
    }
  }
  return (0);
}

int rag_setMergeColor(region_info_t *infoM, rag_t *rag,
		      region_info_t *info1, region_info_t *info2)
{
  if (!infoM) return (-1);
  if (!info1 && !info2)
      return (infoM->color = -1);
  if (!info1)
      return (infoM->color = info2->color);
  if (!info2)
      return (infoM->color = info1->color);
  if (rag->colorTab) {
      rag_color_t *cM = rag_newColor(rag, &infoM->color);
      rag_color_t *c1 = &rag->colorTab[info1->color-1];
      rag_color_t *c2 = &rag->colorTab[info2->color-1];
      float weight1 = ((float) info1->pixelcount 
		       / (float) (info1->pixelcount + info2->pixelcount));
      float weight2 = ((float) info2->pixelcount 
		       / (float) (info1->pixelcount + info2->pixelcount));
      cM->r = weight1 * c1->r + weight2 * c2->r;
      cM->g = weight1 * c1->g + weight2 * c2->g;
      cM->b = weight1 * c1->b + weight2 * c2->b;
      if (infoM->color == 16) {
	  fprintf(stderr,"rag_setMergeColor: color %d = (%g,%g,%g)\n",
		  info1->color, c1->r, c1->g, c1->b);
	  fprintf(stderr,"rag_setMergeColor: color %d = (%g,%g,%g)\n",
		  info2->color, c2->r, c2->g, c2->b);
	  fprintf(stderr,"rag_setMergeColor: new merge color %d = (%g,%g,%g)\n",
		  infoM->color, cM->r, cM->g, cM->b);
      }
  }
  return (infoM->color);
}
 
void rag_fprint_colorTab(FILE *fp, rag_t *rag)
{
    int i;
    for (i=0; i < rag->n_colors; i++) {
	fprintf(fp, "colorTab[%d] = (%g,%g,%g)\n",
		i,rag->colorTab[i].r,rag->colorTab[i].g,rag->colorTab[i].b);
    }
}

int rag_regionIsMerged(rag_t *rag, int label)
{
  region_info_t *reg = rag_getRegionByLabel(rag, label, NULL);
  return (reg->n_subLabels > 1);
}

int rag_isSuperSetOf(rag_t *rag, region_info_t *region1,
		     region_info_t *region2)
{
  int i,j,equal_labels=0;
  int n_merge1 = (region1->n_subLabels + 1) / 2;
  int n_merge2 = (region2->n_subLabels + 1) / 2;

  if (!region1 &&  region2) return 0;
  if (!region1 || !region2) return 1;
  if (!rag) return (0);

  for (i=0; i < region1->n_subLabels; i++)
    if (!rag_regionIsMerged(rag, region1->subLabels[i]))
      for (j=0; j < region2->n_subLabels; j++)
	if (region1->subLabels[i] == region2->subLabels[j])
	  equal_labels++;
  
  /*fprintf(stderr,"rag_isSuperSetOf: %d #1/%d #2/%d\n", equal_labels, n_merge1,
   *  n_merge2);
   */
  return ((equal_labels == n_merge2) ? ( 1) :
	  (equal_labels == n_merge1) ? (-1) : (0)); 

}  


