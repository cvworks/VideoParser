/************************************************************************

   File:		read_ppm.c

   Author(s):		Pavel Dimitrov

   Created:		23 Jun 2002

   Last Revision:	$Date: 2002/06/26 04:30:48 $

   Description:	

   $Revision: 1.1 $

   $Log: read_ppm.c,v $
   Revision 1.1  2002/06/26 04:30:48  pdimit
   After the failed attempt at getting the spline smoothing to work


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define READ_PPM_C

/**********************
     Include Files
 **********************/

/**********************
    Private Defines
 **********************/

/**********************
     Private Types
 **********************/

/**********************
    Private Variables
 **********************/

/**********************
    Private Functions
 **********************/

#include <stdio.h>

extern "C" {
	#include <ppm.h>
}

char* get_arr(int *rows, int *cols, char shapefile[]){

  char *arr;
  int k, i, j;
  
  pixval maxval;
  pixel **pixels;

  FILE* ppmshape = fopen(shapefile, "r");  

  pixels = ppm_readppm(ppmshape, cols, rows, &maxval);

  arr = (char*)malloc((*cols)*(*rows));


  for(i = 0, k = 0; i < *rows; i++)
    for(j = 0; j < *cols; j++, k++)
      {
	char r, g, b;
	
	r = (0xff * PPM_GETR(pixels[i][j])) / maxval;
	g = (0xff * PPM_GETR(pixels[i][j])) / maxval;
	b = (0xff * PPM_GETR(pixels[i][j])) / maxval;
	
	if(r==0 && b==0 && g==0)
	  arr[k] = 0;
	else
	  arr[k] = 0xff;
      }

  ppm_freearray(pixels, *rows);

  
  return arr;
}

/*  int main(int argc, char *argv[]){ */

/*    int r, c; */
/*    char *a = get_arr(&r,&c,argv[1]); */

/*    return 0; */
/*  } */
