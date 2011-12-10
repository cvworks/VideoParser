#ifndef PNM_PSEUDO_H
#define PNM_PSEUDO_H

#include "pnmadd.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int pnm_bg_pixel[3];

void pnm_getPseudoColor(int *r, int *g, int *b, int _value);

ppm_t *pnm_intMap2ppmPseudoColor(ppm_t *ppm_pseudo, 
				 int *intMap, int cols, int rows);
ppm_t *pnm_charMap2ppmPseudoColor(ppm_t *ppm_pseudo, 
				  unsigned char *charMap, int cols, int rows);
int pnm_ppmPseudoColor2intMap(int *intMap, ppm_t *ppm);
void pnm_setIntMargin(int *intMap, int cols, int rows, int value);
void pnm_setIntFrame(int *intMap, int cols, int rows, int offset, int value);

int pnm_ppmSegColor2intMap(int *intMap, ppm_t *ppm, pixel **colors);
int pnm_ppmSegColor2charMap(unsigned char *charMap, ppm_t *ppm, pixel**colors);

ppm_t *pnm_charMap2ppmTabColor(ppm_t *ppm,
			       unsigned char *charMap, int cols, int rows,
			       int n_colors, pixel *colorTab);

int pnm_ppmPseudoColor2charMap(unsigned char *charMap, ppm_t *ppm);
void pnm_setCharMargin(unsigned char *charMap, int cols, int rows, 
		       unsigned char value);

ppm_t *pnm_ppm_darkenByIntMap(ppm_t *ppm, int *map, int n_labels, 
			      const int *labels, float factor);

#ifdef __cplusplus
}
#endif

#endif
