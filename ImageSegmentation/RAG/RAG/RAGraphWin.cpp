/*
 * RAGraphWin.C
 *
 * C++ class for Region-Adjacency-Graph-Window
 *
 * Sven Wachsmuth, 3.12.2002, V0.1
 *
 * Sven Wachsmuth, 19.3.2003, V0.2
 * - zooming of background and graph added
 *
 */
#include <pnmadd/xpm.h>
#include "RAGraphWin.h"

RAGraphWin::RAGraphWin(RAGraph& RAG, 
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
		       const char *win_label/*=""*/)
    : GraphWin(RAG, w,h,win_label) 
{
  init();
  initImage(image_w, image_h, image_x, image_y, image_f, bg_h, bg_x, bg_y,
	    bg_xpm);
  initWindow();
}

RAGraphWin::RAGraphWin(RAGraph& RAG,
		       const char *win_label,
		       int *regionMap,
		       int cols,
		       int rows,
		       int image_s/*=1*/,
		       unsigned char *colorMap/*=NULL*/,
		       ppm_t *image/*=NULL*/)
  : GraphWin(RAG,
	     ((image_s<0) ? cols/(-image_s-1) : cols*(image_s+1))
	     * ((colorMap) ? 2:1) + ((image) ? cols:0) + cols/2,
	     ((image_s<0) ? rows/(-image_s-1) : rows*(image_s+1))
	     + 90, 
	     win_label) 
{
  init();
  initImage(regionMap, cols, rows, image_s, colorMap, image);
  initWindow();
}

RAGraphWin::RAGraphWin(RAGraph& RAG,
		       const char *win_label/*=""*/,
		       int image_s/*=1*/)
  : GraphWin(RAG,
	     ((image_s<0) ? RAG.getCols()/(-image_s+1) 
	      : RAG.getCols()*(image_s+1))
	     * ((RAG.getColorMap()) ? 2:1) 
	     + ((RAG.getImage()) ? RAG.getCols():0) + RAG.getCols()/2,
	     ((image_s<0) ? RAG.getRows()/(-image_s+1) 
	      : RAG.getRows()*(image_s+1))
	     + 90, 
	     win_label)
{
  init();
  initImage(RAG.getRegionMap(), RAG.getCols(), RAG.getRows(), 
	    image_s, RAG.getColorMap(), RAG.getImage());

  initWindow();
}

void RAGraphWin::init(void)
{
  info_window = NULL;
  bg_pixmap = NULL;
  m_bShowNodes = true;
}

void updateDisplay(leda_window *win, double a, double b, double c, double d);

void updateDisplay(leda_window *win, double a, double b, double c, double d)
{
  //cout << "updateDisplay" << endl;
  RAGraphWin *gw = (RAGraphWin *) win->get_graphwin();

  gw->set_bg_redraw(NULL);
  gw->updateImages(win->height());
  gw->set_bg_redraw(updateDisplay);

  set_nodePositions(*gw);
}

void RAGraphWin::initWindow(void)
{
  using namespace leda;
  
  w_scale = 0.0;
  set_node_width(30);
  set_node_label_font(leda::roman_font, 12);
  set_zoom_objects(false); 
  set_animation_steps(0);
  set_nodePositions(*this);
  int menu_id = get_menu("Layout");
  if (menu_id >= 0) {
    add_separator(menu_id);
    add_simple_call((void (*)(GraphWin&)) &set_nodePositions,"re-position",
		    menu_id);
  }
  menu_id = get_menu("File");
  if (menu_id >= 0) {
    add_separator(menu_id);
    add_simple_call((void (*)(GraphWin&)) &RAG_writeContours,"write contours",
		    menu_id);
    add_simple_call((void (*)(GraphWin&)) &RAG_writeSelectedContours,
		    "write sel. contours", menu_id);
    add_simple_call((void (*)(GraphWin&)) &RAG_writeRAG, "write RAG (graph.rag)",
		    menu_id);
    add_simple_call((void (*)(GraphWin&)) &RAG_printRAG, "print RAG",
		    menu_id);
  }
  menu_id = get_menu("Graph");
  if (menu_id >= 0) {
    add_separator(menu_id);
   add_simple_call((void (*)(GraphWin&)) &RAG_mergeSelectedNodes,
		    "merge sel. contours", menu_id);
  }
  menu_id = get_menu("Window");
  if (menu_id >= 0) {
    add_separator(menu_id);
    add_simple_call((void (*)(GraphWin&)) &open_infoWindow, 
		    "open info", menu_id);
    add_simple_call((void (*)(GraphWin&)) &close_infoWindow,
		    "close info", menu_id);
  }
  set_bg_redraw(updateDisplay);
}
  
void RAGraphWin::initImage(int image_w,
			   int image_h, 
			   int image_x, 
			   int image_y, 
			   float image_f,
			   int bg_h, 
			   int bg_x, 
			   int bg_y, 
			   const char **bg_xpm)
{
  image_width = image_w;
  image_height = image_h;
  image_woffset = image_x;
  image_hoffset = image_y;
  image_factor = image_f;
  
  bg_height = bg_h;
  bg_woffset = bg_x;
  bg_hoffset = bg_y;
  
  display();
  bg_pixmap = get_window().create_pixrect(bg_xpm);
}

void RAGraphWin::initImage(int *regionMap,
			   int cols,
			   int rows,
			   int image_s/*=1*/,
			   unsigned char *colorMap/*=NULL*/,
			   ppm_t *image/*=NULL*/)
{
  image_scale = image_s;
  image_factor 
    = (image_s > 0) ? 1+image_s : 1.0/(float) (1-image_s);
  int image_w = (int) (cols * image_factor);
  int image_h = (int) (rows * image_factor);

  pixel background_color;
  int bg_h = 3 * image_h;
  int bg_w  = 6 * image_w;
  pnm_xpm_t *xpm_image;
  PPM_ASSIGN(background_color, 255, 255, 255);
  xpm_image = pnm_xpm_fbox(NULL, bg_w, bg_h, 
			   (image) ? image->pixels[0][0] : background_color);
  //** regionMap
  int image_x = 0;
  int image_y = 0;

  //** original image
  if (image) {
    int orig_x = ((colorMap) ? 2:1) * cols * (int) image_factor;
    pnm_xpm_insertPPM(xpm_image, image, orig_x, image_y);
  }

  //** regionMap
  if (image_scale == 0) {
    pnm_xpm_insertINT(xpm_image, regionMap,cols,rows,255, image_x,image_y, 1);
  } else {
    int *regionMap_scaled
      = pnm_int_scale(NULL, regionMap, cols, rows, image_scale);
    pnm_xpm_insertINT(xpm_image, regionMap_scaled, image_w, image_h, 255, 
		      image_x, image_y, 1);
    free(regionMap_scaled);
  }
  //** colorMap
  if (colorMap) {
    int colorMap_x = image_w;
    if (image_scale == 0) {
      rag_color_t *_tab = ((RAGraph &) get_graph()).getColorTab();
      if (_tab) {
	  pixel *pixelTab = NULL;
	  const rag_t *p_rag = (((RAGraph &) get_graph()).get()) ? 
	      ((RAGraph &) get_graph()).get()->get() : NULL;
	  int n_colors = rag_getColorTabPixels(&pixelTab, (rag_t *)p_rag);
	  pnm_xpm_insertUCHARbyTab(xpm_image, colorMap, cols, rows, 255,
				   colorMap_x, image_y, n_colors,pixelTab);
      } else 
	  pnm_xpm_insertUCHAR(xpm_image, colorMap, cols, rows, 255,
			      colorMap_x, image_y, 1);
    } else {
      unsigned char *colorMap_scaled
	= pnm_uchar_scale(NULL, colorMap, cols, rows, image_scale);
      const rag_color_t *_tab = ((RAGraph &) get_graph()).getColorTab();
      if (_tab) {
	pixel *pixelTab = NULL;
	const rag_t *p_rag = (((RAGraph &) get_graph()).get()) ? 
	    ((RAGraph &) get_graph()).get()->get() : NULL;
	int n_colors = rag_getColorTabPixels(&pixelTab, (rag_t *) p_rag);
	pnm_xpm_insertUCHARbyTab(xpm_image, colorMap_scaled, image_w, image_h,
				 255, colorMap_x, image_y, n_colors,pixelTab);
      } else 
	pnm_xpm_insertUCHAR(xpm_image, colorMap_scaled, image_w, image_h, 255, 
			    colorMap_x, image_y, 1);
      free(colorMap_scaled);
    }
  }
  initImage(image_w, image_h, image_x, image_y, image_factor, bg_h,
	    0, 32-2*image_h, (const char **) xpm_image->data);
  pnm_xpm_destroy(xpm_image);
}

void RAGraphWin::updateImages(int height)
{
  RAGraph* RAG = (RAGraph *) &get_graph();
  int rows = (int) RAG->getRows();
  float factor = (float) (height) / (1.2 * (float) rows);
  int scale = ((factor > 1.0) ? 
	       ((int) factor) - 1 :
	       (- (int) (1.0 / factor)));
  
  //cout << "height " << height << ", rows " << rows << ", scale " << scale
  //     << endl;
  if (scale != image_scale) {
    initImage(RAG->getRegionMap(), RAG->getCols(), RAG->getRows(), 
	      scale, RAG->getColorMap(), RAG->getImage());
    w_scale = 0.0;
  }
}

void set_nodePositions(RAGraphWin& w) 
{
  w.set_bg_redraw(NULL);

  leda_node r;
  RAGraph* RAG = (RAGraph *) &w.get_graph();
  leda_window& win = w.get_window();
  char *pixmap = w.getBgPixmap();
  int   bg_x   = w.getBgWOffset();
  int   bg_y   = w.getBgHOffset();
  int   bg_h   = w.getBgHeight();
  int   img_x  = w.getImgWOffset();
  int   img_y  = w.getImgHOffset();
  float img_f  = w.getImgFactor();

  //  fprintf(stderr,"%d %d %d %d %d %g %g\n",bg_x, bg_y, bg_h, img_x, img_y, 
  //	  img_f, win.scale());
  //  fprintf(stderr,"set pixmap %d %d\n",  
  //	  bg_x, (int) (bg_y / w.get_window().scale()));

  //  cerr << win.xmax() << "," << win.xmin() << ";" << win.width() << endl;
  //  cerr << win.ymax() << "," << win.ymin() << ";" << win.height() << endl;
  //cerr << "scale: " << win.scale() << endl;
  //cerr << "pos: " << win.xpos() << "," << win.ypos() << endl;
  
  if (w.get_window().scale() != w.w_scale) {
    w.set_bg_pixmap(pixmap, bg_x, (int) (win.ymin()) + (bg_y / w.get_window().scale()));
    w.w_scale = w.get_window().scale();

    //cerr << bg_x << "," << (int) (bg_y / w.get_window().scale()) << endl;
    
    forall_nodes(r,*RAG) {
      float x = img_x+bg_x+ img_f * (float) RAG->inf(r).getCenter().xcoord();
      float y = bg_y+bg_h-img_y-img_f*(float) RAG->inf(r).getCenter().ycoord();
      leda_point p((x / win.scale()), (win.ymin()) + (y/ win.scale()));
      
      //cerr << RAG->inf(r).getLabel() << ":" << p << img_f << endl;
      if (w.getShowNodes())
      {
	w.set_position(r,p);
	w.set_color(r,RAG->inf(r).getPseudoColor());
	leda_string label("%d",RAG->inf(r).getLabel());
	w.set_label(r,label);
      }
    }
  }
  w.set_bg_redraw(updateDisplay);
}     

void set_nodePosition(RAGraphWin& w, leda_node r)
{
  RAGraph* pRAG = (RAGraph *) &w.get_graph();
  leda_window& win = w.get_window();
  int   bg_x   = w.getBgWOffset();
  int   bg_y   = w.getBgHOffset();
  int   bg_h   = w.getBgHeight();
  int   img_x  = w.getImgWOffset();
  int   img_y  = w.getImgHOffset();
  float img_f  = w.getImgFactor();

  float x = img_x+bg_x+ img_f * (float) (*pRAG)[r].getCenter().xcoord();
  float y = bg_y+bg_h-img_y-img_f*(float) (*pRAG)[r].getCenter().ycoord();
  leda_point p((x / win.scale()), (win.ymin()) + (y/ win.scale()));
  
  //cerr << p << img_f << endl;
  w.set_position(r,p);
  w.set_color(r,pRAG->inf(r).getPseudoColor());
  leda_string label("%d",pRAG->inf(r).getLabel());
  w.set_label(r,label);
}

void open_infoWindow(RAGraphWin& gw)
{
  using namespace leda;
  
  if (gw.info_window)
    delete gw.info_window;
  gw.info_window = new leda_window(500,200);
  gw.info_window->display();
  gw.info_window->message("-- left mouse click for node/edge information --");
  gw.info_action 
    = gw.set_action(A_LEFT|A_NODE, 
		    (void (*)(GraphWin&,const leda_point&))
		    set_nodeInfoMessage);
  gw.info_action 
    = gw.set_action(A_LEFT|A_EDGE, 
		    (void (*)(GraphWin&,const leda_point&))
		    set_edgeInfoMessage);
}

void close_infoWindow(RAGraphWin& gw)
{
  using namespace leda;
  
  if (gw.info_window) {
    delete gw.info_window;
    gw.set_action(A_LEFT|A_NODE, 
		  (void (*)(GraphWin&,const leda_point&)) gw.info_action);
    gw.info_window = NULL;
  }
}

void set_nodeInfoMessage(RAGraphWin&gw, const leda_point &p)
{
  if (gw.info_window) {
    leda_node r = gw.get_edit_node();
    RAGraph * RAG = (RAGraph *) &gw.get_graph();
    //RAG_Region * reg = (RAG_Region *) &(*RAG)[r];
    const leda_string& s = (*RAG)[r].getInfoString();
    gw.info_window->message(s);
    int n_mergeList;
    const int *mergeList = RAG->getSubLabels(r,n_mergeList);
    if (n_mergeList > 1) {
      leda_string r_info("... consists of");
      int i;
      for(i=0; i < n_mergeList; i++) {
	leda_string _r_info(" %d",mergeList[i]);
	r_info += _r_info;
      }
      gw.info_window->message(r_info);
    }
  }
}

void set_edgeInfoMessage(RAGraphWin&gw, const leda_point &p)
{
  using namespace leda;
  
  if (gw.info_window) {
    edge_struct *edge = gw.get_edit_edge();
    RAGraph * RAG = (RAGraph *) &gw.get_graph();
    RAG_Adjacency *adj = (RAG_Adjacency *) &RAG->inf(edge);
    const leda_string& s = adj->getInfoString();
    gw.info_window->message(s);
  }
}
    
void RAG_writeContours(RAGraphWin& w)
{
  RAGraph* RAG = (RAGraph *) &w.get_graph();
  RAG->writeContours();
}

void RAG_writeSelectedContours(RAGraphWin& w)
{
  using namespace leda;
  
  leda_node node;
  RAGraph* RAG = (RAGraph *) &w.get_graph();
  leda_list<node_struct*> sel_nodes;
  sel_nodes = w.get_selected_nodes();
  forall(node,sel_nodes) {
    RAG->writeContour(node);
  }
}

void RAG_writeRAG(RAGraphWin& w)
{
  RAGraph * RAG = (RAGraph *) &w.get_graph();
  std::ofstream O;
  O.open("graph.rag");
  RAG->write(O);
  O.close();
}

void RAG_printRAG(RAGraphWin& w)
{
  RAGraph * RAG = (RAGraph *) &w.get_graph();
  RAG->write(std::cout);
}


void RAG_mergeSelectedNodes(RAGraphWin& w)
{
  using namespace leda;
  
  RAGraph* RAG = (RAGraph *) &w.get_graph();
  leda_list<node_struct*> sel_nodes;
  sel_nodes = w.get_selected_nodes();
  
  leda_node n1 = sel_nodes.pop();
  leda_node n2 = sel_nodes.pop();
  
  //cout << "merging " << endl << (*RAG)[n1] << endl << (*RAG)[n2];

  leda_node merged_node = RAG->merge(n1, n2);
  
  if (merged_node != nil) {
    //RAG->writeContour(merged_node);
    w.update_graph();
    
    set_nodePosition(w,merged_node);
  }
}
