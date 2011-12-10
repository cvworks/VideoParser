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
Copyright 1993, 2002 Michael J. Black

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

************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <Tools/MathUtils.h>
#include <string.h>

#include "robust.h"
#include "pyramid.h"
#include "basicUtils.h"
#include "outliers.h"
#include "affine-warp.h"
#include "robust-translation.h"
#include "robust-affine.h"
#include "derivatives.h"

void pyramid_regress(float *im1, float *im2, int level,
		     float *a0, float *a1, float *a2, float *a3, 
		     float *a4, float *a5, float *Ix, float *Iy, float *It, 
		     int nx, int ny, int iters, float sigma, float sigma_init, 
		     float omega, float *omask, float *weights, int filter, 
		     int affine, float rate)
{
  float *p_im1, *p_im2, *p_out_im, *warp, *lap1, *lap2;
  int size, small_size;
  float da0=0.0, da1=0.0, da2=0.0, da3=0.0, da4=0.0, da5=0.0;
  /*double floor();*/

  fprintf(stderr," nx %d, ny %d\n", nx, ny); 
  fprintf(stderr, "initial values: %f %f %f %f %f %f\n", 
	  *a0, *a1, *a2, *a3, *a4, *a5);

  size = nx * ny * sizeof( float );
  small_size = (int) floor(nx/2.0) * (int)floor(ny/2.0) * sizeof( float );
  if ((p_im1 = (float *) malloc((size_t) small_size)) == NULL)
    error(" Unable to allocate memory for ", "p_im1");
  if ((p_im2 = (float *) malloc((size_t) small_size)) == NULL)
    error(" Unable to allocate memory for ", "p_im2");
  if ((p_out_im = (float *) malloc((size_t) small_size)) == NULL)
    error(" Unable to allocate memory for ", "p_out_im");
  if ((warp = (float *) malloc((size_t) size)) == NULL)
    error(" Unable to allocate memory for ", "warp");
  if ((lap1 = (float *) malloc((size_t) size)) == NULL)
    error(" Unable to allocate memory for ", "lap1");
  if ((lap2 = (float *) malloc((size_t) size)) == NULL)
    error(" Unable to allocate memory for ", "lap2");

  if (level <= 0) {
    affine_warp_image(im1, warp, *a0, *a1, *a2, *a3, *a4, *a5, nx, ny); 
    if (filter == 1) {
      cmvu_laplacian(warp, lap1, nx, ny);
      cmvu_laplacian(im2, lap2, nx, ny);
    }
    else {
      copy_image(lap1, warp, nx, ny);
      copy_image(lap2, im2, nx, ny);
    }
    horn_dx(lap1, lap2, Ix, nx, ny);
    horn_dy(lap1, lap2, Iy, nx, ny);
    horn_dt(lap1, lap2, It, nx, ny);

    robust_regress_region(lap1, lap2, &da0, &da3, Ix, Iy, It, nx, ny, 
			  iters, sigma, sigma_init, omega, omask, 
			  0, 0, nx, ny, rate);
    *a0 += da0; *a3 += da3; 
    if (affine==1) {
      regress_affine(lap1, lap2, da0, &da1, &da2, da3, &da4, &da5,
		     Ix, Iy, It, nx, ny, 
		     iters, sigma, sigma_init, omega, omask, rate);
      *a1 += da1; *a2 += da2;
      *a4 += da4; *a5 += da5;
    }
  }
  else {
    /* recurse */
    reduce_image(im1, p_im1, nx, ny);
    fprintf(stderr, "reduced\n");
    reduce_image(im2, p_im2, nx, ny);
    fprintf(stderr, "reduced\n");
    reduce_image(omask, p_out_im, nx, ny);
    fprintf(stderr, "reduced\n");
    *a0 = (*a0) / 2.0f;  
    *a3 = (*a3) / 2.0f;

    fprintf(stderr, "recursing...\n");
    pyramid_regress(p_im1, p_im2, (level - 1),
		    a0, a1, a2, a3, a4, a5,
		    Ix, Iy, It, (int)floor(nx / 2.0), (int)floor(ny / 2.0), 
		    iters, sigma, sigma_init, omega, 
		    p_out_im, weights, filter, affine, rate);
    *a0 = *a0 * 2.0f; 
    *a3 = *a3 * 2.0f; 

    fprintf(stderr, "returned %f %f %f %f %f %f\n", 
	  *a0, *a1, *a2, *a3, *a4, *a5);
    
    /* update */
    affine_warp_image(im1, warp, *a0, *a1, *a2, *a3, *a4, *a5, nx, ny);
    if (filter == 1) {
      cmvu_laplacian(warp, lap1, nx, ny);
      cmvu_laplacian(im2, lap2, nx, ny);
    }
    else {
      copy_image(lap1, warp, nx, ny);
      copy_image(lap2, im2, nx, ny);
    }
    horn_dx(lap1, lap2, Ix, nx, ny);
    horn_dy(lap1, lap2, Iy, nx, ny);
    horn_dt(lap1, lap2, It, nx, ny);
    
    da0 = 0.0; da3=0.0;
    robust_regress_region(lap1, lap2, &da0, &da3, Ix, Iy, It, nx, ny, 
			  iters, sigma, sigma_init, omega, omask, 
			  0, 0, nx, ny, rate);
    *a0 += da0; *a3 += da3; 
    if (affine==1) {
      regress_affine(lap1, lap2, da0, &da1, &da2, da3, &da4, &da5,
		     Ix, Iy, It, nx, ny, 
		     iters, sigma, sigma_init, omega, omask, rate);
      *a1 += da1; *a2 += da2;
      *a4 += da4; *a5 += da5;
    }

    fprintf(stderr, "updated %f %f %f %f %f %f\n", 
	    *a0, *a1, *a2, *a3, *a4, *a5);
  }
    
  horn_dx(lap1, lap2, Ix, nx, ny);
  horn_dy(lap1, lap2, Iy, nx, ny);
  horn_dt(lap1, lap2, It, nx, ny);
  good_points(Ix, Iy, It, da0, da1, da2, da3, da4, da5, nx, ny, sigma, omask);

  robust_to_weights(Ix, Iy, It, da0, da1, da2, da3, da4, da5, nx, ny, sigma, weights);
    
  free(p_im1);
  free(p_im2);
  free(p_out_im);
  free(warp);
  free(lap1);
  free(lap2);
}


