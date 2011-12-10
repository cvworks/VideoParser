/*
 * xpm.c
 *
 * generate xpm's from pnm_images
 *
 * Sven Wachsmuth, 16.12.2002
 */
#ifndef _PNM_XPM_H
#define _PNM_XPM_H

#include "pnmadd.h"

#ifdef __cplusplus
extern "C" {
#endif
/*#include <ppmcmap.h>*/

typedef struct pnm_xpm_t {
  int cols;
  int rows;
  int n_colors;
  int max_colors;
  int max_data;
  int n_data;
  int max_charData;
  char **data;
} pnm_xpm_t;

#define PNM_XPM_MAXPSEUDOCOLOR (27)
#define PNM_XPM_PSEUDOCOLOR(i) (_xpm_pseudoColor[(i) % PNM_XPM_MAXPSEUDOCOLOR])

  /** pnm_xpm_colorcode depends on 2 */
#define PNM_XPM_PIXCHARS (2)

#define PNM_XPM_PIXCHAR_OFFSET ('.')
#define PNM_XPM_PIXCHAR_N      (64)

#define PNM_XPM_VALUES(xpm) ((xpm)->data[0])
#define PNM_XPM_COLOR_TAB(xpm) (&(xpm)->data[1])
#define PNM_XPM_COLOR_MAP(xpm) (&(xpm)->data[1+(xpm)->max_colors])

#define PNM_XPM_VALUESIZE (17) /*'XXXX YYYY CCCC 2\0'<cols> <rows> <max_colors>*/
#define PNM_XPM_COLORSIZE (13) /*'PP\tc #RRGGBB\0' <label> <red,green,blue>*/
#define PNM_XPM_ROWSIZE(xpm) ((xpm)->cols*PNM_XPM_PIXCHARS+1)
#define PNM_XPM_MAPSIZE(xpm) (((xpm)->cols*PNM_XPM_PIXCHARS+1)*(xpm)->rows)

#define PNM_XPM_COLOR_MEM(xpm) ((xpm)->data[0]+PNM_XPM_VALUESIZE)
#define PNM_XPM_PIXEL_MEM(xpm) ((xpm)->data[0]+PNM_XPM_VALUESIZE+(xpm)->max_colors*PNM_XPM_COLORSIZE)

pnm_xpm_t *pnm_xpm_init(pnm_xpm_t *xpm);
pnm_xpm_t *pnm_xpm_create(void);
void       pnm_xpm_free(pnm_xpm_t *xpm);
void       pnm_xpm_destroy(pnm_xpm_t *xpm);

pnm_xpm_t *pnm_xpm_cpy(pnm_xpm_t *copy, pnm_xpm_t *xpm);
pnm_xpm_t *pnm_xpm_dup(pnm_xpm_t *xpm);
pnm_xpm_t *pnm_xpm_alloc(pnm_xpm_t *xpm, int cols, int rows, int n_colors);
pnm_xpm_t *pnm_xpm_realloc(pnm_xpm_t *xpm, int cols, int rows, int n_colors);

pnm_xpm_t *pnm_xpm_fbox(pnm_xpm_t *xpm, int cols, int rows, pixel color);

char *     pnm_xpm_colorcode(char *line, int i);

pixel * pnm_xpm2colorhist(pnm_xpm_t *xpm, int *n_hist);
pixel * pnm_colorhist_join(pixel * hist, int *n_hist,
			   pixel *new_hist, int new_n_hist);

pnm_xpm_t *pnm_ppm2xpm(pnm_xpm_t *xpm, ppm_t *ppm);
pnm_xpm_t *pnm_xpm_insertPPM(pnm_xpm_t *xpm, ppm_t *ppm, int x, int y);
pnm_xpm_t *pnm_xpm_insertINT(pnm_xpm_t *xpm, int *data, int cols, int rows, int max_val,
			     int x, int y, int pseudo_color);
pnm_xpm_t *pnm_xpm_insertUCHAR(pnm_xpm_t *xpm, unsigned char *data, int cols, int rows, 
			       unsigned char max_val, int x, int y, int pseudo_color);
pnm_xpm_t *pnm_xpm_insertUCHARbyTab(pnm_xpm_t *xpm, unsigned char *data, 
				    int cols, int rows, 
				    unsigned char max_val, int x, int y,
				    int n_colors, pixel *colorTab);

void       pnm_xpm_fprint(FILE *fp, pnm_xpm_t *xpm);

#ifdef __cplusplus
}
#endif

#endif
