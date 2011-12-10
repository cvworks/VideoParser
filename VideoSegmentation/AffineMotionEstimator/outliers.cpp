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
#include<string.h>

#include "robust.h"
#include "gm2.h"

void outliers(float *Ix, float *Iy, float *It, float a0, float a1, float a2, 
			  float a3, float a4, float a5, int nx, int ny, float sigma, 
			  float *outlier_im)
{
	int index, i, j, x_cent, y_cent;
	float err, outlier, u, v;

	outlier = sigma_to_outlier(sigma);

	x_cent = (int)floor(nx/2.0);
	y_cent = (int)floor(ny/2.0);

	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			outlier_im[(i*nx)+j] = 0.0;
		}
	}

	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			index = (i*nx) + j;
			u = a0 + ((j-x_cent) * a1) + ((i-y_cent) * a2);
			v = a3 + ((j-x_cent) * a4) + ((i-y_cent) * a5);
			err = Ix[index]*u + Iy[index]*v + It[index];
			if (abs(err)>outlier) {
				outlier_im[index] = 1.0;
			}
		}
	}

}

void good_points(float *Ix, float *Iy, float *It, float a0, float a1, float a2, 
				 float a3, float a4, float a5, int nx, int ny, float sigma, 
				 float *outlier_im)
{
	int index, i, j, x_cent, y_cent;
	float err, outlier, u, v;

	outlier = sigma_to_outlier(sigma);

	x_cent = (int)floor(nx/2.0);
	y_cent = (int)floor(ny/2.0);

	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			index = (i*nx) + j;
			u = a0 + ((j-x_cent) * a1) + ((i-y_cent) * a2);
			v = a3 + ((j-x_cent) * a4) + ((i-y_cent) * a5);
			err = Ix[index]*u + Iy[index]*v + It[index];
			if (abs(err)>outlier) {
				outlier_im[index] = 0.0;
			}
		}
	}

}


void robust_to_weights(float *Ix, float *Iy, float *It, float a0, float a1, float a2, 
					   float a3, float a4, float a5, int nx, int ny, float sigma, 
					   float *weight_im)
{
	int index, i, j, x_cent, y_cent;
	float err, u, v, outlier, max_weight;

	x_cent = (int)floor((float)nx/2.0);
	y_cent = (int)floor((float)ny/2.0);

	outlier = sigma_to_outlier(sigma);
	max_weight = psi(outlier, sigma)/(outlier);

	//fprintf(stderr, "sigma %f, outlier %f, max_weight %f\n", 
	//	sigma, outlier, max_weight);

	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			index = (i*nx) + j;
			u = a0 + ((float)(j-x_cent) * a1) + ((float)(i-y_cent) * a2);
			v = a3 + ((float)(j-x_cent) * a4) + ((float)(i-y_cent) * a5);
			err = Ix[index]*u + Iy[index]*v + It[index];
			if (err==0.0) {
				weight_im[index] = max_weight;
			}
			else {
				weight_im[index] = (float)(psi(err, sigma) / (2.0 * err));
			}
		}
	}
}

