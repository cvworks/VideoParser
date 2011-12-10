/*
 * pnmadd/array.c
 *
 * functions for int,char,uchar and pixval arrays
 *
 * Sven Wachsmuth, 24.02.2003
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "array.h"

int pnm_pgm_writeIntMap(FILE *pgm_fp, int *intMap, int cols, int rows,
			int maxval)
{
  gray **pgm_pixels;
  if (!pgm_fp || !intMap) return (-1);

  pgm_pixels = (gray **) pnm_array2matrix(NULL, cols, rows, intMap,
					  sizeof(int));
  pgm_writepgm(pgm_fp, pgm_pixels, cols, rows, maxval, 1);
  free(pgm_pixels);
  return (0);
}

int *pnm_pgm_readIntMap(int *intMap, int *cols, int *rows, int *maxval,
		       FILE *pgm_fp)
{
  gray **pgm_pixels; /** gray is defined as unsigned int (see pgm.h) */
  if (!pgm_fp) return (intMap);

  pgm_pixels = pgm_readpgm(pgm_fp, cols, rows, (gray *) maxval);
  
  if (!pgm_pixels) return (intMap);
  if (!intMap) {
    intMap = (int *) malloc((*cols)*(*rows) * sizeof(int));
  }
  {
    int i;
    int *intMap_ptr = intMap;
    for (i=0; i < *rows; i++, intMap_ptr+= *cols)
      memcpy(intMap_ptr, pgm_pixels[i], (*cols) * sizeof(int));
  }
  pgm_freearray(pgm_pixels, *rows);
  return (intMap);
}

int pnm_pgm_writeCharMap(FILE *pgm_fp, unsigned char *charMap, int cols, 
			 int rows, unsigned char maxval)
{
  int i;
  gray *pgm_pixels;
  unsigned char *charMap_ptr;

  if (!pgm_fp || !charMap) return (-1);

  pgm_writepgminit(pgm_fp, cols, rows, maxval, 1);

  pgm_pixels = (gray *) pgm_allocrow(cols);
  charMap_ptr = charMap;
  for (i=0; i < rows; i++) {
    int j;
    for (j=0; j < cols; j++) pgm_pixels[j] = (gray) charMap_ptr[j];
    pgm_writepgmrow(pgm_fp, pgm_pixels, cols, maxval, 1);
    charMap_ptr += cols;
  }
  pgm_freerow(pgm_pixels);

  return (0);
}

char *pnm_pgm_readCharMap(unsigned char *charMap, int *cols, int *rows, 
			  unsigned char *maxval, FILE *pgm_fp)
{
  gray **pgm_pixels; /** gray is defined as unsigned int (see pgm.h) */
  gray _maxval;
  if (!pgm_fp) return (charMap);

  pgm_pixels = pgm_readpgm(pgm_fp, cols, rows, &_maxval);
  *maxval = (unsigned char) _maxval;
  
  if (!pgm_pixels) return (charMap);
  if (!charMap) {
    charMap = (unsigned char *) malloc((*cols)*(*rows) * sizeof(char));
  }
  {
    int i,j;
    char *charMap_ptr = charMap;
    for (i=0; i < *rows; i++)
      for (j=0; j < *cols; j++)
	*(charMap_ptr++) = (unsigned char) pgm_pixels[i][j];
  }
  pgm_freearray(pgm_pixels, *rows);
  return (charMap);
}

void **pnm_array2matrix(void **matrix, int cols, int rows, void *array,
			size_t size)
{
  int i,j;
  if (!array || size == 0 || cols <= 0 || rows <= 0)
    return (matrix);
  if (!matrix)
    matrix = (void **) malloc(rows * sizeof(void *));
  
  matrix[0] = array;
  for (i=1,j=cols; i < rows; i++,j+=cols)
    matrix[i] = ((char *) array) + j * size;

  return (matrix);
}

