#ifndef PNM_COLORHIST
#define PNM_COLORHIST

#include "pnmadd.h"

#ifdef __cplusplus
extern "C" {
#endif

long int pnm_ppm_pix2long(pixel p);

pixel *pnm_ppm_colorTab(pixel *colorTab, int *n_colors, int *max_colors,
			   int *colorMap, ppm_t *ppm);
int pnm_ppm_colorMap(int *colorMap, ppm_t *ppm);

pixel *pnm_ppm_colorTabChar(pixel *colorTab, int *n_colors, int *max_colors,
			   char *colorMap, ppm_t *ppm);
int pnm_ppm_colorMapChar(char *colorMap, ppm_t *ppm);

#ifdef __cplusplus
}
#endif

#endif





