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

/* 
gradient descent for a single translational motion component
*/
float regress_1_iter(float *grad, float *Ix, float *Iy, float *It, 
					 float u, float v, int nx, int ny, float sigma,
					 float omega, float *omask, 
					 int x, int y, int dx, int dy)
{
	int index, i, j, num_outliers=0, num_good=0;
	float err, delta=0.0, top, bot, gr;
	float outlier, scale, ttmp;

	outlier = sigma_to_outlier(sigma);
	scale = dpsi_max(sigma);

	top = 0.0; 
	bot=0.0;
	for(i=y;i<y+dy;i++){
		for(j=x;j<x+dx;j++){
			index = (i*nx) + j;
			if (omask[index] >= 0.5) {
				num_good++;
				gr = grad[index];
				err = (Ix[index]*u) + (Iy[index]*v) + It[index];

				if (abs(err)>outlier)
					num_outliers++;

				ttmp = psi(err, sigma)*gr;
				top += ttmp;
				bot += (gr*gr)* scale;  
			}
		}
	}
	if (bot==0.0) {
		delta = 0.0;
		fprintf(stderr, "*****No Points Used (inliers=%d, outliers=%d)*****\n", num_good, num_outliers);
	}
	else
		delta = - (omega * top/bot);
	return delta;
}

/*
preform iters interations of gradient descent for translation 
motion components
*/
void robust_regress_region(float *im1, float *im2, float *u, float *v, 
						   float *Ix, float *Iy, float *It, int nx, int ny,
						   int iters, float sigma, float sigma_init, 
						   float omega, float *omask, 
						   int x, int y, int dx, int dy, float rate)
{
	int i;
	float du, dv, s;

	s = sigma_init;

	for(i=0;i<iters;i++)
	{
		//fprintf(stderr, ".");

		du= regress_1_iter(Ix, Ix, Iy, It, *u, *v, nx, ny, 
			s, omega, omask, x, y, dx, dy);

		*u += du;
		dv = regress_1_iter(Iy, Ix, Iy, It, *u, *v, nx, ny, 
			s, omega, omask, x, y, dx, dy);
		*v += dv;
		s = max((s * rate), sigma);

		if ((abs(du)<STOP_THRESHOLD) && (abs(dv)<STOP_THRESHOLD)) 
		{
			if (i > 0)
				fprintf(stderr, "stopping robust regression (%d iters)\n", i);

			i = iters;
		}
	} 

	//fprintf(stderr, "\n");
}  

void robust_regress_image(float *im1, float *im2, float *u, float *v, 
						  float *Ix, float *Iy, float *It, int nx, int ny,
						  int iters, float sigma, float sigma_init, 
						  float omega, float *omask, float rate)
{
	robust_regress_region(im1, im2, u, v, Ix, Iy, It, nx, ny,
		iters, sigma, sigma_init, omega, 
		omask, 0, 0, nx, ny, rate);
}

