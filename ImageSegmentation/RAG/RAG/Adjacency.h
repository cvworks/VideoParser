/*
 * RAGraph/Adjacency.h
 *
 * C++ class for RAG edges
 *
 * Sven Wachsmuth, 28.11.2002
 * Sven Wachsmuth, 1.5.2003 (totally re-organized)
 */
#ifndef _RAG_ADJACENCY_HH
#define _RAG_ADJACENCY_HH

//#include <LEDA/graphics/color.h>
//#include <LEDA/core/string.h>

#include <region/rag_mem.h>

class RAG_RefAdjacency {

  //** external reference on adjacency data
  int label;
  rag_t *rag_info;

  leda_string info_string;

  int contour1_pixelLength;
  int contour2_pixelLength;
  int total_pixelLength1;
  int total_pixelLength2;

public:
  RAG_RefAdjacency(void) { init(); }
  RAG_RefAdjacency(rag_t *rag, int _label=-1) { init(rag,_label); }
  virtual ~RAG_RefAdjacency() { }

  RAG_RefAdjacency& init(void);
  RAG_RefAdjacency& init(rag_t *rag, int _label=-1) { init(); return (set(rag,_label)); }

  RAG_RefAdjacency& initRegions(region_info_t *reg1, region_info_t *reg2);
  RAG_RefAdjacency& initValues(void);

  RAG_RefAdjacency& set(rag_t *rag, int _label=-1) {
    rag_info = rag; label = _label; initValues(); return (*this); }

  virtual const region_adjacency_t *get(void) const { 
    return rag_getAdjacencyByLabel(rag_info, label, NULL); }

  int   getLabel(void) const { return label; }
  int&  refLabel(void) { return label; }
  int&  refPixelLength1(void) { return (total_pixelLength1); }
  int&  refPixelLength2(void) { return (total_pixelLength2); }

  int   getPixelLength1(void) { return total_pixelLength1; }
  int   getPixelLength2(void) { return total_pixelLength2; }

  float getRelPixelLength1(void) { return (float) total_pixelLength1 /
				     (float) contour1_pixelLength; }
  float getRelPixelLength2(void) { return (float) total_pixelLength2 /
				     (float) contour2_pixelLength; }

  const region_info_t *getRegion1(void) const { 
    const region_adjacency_t *_info = get(); 
    return (_info) ? _info->region1:NULL; } 
  const region_info_t *getRegion2(void) const { 
    const region_adjacency_t *_info = get(); 
    return (_info) ? _info->region2:NULL; } 

  const leda_string& setInfoString(void);
  const leda_string& getInfoString(void) const { return info_string; }
  leda_string       *newInfoString(void) const;

  friend std::ostream& operator<<(std::ostream& s, const RAG_RefAdjacency& adj);
};

class RAG_Adjacency : public RAG_RefAdjacency {

  //** internal adjacency data
  region_adjacency_t *info;
  
  void _free(void) { if (info) region_adjacency_destroy(info); }
  void _init(void) { info = NULL; }

 public:

  RAG_Adjacency(void) { init(); }
  RAG_Adjacency(region_adjacency_t *adj) { init(adj); }
  RAG_Adjacency(region_adjacency_t &adj) { init(adj); }
  RAG_Adjacency(rag_t *rag, int _label=-1)
    :RAG_RefAdjacency(rag,_label) { _init(); }

  ~RAG_Adjacency(void) { _free(); }

  RAG_Adjacency& init(void) { 
    RAG_RefAdjacency::init(); _init(); return (*this); }

  RAG_Adjacency& init(region_adjacency_t *adj) { 
    init(); set(adj); return (*this); }
  RAG_Adjacency& init(region_adjacency_t &adj) { 
    init(); set(adj); return (*this); }

  RAG_Adjacency& set(region_adjacency_t *adj) {
    _free(); info = adj; initValues() ; return (*this); }

  RAG_Adjacency& set(region_adjacency_t &adj) {
    info = region_adjacency_cpy(info,&adj); initValues() ; return (*this); }

  RAG_Adjacency& set(const RAG_Adjacency &adj);

  // RAG_Adjacency& set(rag_t *rag, int _label=-1);

  const region_adjacency_t *get(void) const {
    return (info) ? info : RAG_RefAdjacency::get(); }

  RAG_Adjacency& operator=(const RAG_Adjacency& adj) { return set(adj); }

  friend std::istream& operator>>(std::istream& s, RAG_Adjacency& reg);
};

#endif
  
