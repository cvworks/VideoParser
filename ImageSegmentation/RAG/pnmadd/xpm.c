/*
 * xpm.c
 *
 * generate xpm's from ppm
 *
 * Sven Wachsmuth, 16.12.2002
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xpm.h"
#include "colorhist.h"

/* aus colorhist.c */
#include <list/hashmap.h>
extern int pnm_ppm_colorHash(ls_hashmap_t *hash, pixel *colorTab, int *max,
			     int *colorMap, ppm_t *ppm);
extern int pnm_ppm_colorHashPix(ls_hashmap_t *hash,pixel *pixrow,int n_pixrow);

#define PNM_XPM_MAXLINE (2048)

char *_xpm_pseudoColor[PNM_XPM_MAXPSEUDOCOLOR] = {
  "#000000",
  "#ffffff",
  "#ff0000",
  "#00ff00",
  "#0000ff",
  "#ffff00",
  "#ff00ff",
  "#00ffff",
  "#7f7f7f",
  "#7f0000",
  "#007f00",
  "#00007f",
  "#7f7f00",
  "#7f007f",
  "#007f7f",
  "#7fff00",
  "#7f00ff",
  "#ff7f00",
  "#007fff",
  "#ff007f",
  "#00ff7f",
  "#ff7f7f",
  "#7fff7f",
  "#7f7fff",
  "#7fffff",
  "#ff7fff",
  "#ffff7f" };

pnm_xpm_t *pnm_xpm_init(pnm_xpm_t *xpm)
{
  if (!xpm)
    xpm = (pnm_xpm_t *) malloc(sizeof(pnm_xpm_t));
  memset(xpm,0,sizeof(pnm_xpm_t));
  return (xpm);
}

pnm_xpm_t *pnm_xpm_create(void)
{
  return (pnm_xpm_init(NULL));
}

void pnm_xpm_free(pnm_xpm_t *xpm)
{
  if (!xpm) return;
  if (xpm->max_charData > 0) free(xpm->data[0]);
  if (xpm->max_data > 0) free(xpm->data);
}

void pnm_xpm_destroy(pnm_xpm_t *xpm)
{
  if (!xpm) return;
  pnm_xpm_free(xpm);
  free(xpm);
}

pnm_xpm_t *pnm_xpm_cpy(pnm_xpm_t *copy, pnm_xpm_t *xpm)
{
  if (!xpm) return (copy);
  if (!copy)
    copy = pnm_xpm_create();
  memcpy(copy,xpm,sizeof(pnm_xpm_t));
  if (copy->max_data > 0) {
    copy->data = (char **) malloc(copy->max_data * sizeof(char*));
    memcpy(copy->data, xpm->data, copy->max_data * sizeof(char *));
  }
  if (copy->max_charData > 0) {
    copy->data[0] = (char *) malloc(copy->max_charData * sizeof(char));
    memcpy(copy->data[0], xpm->data[0], copy->max_charData * sizeof(char));
    { 
      int i; for (i=1; i < copy->n_data; i++)
	copy->data[i] = xpm->data[i] - xpm->data[0] + copy->data[0];
    }
  }
  return (copy);
}

pnm_xpm_t *pnm_xpm_dup(pnm_xpm_t *xpm)
{
  return (pnm_xpm_cpy(NULL,xpm));
}
 
pnm_xpm_t *pnm_xpm_alloc(pnm_xpm_t *xpm, int cols, int rows, int n_colors)
{
  int lines = 1 + n_colors + rows;

  int mem = PNM_XPM_VALUESIZE; 
  mem += n_colors * PNM_XPM_COLORSIZE; 
  mem += rows * (PNM_XPM_PIXCHARS*cols+1); /* chars per pixel + string_end */

  if (!xpm)
    xpm = pnm_xpm_create();

  xpm->cols = cols;
  xpm->rows = rows;
  xpm->n_colors = 0;
  xpm->max_colors = n_colors;
  xpm->max_data = lines;
  xpm->data = (char **) malloc(lines * sizeof(char *));
  xpm->max_charData = mem;
  xpm->data[0] = (char *) malloc(mem * sizeof(char *));
  {
    char *colorMEM = PNM_XPM_COLOR_MEM(xpm);
    char **colorTab = PNM_XPM_COLOR_TAB(xpm);
    int i;
    for (i=0; i < n_colors; i++, colorMEM+=PNM_XPM_COLORSIZE)
      colorTab[i] = colorMEM;
  } {
    char *pixelMEM = PNM_XPM_PIXEL_MEM(xpm);
    char **colorMap = PNM_XPM_COLOR_MAP(xpm);
    int i;
    for (i=0; i < rows; i++, pixelMEM+=PNM_XPM_PIXCHARS*cols+1)
      colorMap[i] = pixelMEM;
  }
  return (xpm);
}

pnm_xpm_t *pnm_xpm_realloc(pnm_xpm_t *xpm, int cols, int rows, int n_colors)
{
  int max_colors;
  int lines;
  int mem;

  if (!xpm || !xpm->max_data)
    return (pnm_xpm_alloc(xpm,cols,rows,n_colors));

  /** the memory is not reorganized for less colors */
  max_colors = (n_colors > xpm->max_colors) ? n_colors : xpm->max_colors;
  lines = 1 + max_colors + rows;
  mem = (PNM_XPM_VALUESIZE 
	 + max_colors * PNM_XPM_COLORSIZE
	 + rows * (PNM_XPM_PIXCHARS*cols+1));

  if (xpm->max_data > 0 && xpm->max_data < lines) {
    xpm->max_data = lines;
    xpm->data = (char **) realloc(xpm->data, lines * sizeof(char *));
  }
  if (xpm->max_charData >= 0 && xpm->max_charData < mem) {
    char *tmp = xpm->data[0];
    int i;
    xpm->max_charData = mem;
    xpm->data[0] = (char *) ((xpm->max_charData > 0) ?
			     realloc(xpm->data[0], mem * sizeof(char)) :
			     malloc(mem *sizeof(char)));
    /** adapt pointers ... */
    for (i=1; i < xpm->n_data; i++)
      xpm->data[i] = xpm->data[i] - tmp + xpm->data[0];
  }
  /** if color table shall be expanded ... */
  if (xpm->max_colors < max_colors) {
    int i,j,m = max_colors - xpm->max_colors;
    /** move colorMap ... */
    char **colorTab = PNM_XPM_COLOR_TAB(xpm);
    char **colorMap = PNM_XPM_COLOR_MAP(xpm);
    char *colorMEM = PNM_XPM_COLOR_MEM(xpm);
    char *pixelMEM = PNM_XPM_PIXEL_MEM(xpm);
    int  mapSIZE = PNM_XPM_MAPSIZE(xpm);
    memmove(pixelMEM + m * PNM_XPM_COLORSIZE,pixelMEM,mapSIZE * sizeof(char));

    /** adapt map pointers ... */
    for (i = rows-1; i >=0; i--)
      colorMap[i+m] = colorMap[i] + m * PNM_XPM_COLORSIZE;

    /** adapt tab pointers ... */
    for (i = xpm->max_colors; i < max_colors; i++) {
      colorTab[i] = colorMEM + i * PNM_XPM_COLORSIZE;
    }
    xpm->max_colors = max_colors;
    xpm->n_data += m;
  }

  if (xpm->cols != cols) {
    int i;
    char *pixelMEM = PNM_XPM_PIXEL_MEM(xpm);
    char **colorMap = PNM_XPM_COLOR_MAP(xpm);
    /** adapt map pointers ... */
    for (i=0; i < rows; i++) {
      colorMap[i] = pixelMEM + i * (PNM_XPM_PIXCHARS*cols+1);
    }
    xpm->cols = cols;
  }
  xpm->n_data += (rows - xpm->rows);
  xpm->rows = rows;

  { /** update values ... */
    char *values = PNM_XPM_VALUES(xpm);
    sprintf(values,"%d %d %d %d", xpm->cols, xpm->rows, xpm->max_colors,
	    PNM_XPM_PIXCHARS);
  }
  return (xpm);
}

char *pnm_xpm_colorcode(char *line, int i)
{
  line[0] = i / PNM_XPM_PIXCHAR_N + PNM_XPM_PIXCHAR_OFFSET;
  if (line[0] >= '\\') line[0]++; 
  line[1] = i % PNM_XPM_PIXCHAR_N + PNM_XPM_PIXCHAR_OFFSET;
  if (line[1] >= '\\') line[1]++;
  return (line);
}

pnm_xpm_t *pnm_xpm_fbox(pnm_xpm_t *xpm, int cols, int rows, pixel color)
{
  int i;
  char **colorTab, **colorMap, *xpmValues;
  xpm = pnm_xpm_realloc(xpm, cols, rows, 1);
  xpmValues = PNM_XPM_VALUES(xpm);
  sprintf(xpmValues,"%d %d %d %d", cols, rows, xpm->max_colors,
	  PNM_XPM_PIXCHARS);
  colorTab = PNM_XPM_COLOR_TAB(xpm);
  sprintf(colorTab[0],".. c #%02x%02x%02x", 
	  PPM_GETR(color), PPM_GETG(color), PPM_GETB(color));
  xpm->n_colors = 1;
  colorMap = PNM_XPM_COLOR_MAP(xpm);
  memset(colorMap[0],'.', PNM_XPM_MAPSIZE(xpm) * sizeof(char));
  for (i=0; i < rows; i++)
    colorMap[i][2*cols] = '\0';

  xpm->n_data = xpm->max_colors + 1 + rows;
  return (xpm);
}

void _pnm_xpm_setColorTab(pnm_xpm_t *xpm, pixel* hist, int n_hist)
{
  int i;
  char **colorTab = PNM_XPM_COLOR_TAB(xpm);

  for (i=0; i < n_hist; i++) {
    pixel pix = hist[i];
    char *xpm_line = colorTab[i];
    pnm_xpm_colorcode(xpm_line,i);
    xpm_line += PNM_XPM_PIXCHARS;
    sprintf(xpm_line," c #%02x%02x%02x",
	    PPM_GETR(pix),PPM_GETG(pix),PPM_GETB(pix));
  }
  xpm->n_colors = n_hist;
}

void _pnm_xpm_setColorMap(pnm_xpm_t *xpm, int col0, int row0, 
			  pixel **pixels, int cols, int rows,
			  ls_hashmap_t *hash, int end_flag)
{
  int i,j;
  char **colorMap = PNM_XPM_COLOR_MAP(xpm);

  for (i=0; i < rows; i++) {
    char *xpm_line = colorMap[row0+i]+PNM_XPM_PIXCHARS*col0;
    for (j=0; j < cols; j++) {
      pixel pix = pixels[i][j];
      int icol = ls_hashmap_getValue(hash, pnm_ppm_pix2long(pix),0);
      pnm_xpm_colorcode(xpm_line, icol);
      xpm_line += PNM_XPM_PIXCHARS;
    }
    if (end_flag) *(xpm_line++) = '\0';
  }
}

pnm_xpm_t *pnm_ppm2xpm(pnm_xpm_t *xpm, ppm_t *ppm)
{
  int n_hist=0, max_hist=16;
  pixel *hist = (pixel *) calloc(16, sizeof(pixel));
  //ls_hashmap_t hash; DIEGO
  colorhash_table hash;
  char *xpmValues, **colorTab, **colorMap;

  if (!ppm) return xpm;

  /* generate color table */
  n_hist = pnm_ppm_colorHash(&hash, hist, &max_hist, NULL, ppm);

  xpm = pnm_xpm_realloc(xpm, ppm->cols, ppm->rows, n_hist);
  
  xpmValues = PNM_XPM_VALUES(xpm);

  sprintf(xpmValues,"%d %d %d %d", ppm->cols, ppm->rows, n_hist,
	  PNM_XPM_PIXCHARS);

  /** set color table ... */
  _pnm_xpm_setColorTab(xpm, hist, n_hist);

  /** set color map ... */
  _pnm_xpm_setColorMap(xpm, 0,0, ppm->pixels, ppm->cols, ppm->rows, &hash, 1);

  xpm->n_data = 1 + xpm->max_colors + xpm->rows;

  ppm_freecolorhash(hash);
  ppm_freecolorhist(hist);

  return (xpm);
}

void pnm_xpm_fprint(FILE *fp, pnm_xpm_t *xpm)
{
  int i;
  if (!fp || !xpm) return;

  fprintf(fp,"/* XPM */\n\
static char *xpm_file[] = {\n\
/* width height num_colors chars_per_pixel*/\n");
  for (i=0; i < xpm->n_data-1; i++) {
    fprintf(fp, "\"%s\",\n", xpm->data[i]);
  }
  fprintf(fp, "\"%s\"\n};\n", xpm->data[i]);
}

int pnm_xpm_fscan(FILE *fp, pnm_xpm_t *xpm)
{
  int i;
  char line[PNM_XPM_MAXLINE];

  if (!fp || !xpm) return (0);

  /** values */
  while (fgets(line, PNM_XPM_MAXLINE, fp)) {
    int offset = strspn(line," \t\"");
    char *start = line+offset;
    if (isdigit(*start)) {
      sscanf(line,"%d %d %d",&xpm->cols, &xpm->rows, &xpm->max_colors);
      xpm->n_colors = xpm->max_colors;
      break;
    }
  }
  /** color table */
  while (fgets(line,PNM_XPM_MAXLINE, fp)) {
    int offset = strspn(line," \t\"");
    char *start = line+offset;
    
  }
  /** color map */
  while (fgets(line,PNM_XPM_MAXLINE, fp)) {
    int offset = strspn(line," \t\"");
    char *start = line+offset;
  }
  
  return (1);
}

pixel *pnm_xpm2colorhist(pnm_xpm_t *xpm, int *n_hist)
{
  pixel *hist = NULL;
  char **colorTab;
  int i,n;

  if (!xpm || xpm->n_colors <= 0) { *n_hist = 0; return hist; }

  hist = (pixel *) 
    malloc(xpm->n_colors * sizeof(pixel));
  colorTab = PNM_XPM_COLOR_TAB(xpm);

  *n_hist = n = xpm->n_colors;
  for (i=0; i < n; i++) {
    char *color = colorTab[i] + PNM_XPM_PIXCHARS + 4;
    int r,g,b;
    sscanf(color,"%02x%02x%02x",&r,&g,&b);
    PPM_ASSIGN(hist[i], r,g,b);
  }
  return hist;
}

pixel *pnm_colorhist_join(pixel *hist, int *n_hist,
			  pixel *new_hist, int new_n_hist)
{
  int i,n;
  ls_hashmap_t hash;
  pnm_ppm_colorHashPix(&hash, hist, *n_hist);

  hist = (pixel *) 
    realloc(hist, ((*n_hist)+new_n_hist) * sizeof(pixel));
  n = *n_hist;
  for (i = 0; i < new_n_hist; i++) {
    pixel pix = new_hist[i];
    if (!ls_hashmap_exists(&hash, pnm_ppm_pix2long(pix))) {
      hist[n++] = pix;
    }
  }
  *n_hist = n;
  return (hist);
}

pnm_xpm_t *pnm_xpm_insertPPM(pnm_xpm_t *xpm, ppm_t *ppm, int x, int y)
{
  pixel *xpm_hist=NULL, *ppm_hist=NULL;
  int    xpm_n_hist, ppm_n_hist, xpm_max_hist=0, ppm_max_hist=0;

  if (!ppm) return (xpm);
  if (!xpm) return pnm_ppm2xpm(NULL,ppm);

  xpm_hist = pnm_xpm2colorhist(xpm, &xpm_n_hist);

  ppm_hist = pnm_ppm_colorTab(ppm_hist, &ppm_n_hist, &ppm_max_hist, NULL, ppm);
  xpm_hist = 
    pnm_colorhist_join(xpm_hist, &xpm_n_hist, ppm_hist, ppm_n_hist);
  
  pnm_xpm_realloc(xpm, xpm->cols, xpm->rows, xpm_n_hist);
  _pnm_xpm_setColorTab(xpm, xpm_hist, xpm_n_hist);
  {
    int cols = ppm->cols;
    int rows = ppm->rows;
    if (x + cols > xpm->cols) cols -= (x + cols - xpm->cols);
    if (y + rows > xpm->rows) rows -= (y + rows - xpm->rows);

    if (cols > 0 && rows > 0) {
	ls_hashmap_t xpm_hash;
	pnm_ppm_colorHashPix(&xpm_hash, xpm_hist, xpm_n_hist);
	_pnm_xpm_setColorMap(xpm, x,y, ppm->pixels, cols, rows, &xpm_hash, 0);
	ls_hashmap_free(&xpm_hash);
    }
  }
  xpm->n_data = 1 + xpm->max_colors + xpm->rows;

  free (xpm_hist);
  free (ppm_hist);

  return (xpm);
}  

int _pnm_xpm_addPseudo2ColorTab(pnm_xpm_t *xpm, int max_value)
{
  int i;
  int    n_colors = xpm->n_colors;
  char **colorTab = PNM_XPM_COLOR_TAB(xpm) + n_colors;

  for (i=0; i < max_value; i++) {
    char *pseudo = _xpm_pseudoColor[i];
    char *xpm_line = colorTab[i];
    pnm_xpm_colorcode(xpm_line,i+n_colors);
    xpm_line += PNM_XPM_PIXCHARS;
    sprintf(xpm_line," c %s", pseudo);
  }
  xpm->n_colors += max_value;
  return (n_colors);
}

int _pnm_xpm_addPixel2ColorTab(pnm_xpm_t *xpm, int max_value, pixel *pixelTab)
{
  int i;
  int    n_colors = xpm->n_colors;
  char **colorTab = PNM_XPM_COLOR_TAB(xpm) + n_colors;

  for (i=0; i < max_value; i++) {
    char *xpm_line = colorTab[i];
    pnm_xpm_colorcode(xpm_line,i+n_colors);
    xpm_line += PNM_XPM_PIXCHARS;
    sprintf(xpm_line," c #%x%x%x", 
	    PPM_GETR(pixelTab[i]), 
	    PPM_GETG(pixelTab[i]), 
	    PPM_GETB(pixelTab[i]));
  }
  xpm->n_colors += max_value;
  return (n_colors);
}

int _pnm_xpm_addValues2ColorTab(pnm_xpm_t *xpm, int max_val)
{
  int i;
  int    n_colors = xpm->n_colors;
  char **colorTab = PNM_XPM_COLOR_TAB(xpm) + n_colors;

  for (i=0; i < max_val; i++) {
    char *xpm_line = colorTab[i];
    pnm_xpm_colorcode(xpm_line,i+n_colors);
    xpm_line += PNM_XPM_PIXCHARS;
    sprintf(xpm_line," c #%02x%02x%02x", i, i, i);
  }
  xpm->n_colors += max_val;
  return (n_colors);
}

void _pnm_xpm_setColorMapINT(pnm_xpm_t *xpm, int x, int y, 
			     int *data, int cols, int rows, 
			     int max_val, int offset, int end_flag)
{
  int i,j;
  char **colorMap = PNM_XPM_COLOR_MAP(xpm);
  int *row;

  for (row=data, i=0; i < rows; i++, row+=cols) {
    char *xpm_line = colorMap[y+i]+PNM_XPM_PIXCHARS*x;
    for (j=0; j < cols; j++) {
      int icol = (row[j] % max_val) + offset;
      pnm_xpm_colorcode(xpm_line, icol);
      xpm_line += PNM_XPM_PIXCHARS;
    }
    if (end_flag) *(xpm_line++) = '\0';
  }
}

void _pnm_xpm_setColorMapUCHAR(pnm_xpm_t *xpm, int x, int y, 
			       unsigned char *data, int cols, int rows, 
			       int max_val, int offset, int end_flag)
{
  int i,j;
  char **colorMap = PNM_XPM_COLOR_MAP(xpm);
  unsigned char *row;

  for (row=data, i=0; i < rows; i++, row+=cols) {
    char *xpm_line = colorMap[y+i]+PNM_XPM_PIXCHARS*x;
    for (j=0; j < cols; j++) {
      int icol = (row[j] % max_val) + offset;
      pnm_xpm_colorcode(xpm_line, icol-1);
      xpm_line += PNM_XPM_PIXCHARS;
    }
    if (end_flag) *(xpm_line++) = '\0';
  }
}


pnm_xpm_t *pnm_xpm_insertINT(pnm_xpm_t *xpm, int *data, int cols, int rows, int max_val,
			      int x, int y, int pseudo_color)
{
  int offset;
  int max_color = ((pseudo_color) ? ((max_val > PNM_XPM_MAXPSEUDOCOLOR) ? 
				     PNM_XPM_MAXPSEUDOCOLOR : max_val)
		   : max_val);

  if (!data) return (xpm);
  if (!xpm) return (xpm); /*pnm_int2xpm(NULL, data, cols, rows, pseudo_color);*/

  pnm_xpm_realloc(xpm, xpm->cols, xpm->rows, xpm->n_colors + max_color);

  if (pseudo_color) {
    offset = _pnm_xpm_addPseudo2ColorTab(xpm, max_color);
  } else {
    offset = _pnm_xpm_addValues2ColorTab(xpm, max_color);
  }

  if (x + cols > xpm->cols) cols -= (x + cols - xpm->cols);
  if (y + rows > xpm->rows) rows -= (y + rows - xpm->rows);
  
  if (cols > 0 && rows > 0)
    _pnm_xpm_setColorMapINT(xpm, x,y, data, cols, rows, max_color, offset, 0);

  xpm->n_data = 1 + xpm->max_colors + xpm->rows;

  return(xpm);
}

pnm_xpm_t *pnm_xpm_insertUCHAR(pnm_xpm_t *xpm, unsigned char *data, int cols, int rows, 
			       unsigned char max_val, int x, int y, int pseudo_color)
{
  int offset;
  int max_color = ((pseudo_color) ? ((max_val > PNM_XPM_MAXPSEUDOCOLOR) ? 
				     PNM_XPM_MAXPSEUDOCOLOR : max_val)
		   : max_val);

  if (!data) return (xpm);
  if (!xpm) return (xpm); /*pnm_char2xpm(NULL, data, cols, rows, pseudo_color);*/

  pnm_xpm_realloc(xpm, xpm->cols, xpm->rows, xpm->n_colors + max_color);

  if (pseudo_color) {
    offset = _pnm_xpm_addPseudo2ColorTab(xpm, max_color);
  } else {
    offset = _pnm_xpm_addValues2ColorTab(xpm, max_color);
  }

  if (x + cols > xpm->cols) cols -= (x + cols - xpm->cols);
  if (y + rows > xpm->rows) rows -= (y + rows - xpm->rows);
  
  if (cols > 0 && rows > 0)
    _pnm_xpm_setColorMapUCHAR(xpm, x,y, data, cols, rows, max_color, offset, 0);
  
  xpm->n_data = 1 + xpm->max_colors + xpm->rows;

  return(xpm);
}

pnm_xpm_t *pnm_xpm_insertUCHARbyTab(pnm_xpm_t *xpm, unsigned char *data, 
				    int cols, int rows, 
				    unsigned char max_val, int x, int y,
				    int n_colors, pixel *colorTab)
{
  int offset;
  int max_color = n_colors;

  if (!data) return (xpm);
  if (!xpm) return (xpm); /*pnm_char2xpm(NULL, data, cols, rows, pseudo_color);*/

  pnm_xpm_realloc(xpm, xpm->cols, xpm->rows, xpm->n_colors + max_color);

  offset = _pnm_xpm_addPixel2ColorTab(xpm, max_color, colorTab);

  if (x + cols > xpm->cols) cols -= (x + cols - xpm->cols);
  if (y + rows > xpm->rows) rows -= (y + rows - xpm->rows);
  
  if (cols > 0 && rows > 0)
    _pnm_xpm_setColorMapUCHAR(xpm, x,y, data, cols, rows, max_color, offset, 0);
  
  xpm->n_data = 1 + xpm->max_colors + xpm->rows;

  return(xpm);
}
