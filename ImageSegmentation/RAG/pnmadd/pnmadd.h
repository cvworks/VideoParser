#ifndef _PNMADD_H
#define _PNMADD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#include "pnm.h"
#else
#include <pnm.h>
#endif

typedef unsigned char pnm_pixval;

typedef struct pgm_t { 
  int cols;
  int rows;
  gray maxval;
  gray **pixels;
} pgm_t;

typedef struct ppm_t {
  int cols;
  int rows;
  pixval maxval;
  pixel **pixels;
} ppm_t;

typedef struct pnm_image_t {
  int cols;
  int rows;
  int channels;
  pnm_pixval maxval;
  pnm_pixval ***pixels;
} pnm_image_t;


ppm_t *pnm_ppm_create(void);
ppm_t *pnm_ppm_init(ppm_t *ppm);
ppm_t *pnm_ppm_alloc(ppm_t *ppm, int cols, int rows);
ppm_t *pnm_ppm_scale(ppm_t *copy, ppm_t *ppm, int factor);
ppm_t *pnm_ppm_clear(ppm_t *ppm, pixel value);
void   pnm_ppm_free(ppm_t *ppm);
void   pnm_ppm_destroy(ppm_t *ppm);

pgm_t *pnm_pgm_create(void);
pgm_t *pnm_pgm_init(pgm_t *pgm);
pgm_t *pnm_pgm_alloc(pgm_t *pgm, int cols, int rows);
pgm_t *pnm_pgm_scale(pgm_t *copy, pgm_t *pgm, int factor);
pgm_t *pnm_pgm_clear(pgm_t *pgm, gray value);
void   pnm_pgm_free(pgm_t *pgm);
void   pnm_pgm_destroy(pgm_t *pgm);

unsigned char *pnm_uchar_scale(unsigned char *copy, unsigned char *data, 
			       int cols, int rows, int factor);
int *          pnm_int_scale(int *copy, int *data, int cols, int rows, int factor);


pnm_image_t *pnm_image_create(void);
pnm_image_t *pnm_image_init(pnm_image_t *image);
pnm_image_t *pnm_image_alloc(pnm_image_t *image, 
			     int channels, int cols, int rows);
int          pnm_image_pgmWrite(FILE *fp, pnm_image_t *image);
pnm_image_t *pnm_image_pgmRead(pnm_image_t *image, FILE *fp);

pnm_image_t *pnm_image_set(pnm_image_t *image,
			   int cols, int rows, int channels,
			   pnm_pixval maxval, pnm_pixval ***pixels);

pnm_image_t *pnm_ppm2image(pnm_image_t *image, ppm_t *ppm);
ppm_t       *pnm_image2ppm(ppm_t *ppm, pnm_image_t *image);

#ifdef __cplusplus
}
#endif
#endif
