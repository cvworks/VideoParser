/*
 * RAGraph/Region.h
 *
 * C++ class for RAG nodes
 *
 * Sven Wachsmuth, 28.11.2002
 * Sven Wachsmuth, 1.5.2003 (totally re-organized)
 */
#ifndef _RAG_REGION_HH
#define _RAG_REGION_HH

#include <LEDA/geo/point.h>
#include <LEDA/graphics/color.h>
#include <LEDA/graph/graph.h>
#include <region/rag_mem.h>
#include <pnmadd/pnmadd.h>
//#include "util.h"

class RAGraph;

class RAG_RefRegion {

protected:
  //** external reference on region data
  int    label;
  rag_t *rag_info;

  //** external reference on image data
  ppm_t         *image;

  //** region information in leda data-types
  leda_point     center;
  leda_color     pseudo_color;

  int   contour_width;
  int   contour_height;

  leda_string info_string;

public:
  int   external_index;
  float score;

  RAG_RefRegion(void) { init();} ;
  RAG_RefRegion(rag_t *rag, int _label=-1, ppm_t *_image=NULL) 
  { 
  	init(rag,_label,_image); 
  }

  RAG_RefRegion& init(void);
  
  RAG_RefRegion& init(rag_t *rag, int _label=-1, ppm_t *_image=NULL) 
  { 
	init(); return (set(rag, _label, _image)); 
  }
  
  virtual ~RAG_RefRegion() { }

  RAG_RefRegion& initValues(void);
  RAG_RefRegion& initInfoValues(void);
  RAG_RefRegion& initObjectValues(void);

  RAG_RefRegion& set(rag_t *rag, int _label=-1, ppm_t *_image=NULL) {
    rag_info = rag; label = _label; initValues(); image = _image; 
    return (*this); }

  virtual region_info_t *
    get(void) const { return rag_getRegionByLabel(rag_info, label, NULL); }

  void   setImage(ppm_t *_image) { image = _image; }
  ppm_t *getSubImage(ppm_t * subimage) const;
  char  *getContourPixmap(char *pixmap = NULL, int rows = 0, int cols = 0) const;
  pgm_t *getContourPGM(pgm_t *contour_pgm) const;
  int   *getSubLabels(int &n) const {
    region_info_t *info = get(); 
    if (info) n = info->n_subLabels;
    return (info) ? info->subLabels : NULL; }

  int               getLabel(void) const { return label; }
  const leda_point& getCenter(void) const { return center; }
  const leda_color& getPseudoColor(void) const { return pseudo_color; }

  const contour_point_t *getContour(int *n_contour=NULL) const { 
    region_info_t *info = get(); 
    if (n_contour) *n_contour = (info) ? info->n_contour : 0;
    return (info) ? info->contour : NULL; }
  const int getNContour(void) const 
    { region_info_t *info = get(); return (info) ? info->n_contour : 0; }

  int             getNumOfPixels(void) const
    { region_info_t *info = get(); return (info) ? info->pixelcount : 0; }

  int             getMinX(void) const 
    { region_info_t *info = get(); return (info) ? info->min_x : -1; }
  int             getMaxX(void) const 
    { region_info_t *info = get(); return (info) ? info->max_x : -1; }
  int             getMinY(void) const 
    { region_info_t *info = get(); return (info) ? info->min_y : -1; }
  int             getMaxY(void) const 
    { region_info_t *info = get(); return (info) ? info->max_y : -1; }

  int getContourPixmapCols(void) const { return contour_width+2; }
  int getContourPixmapRows(void) const { return contour_height+2;}
  int getWidth(void) const             { return contour_width; }
  int getHeight(void) const            { return contour_height; }

  const leda_string& setInfoString(void);
  const leda_string& getInfoString(void) const { return (info_string); }
  leda_string       *newInfoString(void) const;
  leda_list<leda_point>  *newContourList(void) const;
  leda_list<int>         *newSubLabelList(void) const;

  void setContour(leda_list<leda_point>& contourList);
  void setSubLabels(leda_list<int>& subLabels);

  int  merge_size() {
    region_info_t *info = get();
    return (info) ? (info->n_subLabels + 1) / 2 : 0; }

  friend std::ostream& operator<<(std::ostream& s, const RAG_RefRegion& reg);
};

class RAG_Region : public RAG_RefRegion {

  //** internal region data
  region_info_t *region_info;
  
  void _free(void) { if (region_info) region_info_destroy(region_info); }
  void _init(void) { region_info = NULL; }

 public:
  
  RAG_Region(void) 
  { 
  	_init(); 
  }
  
  RAG_Region(region_info_t *info, ppm_t *_image=NULL) 
  { 
  	init(info, _image); 
  }
  
  RAG_Region(region_info_t &info, ppm_t *_image=NULL) 
  { 
  	init(info, _image); 
  }
		 
  RAG_Region(rag_t *rag, int _label=-1, ppm_t *_image=NULL):RAG_RefRegion(rag,_label,_image) 
  { 
  	_init(); 
  }

  virtual ~RAG_Region(void) { _free(); }

  RAG_Region& init(void) 
  { 
  	RAG_RefRegion::init(); 
	_init(); 
	return *this;
  }

  RAG_Region& init(region_info_t *info, ppm_t *_image=NULL) 
  { 
  	init(); 
	return (set(info, _image)); 
  }
  
  RAG_Region& init(region_info_t &info,  ppm_t *_image=NULL) 
  { 
  	init(); 
	return (set(info, _image)); 
  }

  RAG_Region& set(region_info_t *info, ppm_t *_image=NULL) 
  { 
    _free(); 
	region_info = info; 
	initValues(); 
	setImage(_image); 
    return (*this); 
  }
  
  RAG_Region& set(region_info_t &info, ppm_t *_image=NULL) 
  { 
    region_info = region_info_cpy(region_info,&info); 
	initValues();
    setImage(_image); 
	return (*this); 
  }
	
  RAG_Region& set(const RAG_Region& r);

  // RAG_Region& set(rag_t *rag, int _label=-1, ppm_t *_image=NULL);
  // void setContour(leda_list<leda_point>& contourList);
  // void setSubLabels(leda_list<int>& subLabels);

  region_info_t * get(void) const 
  { 
  	return ((region_info) ? region_info : RAG_RefRegion::get()); 
  }

  std::istream& readInfo(std::istream& s);

  RAG_Region& operator=(const RAG_Region& r) { return set(r); }

  friend std::istream& operator>>(std::istream& s, RAG_Region& reg);
};

#endif
  
