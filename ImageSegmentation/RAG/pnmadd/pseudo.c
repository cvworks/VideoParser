/*
 * pnmadd/pseudo.c
 *
 * conversion of pseudo-color images
 *
 * Sven Wachsmuth, 24.02.2003
 * Sven Wachsmuth, 11.07.2003 (pnm_setIntFrame added)
 */
#include "pseudo.h"
#include "colorhist.h"
#include "label.h"

#define PNM_MAXPSEUDOVALUE (9)

int pnm_bg_pixel[3] = { 255,255,255 };

static int _pnm_pseudoValue[PNM_MAXPSEUDOVALUE] = {
  0, 255, 127, 63, 191, 31, 95, 159, 223
};

static int _pnm_numOfColorsForValue[PNM_MAXPSEUDOVALUE] = {
  1, 8, 27, 64, 125, 216, 343, 512, 729   /* n^3 */
};

#define PNM_MAXPSEUDOCOLOR (27)
 
static int _pnm_pseudoColor[PNM_MAXPSEUDOCOLOR][3] = {
  { 0,0,0 },    /* 0 */
  { 1,1,1 },    /* 1 */
  { 1,0,0 },    /* 0,1 */
  { 0,1,0 },
  { 0,0,1 },
  { 1,1,0 },
  { 1,0,1 },
  { 0,1,1 },
  { 2,2,2 },    /* 8: 2 */
  { 2,0,0 },    /* 9: 0,2 */
  { 0,2,0 },
  { 0,0,2 },
  { 2,2,0 },
  { 2,0,2 },
  { 0,2,2 },
  { 2,1,0 },    /* 15: 0,1,2 */
  { 2,0,1 },
  { 1,2,0 },
  { 0,2,1 },
  { 1,0,2 },
  { 0,1,2 },
  { 1,2,2 },    /* 1,2 */
  { 2,1,2 },
  { 2,2,1 },
  { 2,1,1 },
  { 1,2,1 },
  { 1,1,2 } };

void pnm_getPseudoColor(int *r, int *g, int *b, int _value)
{
  /** above the maximum value colors are used twice */
  int value = _value % _pnm_numOfColorsForValue[PNM_MAXPSEUDOVALUE-1];
  int index[3];
  int i,maxColor;

  /** figure out how many colors are needed: value < maxColor^3) */
  for (maxColor = 0; 
       value >= _pnm_numOfColorsForValue[maxColor]; 
       maxColor++);

  /** if only the first three colors are needed, the coding is pre-defined */
  if (maxColor < 3) {
    index[0] = 0;
    index[1] = 1;
    index[2] = 2;
  } else {
    /* ... otherwise calculate coding:
     * may appear as single color: 1 possibility
     * may appear with one other color: 6 possibilities x #c (colors before)
     * may appear with two other colors: 6 possibilities x (#c * (#c-1) / 2)
     * resulting in (#c = maxColor):
     *    (#c * (#c+1) / 2) * 6 + 1 possibilities
     */
    index[2] = maxColor; /* maxColor is included in all possibilities */

    /* calculate index of coding for maxColor */
    value -= _pnm_numOfColorsForValue[maxColor-1];

    /* if first coding, then select <maxColor,maxColor,maxColor> */
    if (value == 0) {
      value = 8; /* pseudoColor: 2,2,2 */
    } else {
      /* ... otherwise compute index for color selection <select> and
       *     index for coding scheme <modus> */
      int modus, select;
      value--;   /* subtract first coding  possibility <2,2,2> */

      modus = value % 6;   /* there are 6 different coding schemes ... */
      select = value / 6;  /* ... and (#c * (#c+1) / 2) color selections */

      /* compute color selection:
       * (select an element of an upper triangular matrix (size maxColor))
       */
      for (i=0; select >= maxColor-i; i++) select -= (maxColor-i);
      index[0] = i;
      index[1] = select+i;

      /* if a diagonal element is selected, coding schemes 9..14 are used
       * ... otherwise coding schemes 15..20 are used
       */
      value = ((index[0]==index[1]) ? 9 : 15) + modus;
    }
  }
  *r = _pnm_pseudoValue[index[_pnm_pseudoColor[value][0]]];
  *g = _pnm_pseudoValue[index[_pnm_pseudoColor[value][1]]];
  *b = _pnm_pseudoValue[index[_pnm_pseudoColor[value][2]]];
}

ppm_t *pnm_intMap2ppmPseudoColor(ppm_t *ppm_pseudo, 
				 int *intMap, int cols, int rows)
{
  int i,j;
  if (!intMap) return (ppm_pseudo);

  if (!ppm_pseudo) ppm_pseudo = pnm_ppm_alloc(NULL,cols,rows);

  for (i=0; i < rows; i++) {
    for (j=0; j < cols; j++) {
      int value = intMap[cols * i + j];
      int r,g,b;
      pnm_getPseudoColor(&r,&g,&b,value);

      PPM_ASSIGN(ppm_pseudo->pixels[i][j], r, g, b); 
    }
  }
  ppm_pseudo->maxval = 255;
  return (ppm_pseudo);
}

ppm_t *pnm_ppm_darkenByIntMap(ppm_t *ppm, int *map, 
			      int n_labels, const int *labels,
			      float factor)
{
    int labelMax = -1;
    int *labelFlags = NULL;
    int i,j,k;
    
    if (!ppm || !map || !labels) return (ppm);

    labelFlags = pnm_label_createLookup(&labelMax, n_labels, labels);

    if (!labelFlags) return (ppm);

    for (i=0, k=0; i < ppm->rows; i++) {
	int j;
	for (j=0; j < ppm->cols; j++, k++) {
	    if (pnm_label_lookup(labelMax, labelFlags, map[k])) {
		pixel p = ppm->pixels[i][j];
		int r = (int) (factor * (float) PPM_GETR(p));
		int g = (int) (factor * (float) PPM_GETG(p));
		int b = (int) (factor * (float) PPM_GETB(p));
		PPM_ASSIGN(ppm->pixels[i][j], r,g,b);
	    }
	}
    }
    free(labelFlags);
    return (ppm);
}

ppm_t *pnm_charMap2ppmPseudoColor(ppm_t *ppm_pseudo, 
				  unsigned char *charMap, int cols, int rows)
{
  int i,j;
  if (!charMap) return (ppm_pseudo);

  if (!ppm_pseudo) ppm_pseudo = pnm_ppm_alloc(NULL,cols,rows);

  for (i=0; i < rows; i++) {
    for (j=0; j < cols; j++) {
      unsigned char value = charMap[cols * i + j];
      int r,g,b;
      pnm_getPseudoColor(&r,&g,&b,value);

      PPM_ASSIGN(ppm_pseudo->pixels[i][j], r, g, b); 
    }
  }
  ppm_pseudo->maxval = 255;
  return (ppm_pseudo);
}

ppm_t *pnm_charMap2ppmTabColor(ppm_t *ppm,
			       unsigned char *charMap, int cols, int rows,
			       int n_colors, pixel *colorTab)
{
  int i,j;
  if (!charMap) return (ppm);
  ppm = pnm_ppm_alloc(ppm,cols,rows);
  
  for (i=0; i < rows; i++) {
    for (j=0; j < cols; j++) {
      unsigned char value = charMap[cols * i + j];
      
      if (value > 0 && value <= n_colors) {
	  /*PPM_ASSIGN(ppm->pixels[i][j],value,value,value);*/
	  ppm->pixels[i][j] = colorTab[value-1];
      } else {
	  PPM_ASSIGN(ppm->pixels[i][j],
		     pnm_bg_pixel[0],pnm_bg_pixel[0],pnm_bg_pixel[0]);
      }
    }
  }
  ppm->maxval = 255;
  return (ppm);
}

int pnm_ppmPseudoColor2intMap(int *intMap, ppm_t *ppm)
{
    if (!intMap || !ppm) return (0);

    return (pnm_ppm_colorMap(intMap, ppm));
}


int pnm_ppmSegColor2intMap(int *intMap, ppm_t *ppm, pixel **colors)
{
  int n_hist, max_hist=0;
  
  if (!intMap || !ppm) return (0);

  if (colors) {
      (*colors) = pnm_ppm_colorTab(*colors, &n_hist, &max_hist, intMap, ppm);
  } else {
      n_hist = pnm_ppm_colorMap(intMap, ppm);
  }
  return (n_hist+1);
}

void pnm_setIntMargin(int *intMap, int cols, int rows, int value)
{
  int i;
  int *intMap_ptr1 = intMap;
  int *intMap_ptrN = &intMap[cols * (rows - 1)];
  
  if (!intMap) return;

  for (i=0; i < cols; i++)
    *(intMap_ptrN++) = *(intMap_ptr1++) = value;

  intMap_ptr1 = &intMap[cols];
  intMap_ptrN = &intMap[2 * cols - 1];
  rows -= 2;
  for (i=0; i < rows; i++, intMap_ptr1+=cols, intMap_ptrN+=cols)
    *intMap_ptrN = *intMap_ptr1 = value;
}

void pnm_setIntFrame(int *intMap, int cols, int rows, int offset, int value)
{
  int i;
  int *intMap_ptr1 = &intMap[cols * offset + offset];
  int *intMap_ptrN = &intMap[cols * (rows - 1 - offset) + offset];
  int n = cols - 2 * offset;
  int m = rows - 2 * offset;

  if (!intMap) return;

  for (i=0; i < n; i++)
    *(intMap_ptrN++) = *(intMap_ptr1++) = value;

  intMap_ptr1 = &intMap[(offset+1)*cols + offset];
  intMap_ptrN = &intMap[(offset+2) * cols - 1 - offset];
  m -= 2;
  for (i=0; i < m; i++, intMap_ptr1+=cols, intMap_ptrN+=cols)
    *intMap_ptrN = *intMap_ptr1 = value;
}

int pnm_ppmPseudoColor2charMap(unsigned char *charMap, ppm_t *ppm)
{
    if (!charMap || !ppm) return (0);

    return (pnm_ppm_colorMapChar(charMap, ppm));
}

int pnm_ppmSegColor2charMap(unsigned char *charMap, ppm_t *ppm, pixel **colors)
{
  int n_hist, max_hist=0;
  
  if (!charMap || !ppm) return (0);

  if (colors) {
      (*colors) = pnm_ppm_colorTabChar(*colors,&n_hist,&max_hist,charMap,ppm);
  } else {
      n_hist = pnm_ppm_colorMapChar(charMap, ppm);
  }
  /* return value corresponds to values in charMap 0 < x < n_hist+1 
   * colors are defined for values 0 <= x < n_hist */
  return (n_hist+1);
}

void pnm_setCharMargin(unsigned char *charMap, int cols, int rows, 
		       unsigned char value)
{
  int i;
  unsigned char *charMap_ptr1 = charMap;
  unsigned char *charMap_ptrN = &charMap[cols * (rows - 1)];
  
  if (!charMap) return;

  for (i=0; i < cols; i++)
    *(charMap_ptrN++) = *(charMap_ptr1++) = value;

  charMap_ptr1 = &charMap[cols];
  charMap_ptrN = &charMap[2 * cols - 1];
  rows -= 2;
  for (i=0; i < rows; i++, charMap_ptr1+=cols, charMap_ptrN+=cols)
    *charMap_ptrN = *charMap_ptr1 = value;
}
