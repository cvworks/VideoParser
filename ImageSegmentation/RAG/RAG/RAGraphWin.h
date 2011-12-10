/*
 * RAGraphWin.h
 *
 * C++ class for Region-Adjacency-Graph-Window
 *
 * Sven Wachsmuth, 3.12.2002, V0.1
 *
 * Sven Wachsmuth, 19.3.2003, V0.2
 * - zooming of background and graph added
 *
 */
#ifndef _RAGRAPHWIN_H
#define _RAGRAPHWIN_H

#include <LEDA/graphics/graphwin.h>

#include "RAGraph.h"

//class RAGraphWin;

class RAGraphWin : public leda::GraphWin {

  //** position and size of image in background xpm
  int image_width;
  int image_height;
  int image_woffset;
  int image_hoffset;
  float image_factor;
  int   image_scale;
  
  float w_scale;

  //** position and height of background xpm
  int bg_woffset;
  int bg_hoffset;
  int bg_height;

  //** background xpm
  char *bg_pixmap;

  bool m_bShowNodes;

public:

  leda_window *info_window;
  leda::gw_action info_action;

  RAGraphWin(RAGraph& RAG, 
	     int image_w, 
	     int image_h, 
	     int image_x, 
	     int image_y, 
	     float image_f,
	     int bg_h, 
	     int bg_x, 
	     int bg_y, 
	     const char **bg_xpm,
	     int w, 
	     int h, 
	     const char *win_label="");

  RAGraphWin(RAGraph& RAG,
	     const char *win_label,
	     int *regionMap,
	     int image_w,
	     int image_h,
	     int image_s=1,
	     unsigned char *colorMap=NULL,
	     ppm_t *image=NULL);

  RAGraphWin(RAGraph& RAG,
	     const char *win_label="",
	     int image_s=1);

  RAGraphWin(void) { init(); }
  ~RAGraphWin(void) {};

  void init(void);
  void initWindow(void);

  void initImage(int image_w, 
		 int image_h, 
		 int image_x, 
		 int image_y, 
		 float image_f,
		 int bg_h, 
		 int bg_x, 
		 int bg_y, 
		 const char **bg_xpm);

  void initImage(int *regionMap,
		 int image_w,
		 int image_h,
		 int image_s=1,
		 unsigned char *colorMap=NULL,
		 ppm_t *image=NULL);

  void updateImages(int height);

  char       *getBgPixmap(void) { return bg_pixmap; };
  const int   getBgWOffset(void) const { return bg_woffset; };
  const int   getBgHOffset(void) const { return bg_hoffset; };
  const int   getBgHeight(void) const { return bg_height; };
  const int   getImgWOffset(void) const { return image_woffset; };
  const int   getImgHOffset(void) const { return image_hoffset; };
  const float getImgFactor(void) const { return image_factor; };
  
  bool getShowNodes() const { return m_bShowNodes; }
  void setShowNodes(bool bShowNodes) { m_bShowNodes = bShowNodes; }

  friend void set_nodePositions(RAGraphWin& w);
  friend void set_nodePosition(RAGraphWin& w, leda_node r);
  friend void RAG_mergeSelectedNodes(RAGraphWin& w);
  
};      

void set_nodeInfoMessage(RAGraphWin&gw, const leda_point &p);
void set_edgeInfoMessage(RAGraphWin&gw, const leda_point &p);
void open_infoWindow(RAGraphWin& w);
void close_infoWindow(RAGraphWin& w);

//void set_nodePosition(RAGraphWin& w);
void RAG_writeContours(RAGraphWin& w);
void RAG_writeSelectedContours(RAGraphWin& w);
void RAG_writeRAG(RAGraphWin& w);
void RAG_printRAG(RAGraphWin& w);

#endif



