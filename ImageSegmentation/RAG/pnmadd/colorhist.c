#include "colorhist.h"
#include <list/hashmap.h>

long int pnm_ppm_pix2long(pixel pix)
{
#ifdef PPM_PACKCOLORS
    return pix;
#else
    long int fin;
    fin = pix.r + pix.g*256 + pix.b*65536;
    /* (((long int) pix.b)<<16) | (((long int) pix.g)<<8) | ((long int) pix.r)*/
    return fin;
#endif
}

int pnm_ppm_colorHashPix(ls_hashmap_t *hash, pixel *pixrow, int n_pixrow)
{
    int i,j=0;
    for (i=0; i < n_pixrow; i++) {
	long pixlong = pnm_ppm_pix2long(pixrow[i]);
	if (!ls_hashmap_exists(hash, pixlong))
	    ls_hashmap_put(hash, pixlong, j++);
    }
    return (j);
}

int pnm_ppm_colorHash(ls_hashmap_t *hash, pixel **colorTab, int *max,
		      int *colorMap, ppm_t *ppm)
{
    int i,k,n;
    ls_hashmap_initSTD(hash);

    
    for (i = 0, n=0; i < ppm->rows ; i++) {
	pixel *current_row = ppm->pixels[i];
	int j;
	for (j = 0;j<ppm->cols; j++){
	    pixel current_pixel = current_row[j];
	    long int current_long = pnm_ppm_pix2long(current_pixel);
	    int current_value = ls_hashmap_getValue(hash, current_long, -1);

	    if (current_value < 0) {
		
		if (colorTab && max) {
		    if (*colorTab) {
			if (n >= *max) {
			    (*colorTab) = (pixel *)  
				realloc(*colorTab, ((*max)+= 16) * sizeof(pixel));
			}
		    } else {
			(*colorTab) = (pixel *)
			    malloc((*max = 16) * sizeof(pixel));
		    }
		    /** ACHTUNG: index von colorTab startet bei 0,
		     *     colorMap-eintraege starten bei 1.
		     */
		    (*colorTab)[n] = current_pixel;
		}
		current_value = n;
		ls_hashmap_put(hash, current_long, n++);
	    }
	    if (colorMap)
		colorMap[k++] = current_value+1;
	}
    }
    return (n);
}

pixel *pnm_ppm_colorTab(pixel *colorTab, int *n_colors, int *max_colors,
			int *colorMap, ppm_t *ppm)
{
    ls_hashmap_t hash;
    if (!colorTab)
	colorTab = (pixel*) malloc(((*max_colors) = 16) * sizeof(pixel));

    *n_colors = pnm_ppm_colorHash(&hash, &colorTab, max_colors, colorMap, ppm);

    return (colorTab);
}

int pnm_ppm_colorMap(int *colorMap, ppm_t *ppm)
{
    ls_hashmap_t hash;
    if (!colorMap) return (0);
    
    return (pnm_ppm_colorHash(&hash, NULL, NULL, colorMap, ppm));
}

int pnm_ppm_colorHashChar(ls_hashmap_t *hash, pixel **colorTab, int *max,
			  char *colorMap, ppm_t *ppm)
{
    int i,k,n;
    ls_hashmap_initSTD(hash);

    
    for (i = 0, n=0, k=0; i < ppm->rows ; i++) {
	pixel *current_row = ppm->pixels[i];
	int j;
	for (j = 0;j<ppm->cols; j++){
	    pixel current_pixel = current_row[j];
	    long int current_long = pnm_ppm_pix2long(current_pixel);
	    int current_value = ls_hashmap_getValue(hash, current_long, -1);

	    if (current_value < 0) {
		
		if (colorTab && max) {
		    if (*colorTab) {
			if (n >= *max) {
			    (*colorTab) = (pixel *)  
				realloc(*colorTab, ((*max)+= 16) * sizeof(pixel));
			}
		    } else {
			(*colorTab) = (pixel *)
			    realloc(*colorTab, ((*max) = 16) * sizeof(pixel));
		    }
		    /** ACHTUNG: index von colorTab startet bei 0,
		     *     colorMap-eintraege starten bei 1. 
		     */
		    (*colorTab)[n] = current_pixel;
		}
		current_value = n;
		ls_hashmap_put(hash, current_long, n++);
	    }
	    if (colorMap)
		colorMap[k++] = current_value+1;
	}
    }
    return (n);
}

pixel *pnm_ppm_colorTabChar(pixel *colorTab, int *n_colors, int *max_colors,
			    char *colorMap, ppm_t *ppm)
{
    ls_hashmap_t hash;
    if (!colorTab)
	colorTab = (pixel*) malloc(((*max_colors) = 16) * sizeof(pixel));

    *n_colors = pnm_ppm_colorHashChar(&hash, &colorTab, max_colors, colorMap, ppm);

    return (colorTab);
}

int pnm_ppm_colorMapChar(char *colorMap, ppm_t *ppm)
{
    ls_hashmap_t hash;
    if (!colorMap) return (0);
    
    return (pnm_ppm_colorHashChar(&hash, NULL, NULL, colorMap, ppm));
}





