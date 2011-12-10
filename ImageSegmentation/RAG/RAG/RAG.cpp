/*
 * RAG.C
 *
 * C++ interface for rag_t
 *
 * Sven Wachsmuth, 11.07.2003
 */
#include <pnmadd/pseudo.h>
#include "RAG.h"

int RAG::offset = 1;

RAG& RAG::setMap(int *regionMap, 
	      int max_regionMap, 
	      int cols, int rows, 
	      int n_regions) 
{
  int i;
  rag_setRegionMap(&rag,regionMap,max_regionMap,cols,rows);

  //** make sure that marginal pixels have no region label
  for (i=0; i < offset; i++)
    pnm_setIntFrame(rag.regionMap, rag.cols, rag.rows,i,-1);

  rag_computeFromRegionMap(&rag,n_regions); 
  return (*this); 
};

RAG& RAG::setMap(unsigned char *colorMap,
	      int max_colorMap,
	      int cols, int rows,
	      int n_colors, rag_color_t *colorTab/*=NULL*/, int max_colorTab/*=-1*/) 
{
  int i,n_regions;
  rag_setColorMap(&rag, colorMap, max_colorMap, cols, rows);
  rag_setColorTab(&rag, colorTab, max_colorTab, n_colors);
  n_regions = rag_computeRegionMap(&rag);

  //** make sure that marginal pixels have no region label
  for (i=0; i < offset; i++)
    pnm_setIntFrame(rag.regionMap, rag.cols, rag.rows,i,-1);

  rag_computeFromRegionMap(&rag,n_regions);
 
  return (*this); 
};

RAG& RAG::setMap(unsigned char *colorMap,
	      int max_colorMap,
	      int cols, int rows,
	      int n_colors, pixel *colorTab/*=NULL*/, int max_colorTab/*=-1*/) 
{
  int i,n_regions;
  rag_setColorMap(&rag, colorMap, max_colorMap, cols, rows);
  if (colorTab)
      rag_setColorTabByPixel(&rag, colorTab, n_colors);

  n_regions = rag_computeRegionMap(&rag);

  //** make sure that marginal pixels have no region label
  for (i=0; i < offset; i++)
    pnm_setIntFrame(rag.regionMap, rag.cols, rag.rows,i,-1);

  rag_computeFromRegionMap(&rag,n_regions);
 
  return (*this); 
};

