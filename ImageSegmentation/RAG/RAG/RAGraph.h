/*
* RAGraph.h
*
* C++ class for Region-Adjacency-Graph
*
* Sven Wachsmuth, 3.12.2002
* Sven Wachsmuth,  1.5.2003, V0.4 (totally re-organized)
* Sven Wachsmuth,  6.5.2003, V0.4.1 (updated read and write for RAGraph)
* Sven Wachsmuth,  7.5.2003, V0.4.1b (version variable added)
* Sven Wachsmuth, 11.5.2003, V0.4.1c (RAG::offset included)
* Sven Wachsmuth, 28.7.2003, V0.4.1d (RAGraph::getNodeByExtIndex,
*                                     and png-writing of contours added)
*/
#ifndef _RAG_RAGRAPH_H
#define _RAG_RAGRAPH_H

#include <LEDA/graph/graph.h>
#include "RAG.h"
#include "Region.h"
#include "Adjacency.h"

class RAGraph: public leda::GRAPH<RAG_Region,RAG_Adjacency> {

	static RAG mRAG;

	RAG *pRAG;

	int ignore_color;
	int ignore_threshold;

	char *contour_format;

	void initGraph(int _ignore_color=-1, 
		int _ignore_threshold=-1);

public:
	static const char *version;
	static int         skip_colorMap;

	static std::ostream& write_options(std::ostream& s, const char *prefix=NULL);

	RAGraph(void) { init(); }
	//RAGraph(RAGraph& _RAG) { init(_RAG); }
	RAGraph(RAG *_pRAG,
		int _ignore_color=-1,
		int _ignore_threshold=-1) 
	{ init(_pRAG, _ignore_color, _ignore_threshold); }

	~RAGraph(void) {}

	virtual std::ostream& info(std::ostream &s) 
	{ 
		s << "class RAGraph" << std::endl; 
		return s;
	}

	char *setContourFormat(char *format) 
	{ 
		char *tmp = contour_format; 
		contour_format = format; 
		return (tmp); 
	}

	char *getContourFormat(void) const 
	{ 
		return contour_format; 
	} 

	unsigned char * setMergedColorMap(unsigned char *map);

	RAGraph& init(void);

	//RAGraph& init(const RAGraph& _RAG) { init(); return setRAG(_RAG); }
	RAGraph& init(RAG *_pRAG,
		int _ignore_color=-1,
		int _ignore_threshold=-1)
	{ init(); return setRAG(_pRAG, _ignore_color, _ignore_threshold); }

	//RAGraph& setRAG(const RAGraph& _RAG);
	RAGraph& setRAG(RAG *_pRAG,
		int _ignore_color=-1,
		int _ignore_threshold=-1) { 
			pRAG = _pRAG; initGraph(_ignore_color,_ignore_threshold);
			return (*this); }

	RAGraph& update(int _ignore_color=-1,
		int _ignore_threshold=-1) {
			initGraph(_ignore_color,_ignore_threshold);
			return (*this); }

	RAG *get(void) const { return pRAG; }

	RAGraph& updateAdjacencies(void);
	RAGraph& updateRAG(RAG *_pRAG=NULL);

	//RAGraph& operator=(const RAGraph& _RAG) { return setRAG(_RAG); }

	int  getIgnoreColor(void);
	int *getMarginalCount(int *count, int &n_count, int &imax); 

	void writeContour(leda_node r, const char *format=NULL);
	void writeContours(const char *format=NULL);

	void writeMergedColorMap(const char *filename);

	int  readColorMap(const char *filename);
	void freeMaps(void) { 
		rag_setColorMap((rag_t *) pRAG->get(),NULL,0, 
			pRAG->get()->cols, pRAG->get()->rows);
		rag_setRegionMap((rag_t *) pRAG->get(),NULL,0, 
			pRAG->get()->cols, pRAG->get()->rows);
		rag_setContourIndexMap((rag_t *) pRAG->get(),NULL,0,
			pRAG->get()->cols,pRAG->get()->rows);
	}

	int getCols(void) const 
	{ return (pRAG && pRAG->get()) ? pRAG->get()->cols : 0; }
	int getRows(void) const 
	{ return (pRAG && pRAG->get()) ? pRAG->get()->rows : 0; }
	int getMinX(leda_node r) const
	{ return (r != nil) ? (*this)[r].get()->min_x : -1; }
	int getMaxX(leda_node r) const
	{ return (r != nil) ? (*this)[r].get()->max_x : -1; }
	int getMinY(leda_node r) const
	{ return (r != nil) ? (*this)[r].get()->min_y : -1; }
	int getMaxY(leda_node r) const
	{ return (r != nil) ? (*this)[r].get()->max_y : -1; }
	int getCenterX(leda_node r) const
	{ return (r != nil) ? (int) (*this)[r].get()->center_x : -1; }
	int getCenterY(leda_node r) const
	{ return (r != nil) ? (int) (*this)[r].get()->center_y : -1; } 

	int *getRegionMap(void) const 
	{ return (pRAG && pRAG->get()) ? pRAG->get()->regionMap : NULL; }
	unsigned char *getColorMap(void) const 
	{ return (pRAG && pRAG->get()) ? pRAG->get()->colorMap : NULL; }
	rag_color_t   *getColorTab(int *n_colors=NULL) const { 
		const rag_t *p_rag = (pRAG) ? pRAG->get() : NULL;
		if (p_rag && n_colors) (*n_colors) = p_rag->n_colors;
		return (p_rag) ? p_rag->colorTab : NULL; }

	int getMaxLabel(void) const; 

	region_info_t *getRegion(int label) const 
	{ if (pRAG) return rag_getRegionByLabel((rag_t *)pRAG->get(),label,NULL);
	const leda_node r = getNode(label);
	return (r != nil) ? (*this)[r].get() : NULL; }

	ppm_t *getImage(void) const { return (pRAG) ? pRAG->getImage() : NULL; }

	region_adjacency_t **getTmpAdjacencyMem(int tmp_index) const;
	region_adjacency_t **getAdjacencies(region_adjacency_t **adjs,
		int n_adjs,
		leda_node r,
		leda_node s=nil) const;
	region_adjacency_t **getRevAdjacencies(region_adjacency_t **adjs,
		int n_adjs,
		leda_node r,
		leda_node t=nil) const;

	int areDisjunct(const leda_node& r1, const leda_node& r2) const {
		region_info_t *reg1 = (*this)[r1].get();
		region_info_t *reg2 = (*this)[r2].get();
		return region_areDisjunct(reg1, reg2);
	}

	const int *getSubLabels(const leda_node& r, int &n_subLabels) const {
		if (!pRAG) { n_subLabels=0; return (NULL); }
		return rag_getSubLabelsByLabel((rag_t *)pRAG->get(),(*this)[r].getLabel(),
			&n_subLabels);
	}

	int isSubLabel(const leda_node& r, int subLabel) {
		int i,n_subLabels;
		const int *subLabels = getSubLabels(r, n_subLabels);
		if (!subLabels) { return (0); }
		for (i=0; i < n_subLabels && subLabels[i] != subLabel; i++);
		return (i < n_subLabels); }

	int isSuperSetOf(const leda_node &r1, const leda_node &r2) {
		region_info_t *reg1 = (*this)[r1].get();
		region_info_t *reg2 = (*this)[r2].get();
		const rag_t *p_rag = (get()) ? get()->get() : NULL;
		return (rag_isSuperSetOf((rag_t *) p_rag, reg1, reg2)); }

	const leda_node getNode(int label) const;
	const leda_node getNodeByExtIndex(int index) const;
	virtual leda_node merge_OLD(const leda_node& r1, const leda_node& r2);
	virtual leda_node merge(const leda_node& r1, const leda_node& r2);

	void computeMerges(int depth);
	void _computeMerges(int depth, leda_node r, int max_index);

	RAGraph& hide_merges();
	RAGraph& restore_merges();

	void computeMoments(void);
	void align(double &scale, 
		double &offset_x, double &offset_y,
		leda_node r, region_info_t *match);

	// initializes mask with value 0 and sets mask with value 1
	void getRegionMask(unsigned char *mask, int mask_cols, int mask_rows,
		double scale, double offset_x, double offset_y,
		leda_node r);

	// sets region pixels in mask with value <value>
	void setRegionMask(unsigned char *mask, int mask_cols, int mask_rows,
		double scale, double offset_x, double offset_y,
		leda_node r, int value);
	void setRegionMask(int *mask, int mask_cols, int mask_rows,
		double scale, double offset_x, double offset_y,
		leda_node r, int value);

	void writeRegionMask(leda_node r, const char *filename);

	int consistent(const char *text="");

	void print(std::ostream& s=std::cout) { write(s); }
	//void print(leda_string s, ostream& O=cout) { }
	void write(std::ostream& s=std::cout);
	//void write(leda_string s) { ofstream O.open(s); write(O); O.close(); }
	virtual int read(std::istream& I=std::cin);

	friend std::ostream& operator<<(std::ostream& s, RAGraph& gRAG);
	friend std::istream& operator>>(std::istream& s, RAGraph& gRAG);
};

#endif


