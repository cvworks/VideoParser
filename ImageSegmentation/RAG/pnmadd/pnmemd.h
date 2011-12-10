#ifndef PNMEMD_H
#define PNMEMD_H

#include <emd/libemd.h>
#include "pnmadd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int **map;
    int cols;
    int rows;
    int   n_labels;
    const int  *labels;
} pnm_labelmap_t;

pnm_labelmap_t *pnm_labelmap_init(pnm_labelmap_t *pgml,
				  const int *map, int cols, int rows,
				  int n_labels, const int *labels);
pnm_labelmap_t *pnm_labelmap_create(const int *map, int cols, int rows,
				     int n_labels, const int *labels);
void pnm_labelmap_free(pnm_labelmap_t *pgml);
void pnm_labelmap_destroy(pnm_labelmap_t *pgml);

/* -------------------------------------------------------------- */

float pnm_ppm_emd(ppm_t * ppm1, ppm_t * ppm2, int num, pixel bg);
float pnm_pgm_emd(pgm_t * pgm1, pgm_t * pgm2, int num, gray bg);
float pnm_labelmap_emd(pnm_labelmap_t *pgml1, 
		       pnm_labelmap_t *pgml2,
		       int num);

int pnm_ppm_bbox(int *x_min, int *y_min, int *x_max, int *y_max,
		 float *x_center, float *y_center, ppm_t *ppm, pixel bg);
int pnm_pgm_bbox(int *x_min, int *y_min, int *x_max, int *y_max,
		 float *x_center, float *y_center, pgm_t *pgm, gray  bg);
int pnm_labelmap_bbox(int *x_min, int *y_min, int *x_max, int *y_max,
		      float *x_center, float *y_center,
		      pnm_labelmap_t *pgml);

int pnm_ppm_numOfPix(ppm_t *ppm, pixel bg);
int pnm_pgm_numOfPix(pgm_t *pgm, gray  bg);
int pnm_labelmap_numOfPix(pnm_labelmap_t *pgml);

emd_signature_t *
pnm_ppm_signature(emd_signature_t *sig, ppm_t *ppm, int num, pixel bg);
emd_signature_t *
pnm_pgm_signature(emd_signature_t *sig, pgm_t *pgm, int num, gray bg);
emd_signature_t *
pnm_labelmap_signature(emd_signature_t *sig, pnm_labelmap_t *pgml, int num);

float pnm_ppm_sigWeight(ppm_t *ppm, float x, float y, float x_max,
			float y_max, float scale2, pixel bg);
float pnm_pgm_sigWeight(pgm_t *pgm, float x, float y, float x_max,
			float y_max, float scale2, gray bg);
float pnm_labelmap_sigWeight(pnm_labelmap_t *pgml, 
			     float x, float y, float x_max,
			     float y_max, float scale2);


#ifdef __cplusplus
}
#endif

#endif

