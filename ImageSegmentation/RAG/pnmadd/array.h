/*
 * pnmadd/array.h
 *
 * functions for int,char,uchar and pixval arrays
 *
 * Sven Wachsmuth, 24.02.2003
 */
#ifndef _PNMADD_ARRAY_H
#define _PNMADD_ARRAY_H

#include "pnmadd.h"

#ifdef __cplusplus
extern "C" {
#endif

int pnm_pgm_writeIntMap(FILE *pgm_fp, int *intMap, int cols, int rows,
			int maxval);
int *pnm_pgm_readIntMap(int *intMap, int *cols, int *rows, int *maxval,
			FILE *pgm_fp);

int pnm_pgm_writeCharMap(FILE *pgm_fp, unsigned char *charMap, int cols, 
			 int rows, unsigned char maxval);
char *pnm_pgm_readCharMap(unsigned char *charMap, int *cols, int *rows, 
			  unsigned char *maxval, FILE *pgm_fp);
 
void **pnm_array2matrix(void **matrix, int cols, int rows, void *array,
			size_t size);

#ifdef __cplusplus
}
#endif


#endif

