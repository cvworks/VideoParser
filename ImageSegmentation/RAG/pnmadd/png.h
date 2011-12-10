/*
 * pnmadd/png.c
 *
 * reading and writing ppm_t as png
 *
 * Sven Wachsmuth, 16.7.2003
 */
#ifndef _PNMADD_PNG_H
#define _PNMADD_PNG_H

#include "pnmadd.h"

#define PNM_PNG_OK (0)
#define PNM_PNG_NOMEM (-1)
#define PNM_PNG_ERROR (-2)
#define PNM_PNG_NOPNG (-3)

#ifdef __cplusplus
extern "C" {
#endif

extern int pnm_png_verbose;

int pnm_ppm_readImage(ppm_t *ppm, FILE *fp, const char *prefix);
int pnm_pgm_readImage(pgm_t *pgm, FILE *fp, const char *prefix);

int pnm_ppm_readpng(ppm_t *ppm, FILE *fp);
int pnm_pgm_readpng(pgm_t *pgm, FILE *fp);
int pnm_ppm_writepng(FILE *fp, ppm_t *ppm);
int pnm_pgm_writepng(FILE *fp, pgm_t *pgm);

#ifdef __cplusplus
}
#endif

#endif
