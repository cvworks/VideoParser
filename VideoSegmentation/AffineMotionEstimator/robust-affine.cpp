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
#include <Tools/MathUtils.h>
#include <string.h>

#include "robust.h"
#include "gm2.h"
#include "basicUtils.h"

#define DBG_SHOW0(M, P0) fprintf(stderr, M, P0)
#define DBG_SHOW4(M, P0, P1, P2, P3) fprintf(stderr, M, P0, P1, P2, P3)

/*
This is the main routine that does the gradient descent on the
affine parameters.
*/
float affine_regress_1_iter(float *Ix, float *Iy, float *It, 
							float a0, float *a1, float *a2, float a3, 
							float *a4, float *a5, int nx, int ny, float sigma,
							float omega, float *omask)
{
	int index, i, j, x, y, x_cent, y_cent, num_outliers=0;
	float err, delta, max_delta=0.0, top, scale, gr;
	float outlier, u, v, dscale;
	//DBG_SHOW4("\nGiven values are: %f, %f, %f, %f\n", *a1, *a2, *a4, *a5);
	outlier = sigma_to_outlier(sigma);

	x_cent = (int)floor(nx/2.0);
	y_cent = (int)floor(ny/2.0);

	dscale = dpsi_max(sigma);

	top = 0.0; scale=0.0;
	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			x = j - x_cent;
			y = i - y_cent;
			index = (i*nx) + j;
			if (omask[index] >= 0.5) {
				u = a0 + (*a1*x) + (*a2*y);
				v = a3 + (*a4*x) + (*a5*y);
				err = (Ix[index]*u) + (Iy[index]*v) + It[index];
				gr = Ix[index];
				top += x * gr * psi(err, sigma);
				scale += (x*x*gr*gr)*dscale; 
			}
		}
	}
	delta = (omega * top/scale);
	max_delta = max(abs(delta), max_delta);
	*a1 -=  delta;

	top = 0.0; scale=0.0;
	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			x = j - x_cent;
			y = i - y_cent;
			index = (i*nx) + j;
			if (omask[index] >= 0.5) {
				u = a0 + (*a1*x) + (*a2*y);
				v = a3 + (*a4*x) + (*a5*y);
				err = (Ix[index]*u) + (Iy[index]*v) + It[index];
				gr = Ix[index];
				top += y * gr * psi(err, sigma);
				scale += (y*y*gr*gr)*dscale; 
			}
		}
	}
	delta = (omega * top/scale);
	max_delta = max(abs(delta), max_delta);
	*a2 -=  delta;

	top = 0.0; scale=0.0;
	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			x = j - x_cent;
			y = i - y_cent;
			index = (i*nx) + j;
			if (omask[index] >= 0.5) {
				u = a0 + (*a1*x) + (*a2*y);
				v = a3 + (*a4*x) + (*a5*y);
				err = (Ix[index]*u) + (Iy[index]*v) + It[index];
				gr = Iy[index];
				top += x * gr * psi(err, sigma);
				scale += (x*x*gr*gr)*dscale; 
			}
		}
	}
	delta = (omega * top/scale);
	max_delta = max(abs(delta), max_delta);
	*a4 -=  delta;

	top = 0.0; scale=0.0;
	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			x = j - x_cent;
			y = i - y_cent;
			index = (i*nx) + j;
			if (omask[index] >= 0.5) {
				u = a0 + (*a1*x) + (*a2*y);
				v = a3 + (*a4*x) + (*a5*y);
				err = (Ix[index]*u) + (Iy[index]*v) + It[index];
				if (abs(err)>outlier)
					num_outliers++;
				gr = Iy[index];
				top += y * gr * psi(err, sigma);
				scale += (y*y*gr*gr)*dscale; 
			}
		}
	}
	delta = (omega * top/scale);
	max_delta = max(abs(delta), max_delta);
	*a5 -=  delta;

	//DBG_SHOW4("Return values are: %f, %f, %f, %f\n", *a1, *a2, *a4, *a5);

	return(max_delta);
}


/* 
This just does the iterations calling affine_regress_1_iter
while lowering the sigma value according the annealing schedule.
*/
void regress_affine(float *im1, float *im2, float a0, float *a1, float *a2, 
					float a3, float *a4, float *a5, float *Ix, float *Iy, 
					float *It, int nx, int ny, int iters, float sigma, 
					float sigma_init, float omega, float *omask, float rate)
{
	int i;
	float max_delta, s;

	s = sigma_init;

	fprintf(stderr, "Affine;  a0: %f, a3: %f\n ", a0, a3);

	if ((nx>=MIN_AFFINE_SIZE) || (ny>=MIN_AFFINE_SIZE)) 
	{
		for(i=0;i<iters;i++)
		{
			max_delta =
				affine_regress_1_iter(Ix, Iy, It, a0, a1, a2, a3, a4, a5, nx, ny, 
				s, omega, omask);

			s = max((s * rate), sigma);
			
			//fprintf(stderr, ".");

			if (max_delta <= AFFINE_THRESHOLD) 
			{
				if (i > 0)
				{
					fprintf(stderr, "stopping affine regression (%d iters)\n", i);
				}

				i = iters; // ie, break
			}
		}
	}
	else
	{
		fprintf(stderr, "Region (%d, %d) too small for affine flow.\n",
		nx, ny);
	}

	//fprintf(stderr, "\n");
}  
