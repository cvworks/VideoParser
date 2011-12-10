/*
 * RAG.h
 *
 * C++ interface for rag_t
 *
 * Sven Wachsmuth, 3.12.2002
 * Sven Wachsmuth, 11.7.2003 (offset parameter included, code restructuring)
 */
#ifndef _RAG_RAG_HH
#define _RAG_RAG_HH

#include <iostream>

#include <region/rag_mem.h>
#include <region/rag_gen.h>

class RAG {
  rag_t rag;

 public:

  static int offset;

  RAG(void) { init(); };
  RAG(rag_t *_rag) { init(_rag); };
  RAG(int *regionMap, 
      int max_regionMap, 
      int cols, int rows, 
      int n_regions) 
    { init(regionMap,max_regionMap,cols,rows,n_regions); };
  RAG(unsigned char *colorMap,
      int max_colorMap,
      int cols, int rows,
      int n_colors, rag_color_t *colorTab=NULL, int max_colorTab=-1)
    { initColorTab(colorMap, max_colorMap, cols, rows,
		   n_colors,colorTab,max_colorTab); };
  RAG(unsigned char *colorMap,
      int max_colorMap,
      int cols, int rows,
      int n_colors, pixel *colorTab=NULL, int max_colorTab=-1)
    { initPixelTab(colorMap, max_colorMap, cols, rows,
		   n_colors,colorTab,max_colorTab); };

  ~RAG(void) { rag_free(&rag); }

  RAG& init(void) { rag_init(&rag); return *this; };
  RAG& init(rag_t *_rag) { init(); return setRAG(_rag); };
  RAG& init(int *regionMap, 
	    int max_regionMap, 
	    int cols, int rows, 
	    int n_regions)
    { init(); return setMap(regionMap,max_regionMap,cols,rows,n_regions); };
  RAG& initColorTab(unsigned char *colorMap,
		    int max_colorMap, 
		    int cols, int rows,
		    int n_colors, rag_color_t *colorTab=NULL, 
		    int max_colorTab=-1)
    { init(); return setMap(colorMap,max_colorMap,cols,rows,n_colors,
			 colorTab,max_colorTab); };
  RAG& initPixelTab(unsigned char *colorMap,
		    int max_colorMap,
		    int cols, int rows,
		    int n_colors, pixel *colorTab=NULL, int max_colorTab=-1)
    { init(); return setMap(colorMap,max_colorMap,cols,rows,n_colors,
			 colorTab,max_colorTab); };

  RAG& setRAG(const RAG& _RAG) { rag_cpy(&rag,_RAG.get()); return (*this); };
  RAG& setRAG(rag_t *_rag) { rag_cpy(&rag,_rag); return (*this); };
  RAG& setMap(int *regionMap, 
	   int max_regionMap, 
	   int cols, int rows, 
	   int n_regions);
  RAG& setMap(unsigned char *colorMap,
	   int max_colorMap,
	   int cols, int rows,
	   int n_colors, rag_color_t *colorTab=NULL, int max_colorTab=-1);
  RAG& setMap(unsigned char *colorMap,
	   int max_colorMap,
	   int cols, int rows,
	   int n_colors, pixel *colorTab=NULL, int max_colorTab=-1);
  
  const rag_t *get(void) const { return &rag; };

  RAG& operator=(const RAG& _RAG) { return setRAG(_RAG); }

  ppm_t *setImage(ppm_t *image) { return rag_setImage(&rag,image); };
  ppm_t *getImage(void) const { return rag.image; };

};

#endif
