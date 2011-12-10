/***********************************************

Author contact information: 
    Michael J. Black
    Department of Computer Science
    Brown University 
    P.O. Box 1910
    Providence, RI 02912 

    http://www.cs.brown.edu/people/black/
    email: black@cs.brown.edu

    401-863-7637 (voice) 
    401-863-7657 (fax) 

---------------------------------------------------------------------
Copyright 1993, 2002 Michael J. Black and Allan Jepson

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any non-commercial purpose is hereby granted without
fee, provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the author not be used
in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

For commercial use and licensing information contact the author
at the address above
---------------------------------------------------------------------

File: warp.c

Synopsis: Warp an image based on the affine parameters (a0 -- a5).

Details: This does bi-linear interpolation.

************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <Tools/MathUtils.h>
#include <string.h>

#include "robust.h"
#include "basicUtils.h"

#define default 0.0

float machineEps = 1.0e-6f;

/*
Backwards warp
*/

/*
Backwards warp of "image" by (u,v).  The result is stored in "warped".
Pixel values are determined by linearly interpolating the warped image.
While this is fairly fast, a better interpolation may be desirable.
*/
void affine_warp_image(float *image, float *warped, float a0, float a1, 
		       float a2, float a3, float a4, float a5, int nx, int ny)
{
  float  new_i, new_j, remainder_i, remainder_j;
  int index, i, j, floor_i, floor_j, ceil_i, ceil_j;
  float matrix[2][2], b0, b1, b2, b3, b4, b5, det;
  int x_cent, y_cent;

  x_cent = (int)floor(nx/2.0);
  y_cent = (int)floor(ny/2.0);

  fprintf(stderr, "forwards warp %f %f %f %f %f %f\n", 
	  a0, a1, a2, a3, a4, a5);

  /** Compute backwards warp :

   Affine coordinate transform:
    ( x, y)^T = (new_x,new_y)^T + (a0, a3)^T + A * (new_x - x_cent, new_y- y_cent)^T
     = (x_cent,y_cent)^T + (a0, a3)^T + (I+A) * (new_x - x_cent, new_y- y_cent)^T

     Inverse transform
    (new_x - x_cent, new_y - y_cent)^T = 
      (I+A)^{-1} { (x - x_cent,y - y_cent)^T - (a0, a3)^T }

    SO:
    (new_x, new_y )^T 
     =(I+A)^{-1} { (x - x_cent,y - y_cent)^T - (a0, a3)^T } + (x_cent, y_cent)^T
     = (x_cent,y_cent)^T + (b0, b3)^T + (I+B) * (x - x_cent, y - y_cent)^T
     = (x,y)^T + (b0, b3)^T + B * (x - x_cent, y - y_cent)^T

    WITH:
     B = (I+A)^{-1} - I
     (b0 b3)^T =  -(I+B) * (a0, a3)^T
  *******************************************************************
    Form I+A:

  ********************/
  matrix[0][0] = 1 + a1;
  matrix[0][1] = a2;
  matrix[1][0] = a4;
  matrix[1][1] = 1 + a5;

  /* I+A inverse */
  det = matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
  if (abs(det) < machineEps) {
   fprintf(stderr, "error: affine transform nearly singular\n");
   exit(1);
  }
  matrix[0][0] /= det;
  matrix[0][1] /= -det;
  matrix[1][0] /= -det;
  matrix[1][1] /= det;
  /* swich diagonal elements */
  det = matrix[1][1];
  matrix[1][1] = matrix[0][0];
  matrix[0][0] = det;
  /*  now matrix is I+B == inverse(I+A) */
 
  b0 = -matrix[0][0] * a0 - matrix[0][1] * a3;
  b3 = -matrix[1][0] * a0 - matrix[1][1] * a3;
  
  b1 = matrix[0][0] - 1;
  b2 = matrix[0][1];
  b4 = matrix[1][0];
  b5 = matrix[1][1] - 1;
  
  fprintf(stderr, "backwards warp %f %f %f %f %f %f\n", 
	  b0, b1, b2, b3, b4, b5);
  for(i=0;i<ny;i++){
    for(j=0;j<nx;j++){
      index = (i*nx) + j;
      new_j = ((float) j) + b0 + ((j-x_cent) * b1) + ((i-y_cent) * b2);
      new_i = ((float) i) + b3 + ((j-x_cent) * b4) + ((i-y_cent) * b5);

      if (new_j < 0 || new_j > (nx - 1) || new_i < 0 || new_i > (ny - 1)) 
	warped[index] = default;
      else{
	floor_i = ((int) floor(new_i)) % ny;
	ceil_i = ((int) ceil(new_i)) % ny;
	remainder_i = new_i - ((float) floor_i);
	floor_j = ((int) floor(new_j)) % nx;
	ceil_j = ((int) ceil(new_j)) % nx;
	remainder_j = new_j - ((float) floor_j);
      
	warped[index] = 
	  linear_interp
	  (linear_interp(image[(floor_i*nx)+floor_j],
			 image[(ceil_i*nx)+floor_j],
			 remainder_i),
	   linear_interp(image[(floor_i*nx)+ceil_j],
			 image[(ceil_i*nx)+ceil_j],
			 remainder_i),
	   remainder_j);
      }
    }
  }
}

