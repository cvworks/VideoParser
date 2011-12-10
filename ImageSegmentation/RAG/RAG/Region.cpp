/*
 * RAGraph/Region.C
 *
 * C++ class for RAG nodes
 *
 * Sven Wachsmuth, 28.11.2002
 * Sven Wachsmuth, 1.5.2003 (totally re-organized)
 */
#include <string.h>
#include <pnmadd/pseudo.h>
#include <pnmadd/array.h>
#include "Region.h"
#include "RAGraph.h"

using namespace std;

RAG_RefRegion& RAG_RefRegion::initInfoValues(void) 
{
  region_info_t *info = get();
  if (!info) return (*this);

  region_center(info);
  region_minmax(info);

  return (*this);
}

RAG_RefRegion& RAG_RefRegion::initValues(void) 
{
  initInfoValues();
  initObjectValues();

  return (*this);
}

RAG_RefRegion& RAG_RefRegion::initObjectValues(void) 
{
  region_info_t *info = get();
  if (!info) return (*this);

  leda_point p(info->center_x, info->center_y);
  center = p;
  int r,g,b;
  pnm_getPseudoColor(&r,&g,&b, info->label);
  leda_color c(r,g,b);
  pseudo_color = c;

  contour_width = info->max_x - info->min_x + 1;
  contour_height = info->max_y - info->min_y + 1;
  
  setInfoString();
  
  return (*this);
}

RAG_RefRegion& RAG_RefRegion::init(void)
{
  label = -1;
  rag_info = NULL;

  image = NULL;
  info_string = "";

  external_index = -1;
  score          = 0.0;

  return (*this);
}

ppm_t *RAG_RefRegion::getSubImage(ppm_t *subimage) const
{
  if (!image) return (subimage);

  region_info_t *info = get();
  if (!info) return (subimage);
  
  int cols = info->max_x - info->min_x + 3;
  int rows = info->max_y - info->min_y + 3;
  int xoffset = info->min_x - 1;
  int yoffset = info->min_y - 1;
  if (xoffset < 0) { cols+=xoffset; xoffset = 0; }
  if (xoffset + cols > image->cols) { cols = image->cols - xoffset; }
  if (yoffset < 0) { rows+=yoffset; yoffset = 0; }
  if (yoffset + rows > image->rows) { rows = image->rows - yoffset; }

  if (!subimage)
    subimage = pnm_ppm_create();

  if (subimage->pixels) {
    if (subimage->rows < rows) 
      subimage->pixels = (pixel **)
	realloc(subimage->pixels, rows * sizeof(pixel *));
  } else
    subimage->pixels = (pixel **) malloc(rows * sizeof(pixel *));

  subimage->cols = cols;
  subimage->rows = rows;
  subimage->maxval = image->maxval;
  int i;
  for (i=0; i < rows; i++)
    subimage->pixels[i] = image->pixels[yoffset] + xoffset;

  return (subimage);
}

char *RAG_RefRegion::getContourPixmap(char *pixmap /*= NULL*/, int rows /*= 0*/, int cols /*= 0*/) const
{
  region_info_t *info = get();
  if (!info) return (pixmap);

  int pixmap_cols = getContourPixmapCols();
  int pixmap_rows = getContourPixmapRows();
    
  if (pixmap == NULL)
    pixmap = (char *) malloc(pixmap_cols * pixmap_rows * sizeof(char));
  else if (pixmap != NULL && (pixmap_rows != rows || pixmap_cols != cols))
  	// pixmap = (char *) realloc(pixmap, pixmap_cols*pixmap_rows*sizeof(char));
	return NULL;

  memset(pixmap, 0xff, pixmap_cols * pixmap_rows);
  char *offset_pixmap 
    = pixmap - (info->min_y - 1) * pixmap_cols - (info->min_x - 1);
  region_contour2charImage(offset_pixmap, pixmap_cols, 
			   info->n_contour, info->contour, 0x0);
  return (pixmap);
}

pgm_t *RAG_RefRegion::getContourPGM(pgm_t *contour_pgm) const
{
  gray *pixmap;
  region_info_t *info = get();
  if (!info) return (contour_pgm);

  if (!contour_pgm)
    contour_pgm = pnm_pgm_create();
  else {
    pnm_pgm_free(contour_pgm);
    pnm_pgm_init(contour_pgm);
  }

  contour_pgm->cols = info->max_x - info->min_x + 3;
  contour_pgm->rows = info->max_y - info->min_y + 3;
  contour_pgm->maxval = 255;
  
  pixmap = (gray *) malloc(contour_pgm->cols*contour_pgm->rows * sizeof(gray));

  contour_pgm->pixels = (gray **) 
    pnm_array2matrix(NULL, contour_pgm->cols, contour_pgm->rows,
		     (void *) pixmap, sizeof(gray));
  pnm_pgm_clear(contour_pgm, 255);

  gray *offset_pixmap 
      = pixmap - (info->min_y - 1) * contour_pgm->cols - (info->min_x - 1);
  region_contour2image((int *) offset_pixmap, contour_pgm->cols,
		       info->n_contour, info->contour, 0);
  return (contour_pgm);
}

const leda_string& RAG_RefRegion::setInfoString(void)
{
  leda_string *_info_string = newInfoString();
  info_string = *_info_string;
  delete _info_string;
  return (info_string);
}

leda_string *RAG_RefRegion::newInfoString(void) const
{
  leda_string *_string = NULL;
  region_info_t *info = get();
  if (info)
    _string 
      = new leda_string("region %d: C%d #%d O#%d -#%d |#%d [%d,%d..%d,%d] *(%.1f,%.1f)",
			info->label, info->color, info->pixelcount,
			info->n_contour, contour_width, contour_height,
			info->min_x, info->min_y, info->max_x, info->max_y,
			info->center_x, info->center_y);
  else
    _string = new leda_string();

  return (_string);
}

leda_list<leda_point> * RAG_RefRegion::newContourList(void) const
{
  leda_list<leda_point> *contourList = new leda_list<leda_point>();
  int n;
  const contour_point_t * contour = getContour(&n);
  int i;
  for (i = 0; i < n; i++) {
    contourList->append(leda_point(contour[i].x, contour[i].y));
  }
  return (contourList);
}

leda_list<int> * RAG_RefRegion::newSubLabelList(void) const
{
  leda_list<int> *subLabelList = new leda_list<int>();
  int n;
  int * subLabels = getSubLabels(n);
  int i;
  for (i = 0; i < n; i++) {
    subLabelList->append(subLabels[i]);
  }
  return (subLabelList);
}

istream& RAG_Region::readInfo(istream& s)
{
  leda_string str;
  const char *_str;
  char t;

  s >> external_index >> t; 
  str.read(s,'}');
  if (!region_info)
    region_info = region_info_create();
  else {
    region_info_free(region_info);
    region_info_init(region_info);
  }
  region_info_sscan(region_info,(char *) (_str = str));

  return (s);
}

void RAG_RefRegion::setSubLabels(leda_list<int>& subLabels)
{
  int subLabel;
  region_info_t *info = get();
  if (!info) return;

  region_clearSubLabels(info);
  forall(subLabel, subLabels) {
    region_addSubLabel(info, subLabel);
  }
}

void RAG_RefRegion::setContour(leda_list<leda_point>& contourList)
{
  leda_point p;
  region_info_t *info = get();
  if (!info) return;
  
  info->n_contour = contourList.length();

  if (info->max_contour < info->n_contour) {
    if (info->max_contour > 0) {
      info->contour = (contour_point_t *)
	realloc(info->contour, info->n_contour * sizeof(contour_point_t));
    } else {
      info->contour = (contour_point_t *)
	malloc(info->n_contour * sizeof(contour_point_t));
    }
    info->max_contour = info->n_contour;
  }
  int i=0;
  forall(p,contourList) {
    if (i >= info->n_contour) break;
    info->contour[i].x = (int) p.xcoord();
    info->contour[i++].y = (int) p.ycoord();
  }
}

RAG_Region& RAG_Region::set(const RAG_Region& r)
{
  (RAG_RefRegion&) (*this) = (RAG_RefRegion&) r;

  region_info = region_info_cpy(region_info,r.region_info);

  return (*this);
}
  

ostream& operator<<(ostream& s, const RAG_RefRegion& reg)
{
  char *str = NULL;
  str = region_info_sprint(str,reg.get());
  s << reg.external_index << "|" << str;
  free(str);
  return (s);

  /*************
  leda_list<leda_point> *contourList = reg.newContourList();
  leda_list<int> *subLabels= reg.newSubLabelList();

  s << reg.getInfoString();
  s << " [#" << subLabels->length() << *subLabels << "]";
  s << " [#" << contourList->length() << *contourList << "]";
  delete contourList;
  delete subLabels;
  return (s);
  *************/
}

istream& operator>>(istream& s, RAG_Region& reg)
{ 
  //int x;
  reg.readInfo(s);
  /*************
  _drop(s,"[#") >> x;
  leda_list<int> subLabels;
  subLabels.read(s,']');
  reg.setSubLabels(subLabels);
  leda_list<leda_point> contourList;
  _drop(s,"[#") >> x;
  contourList.read(s,']');
  reg.setContour(contourList);
  **********/

  reg.initObjectValues();

  return (s);
}




