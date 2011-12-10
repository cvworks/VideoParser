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

Synopsis: Compute image derivatives (dx, dy, dt).

Details: None of these are very good.  The simple ones
are from Horn's book.

************************************************/

#include <stdio.h>
#include <Tools/MathUtils.h>
#include <string.h>

#include "robust.h"
#include "basicUtils.h"

/* 
laplacian from page 167 of Horn 
*/
void laplacian(float *image, float *lap, int nx, int ny)
{
	int index, i, j;

	for(i=1;i<ny-1;i++){
		for(j=1;j<nx-1;j++){
			index = (i*nx) +j;
			lap[index] = (float)((image[(i-1)*nx + (j-1)] + 
				image[(i-1)*nx + (j+1)] + 
				image[(i+1)*nx + (j+1)] + 
				image[(i+1)*nx + (j-1)] +
				(4.0 * (image[index+1] + image[index+nx] + 
				image[index-1] + image[index-nx])) +
				(-20.0 * image[index])) / 6.0);
		}
	}
	for(j=0;j<nx;j++){
		lap[(ny-1)*nx + j] = 0.0;
		lap[j] = 0.0;
	}
	for(i=0;i<ny;i++){
		lap[(i*nx) + nx-1] = 0.0;
		lap[i*nx] = 0.0;
	}
}


/* 

*/
void cmvu_laplacian(float *image, float *lap, int nx, int ny)
{
	int index, i, j;

	for(i=1;i<ny-1;i++){
		for(j=1;j<nx-1;j++){
			index = (i*nx) +j;
			lap[index] = (float)(((4.0 * image[index]) -
				image[index+1] - image[index+nx] -
				image[index-1] - image[index-nx])
				/ 16.0);

		}
	}
	for(j=0;j<nx;j++){
		lap[(ny-1)*nx + j] = 0.0;
		lap[j] = 0.0;
	}
	for(i=0;i<ny;i++){
		lap[(i*nx) + nx-1] = 0.0;
		lap[i*nx] = 0.0;
	}
}




void horn_dy(float *image1, float *image2, float *dy, int nx, int ny)
{
	int index, i, j;

	for(i=0;i<ny-1;i++){
		for(j=0;j<nx-1;j++){
			index = (i*nx) +j;
			dy[index] = (float)(
				((image1[index+nx] + image2[index+nx] + 
				image1[index+nx+1] + image2[index+nx+1]) / 4.0) -
				((image1[index] + image2[index] + 
				image1[index+1] + image2[index+1]) / 4.0));
		}
	}
	for(j=0;j<nx;j++){
		dy[(ny-1)*nx + j] = 0.0;
	}
	for(i=0;i<ny;i++){
		dy[(i*nx) + (nx-1)] = 0.0;
	}
}

void horn_dx(float *image1, float *image2, float *dx, int nx, int ny)
{
	int index, i, j;

	for(i=0;i<ny-1;i++){
		for(j=0;j<nx-1;j++){
			index = (i*nx) +j;
			dx[index] = (float)(((image1[index+1] + image2[index+1] + 
				image1[index+nx+1] + image2[index+nx+1]) / 4.0) -
				((image1[index] + image2[index] + 
				image1[index+nx] + image2[index+nx]) / 4.0));
		}
	}
	for(j=0;j<nx;j++){
		dx[(ny-1)*nx + j] = 0.0;
	}
	for(i=0;i<ny;i++){
		dx[(i*nx)+(nx-1)] = 0.0;
	}
}


void horn_dt(float *image1, float *image2, float *dt, int nx, int ny)
{
	int index, i, j;

	for(i=0;i<ny-1;i++){
		for(j=0;j<nx-1;j++){
			index = (i*nx) +j;
			dt[index] = (float)(((image2[index] + image2[index+1] + 
				image2[index+nx] + image2[index+nx+1]) / 4.0) -
				((image1[index] + image1[index+1] + 
				image1[index+nx] + image1[index+nx+1]) / 4.0));
		}
	}
	for(j=0;j<nx;j++){
		dt[(ny-1)*nx + j] = 0.0;
	}
	for(i=0;i<ny;i++){
		dt[(i*nx) + (nx-1)] = 0.0;
	}
}

