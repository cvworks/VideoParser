/*
 * region/rag_gen.h
 *
 * Sven Wachsmuth, 10.04.2003
 *
 * computation of regions, adjacencies, and merges
 */
#ifndef _REGION_RAGGEN_H
#define _REGION_RAGGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rag_mem.h"

int  rag_computeRegionMap(rag_t *rag);
int  rag_computeRegions(rag_t *rag, int n_regions);
int  rag_computeAdjacencies(rag_t *rag, int region_label,
			    region_adjacency_t **adj);
int rag_computeContourIndexMap(rag_t *rag);

int rag_computeFromColorMap(rag_t *rag);
int rag_computeFromRegionMap(rag_t *rag, int n_regions);
int rag_computeFromRegions(rag_t *rag);

int  rag_computeMerge(rag_t *rag, int label1, int label2);
region_info_t *
rag_computeMergeOutEdges(region_info_t *merged_region,
			 region_adjacency_t **adjacenciesM,
			 region_adjacency_t **rev_adjacenciesM,
			 rag_t *rag, int label1, int label2,
			 region_adjacency_t **adjacencies1,
			 region_adjacency_t **adjacencies2);

int  rag_computeMergeInEdges(region_adjacency_t **adjacenciesM,
			     region_adjacency_t **rev_adjacenciesM,
			     rag_t *rag, 
			     int merged_label, int label1, int label2,
			     region_adjacency_t **rev_adjacencies1);

int  rag_computeMerge2(rag_t *rag, int label1, int label2);

int rag_setMergeColor(region_info_t *infoM, rag_t *rag,
		      region_info_t *info1, region_info_t *info2);

int rag_regionIsMerged(rag_t *rag, int label);

int rag_isSuperSetOf(rag_t *rag, region_info_t *region1,
		     region_info_t *region2);

void rag_fprint_colorTab(FILE *fp, rag_t *rag);

#ifdef __cplusplus
}
#endif

#endif




