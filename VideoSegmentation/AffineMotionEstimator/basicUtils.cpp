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
Copyright 1993, 2002, Michael J. Black

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
#include "basicUtils.h"

#ifndef MAXLEN
#define MAXLEN 256
#endif

void error(char *s1, char *s2)
{
	fprintf(stderr, "%s %s", s1, s2);
	exit(1);
}

float linear_interp(float low_value, float high_value, float position)
{
	float result;

	result = ((low_value * (1 - position))
		+ (high_value * position));

	return(result);
}


void pad_string(char *fnCurr, int pad, int im)
{
	if (pad > 0) {
		if (im==0)
			im++;
		if (im < pad)
			strcat(fnCurr, "0");
		if (im < pad/10)
			strcat(fnCurr, "0");
		if (im < pad/100)
			strcat(fnCurr, "0");
		if (im < pad/1000)
			strcat(fnCurr, "0");
	}
}

void read_image_float(float *image, char *fnIn, int nx, int ny)
{
	int sizeInput;
	FILE *infile;

	infile = fopen(fnIn, "rb");
	if (infile == NULL) {
		fprintf(stderr, "infile: NULL\n");
	}

	sizeInput = nx * ny * sizeof( float );
	fread((float *) image, sizeInput, 1, infile);

	fclose(infile);
}


void read_image_strip_bytes(float *image, char *fnIn, 
							int nx, int ny, int bytes)
{
	unsigned char *temp, *c;
	FILE *infile;
	int index, i, sizeInput;

	infile = fopen(fnIn, "rb");
	if (infile == NULL) {
		fprintf(stderr, "infile: %s NULL\n", fnIn);
	}

	sizeInput = bytes * sizeof( char );
	if ((c = (unsigned char *) malloc((size_t) sizeInput)) == NULL)
		error(" Unable to allocate memory for ", "image");
	fread((char *) c, sizeInput, 1, infile);

	sizeInput = nx * ny * sizeof( char );
	if ((temp = (unsigned char *) malloc((size_t) sizeInput)) == NULL)
		error(" Unable to allocate memory for ", "image");
	fread((char *) temp, sizeInput, 1, infile);

	i=nx*ny;
	index=0;
	while(i--) {
		image[index] = (float) temp[index];
		index++;
	}

	free(temp);
	free(c);
	fclose(infile);
}

/*
copy image from to image to.
*/

void copy_image(float *to, float *from, int nx, int ny)
{
	int i=nx*ny;
	while(i--){
		*to++ = *from++;
	}
}

/*
intialize image to 1.0
*/
void ones_image(float *image, int nx, int ny)
{
	int i=nx*ny;
	while(i--)
		*image++ = 1.0;
}

/*
intialize image to 0.0
*/
void zero_image(float *image, int nx, int ny)
{
	int i=nx*ny;
	while(i--)
		*image++ = 0.0;
}

void invert_image(float *image, int nx, int ny)
{
	int index, i, j;

	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			index = i*nx + j;
			image[index] = abs(image[index] - 1);
		}
	}
}


void average_images(float *image1, float *image2, float *mean, int nx, int ny)
{
	int index, i, j;

	for(i=0;i<ny;i++){
		for(j=0;j<nx;j++){
			index = i*nx + j;
			mean[index] = (image1[index] + image2[index]) / 2.0f;
		}
	}
}


/* 
add "add" to "image" 
*/
void add_image(float *image, float *add, int nx, int ny)
{
	int i=nx*ny;
	while(i--)
		*image++ += *add++;
}

void subtract_image(float *image, float *sub, int nx, int ny)
{
	int i=nx*ny;
	while(i--)
		*image++ -= *sub++;
}


void abs_image(float *image, int nx, int ny)
{
	int i=nx*ny, index=0;
	while(i--) {
		image[index] = abs(image[index]);
		index++;
	}
}

/* 
divide image by a float
*/
void divide_image(float *image, float divisor, int nx, int ny)
{
	int i=nx*ny, index=0;
	while(i--) {
		image[index] = image[index] / divisor;
		index++;
	}
}


void mul_images(float *image1, float *image2, int nx, int ny)
{
	int i=nx*ny, index=0;
	while(i--) {
		image1[index] = image1[index] * image2[index];
		index++;
	}
}

float max_image_val(float *image, int nx, int ny)
{
	int i=nx*ny;
	float maxval;

	maxval=*image++;
	while(i--)
		if (*image>maxval)
			maxval = *image++;
	return(maxval);
}

float min_image_val(float *image, int nx, int ny)
{
	int i=nx*ny;
	float minval;

	minval=*image;
	while(i--)
		if (*image<minval)
			minval = *image++;
	return(minval);
}


float image_abs_max(float *image, int nx, int ny)
{
	float result;
	int i=nx*ny;

	result = abs(*image++);
	while(i--)
		result = max(result,abs(*image++));
	return(result);
}


float image_max(float *image, int nx, int ny)
{
	float result;
	int i=nx*ny;

	result = *image++;
	while(i--)
		result = max(result,*image++);
	return(result);
}


float image_min(float *image, int nx, int ny)
{
	float result;
	int i=nx*ny;

	result = *image++;
	while(i--)
		result = min(result,*image++);
	return(result);
}


void save_pgm(char *path, float *image, int nx, int ny)
{
	char dataOut[MAXLEN];
	int index, i, sizeOutput;
	float min_val, max_val, scale, val;
	char pgmheadstr[2*MAXLEN];
	unsigned char *quant;
	FILE *outfile;
	int headLen;

	strcpy(dataOut, "");
	index=0;
	min_val = image[index];
	max_val = min_val;
	i=nx*ny;
	while(i--){
		min_val = min(min_val, image[index]);
		max_val = max(max_val, image[index]);
		index++;
	}

	if (min_val == max_val)
		scale = 1.0;
	else
		scale = 255.0f / (max_val - min_val);

	strcat(dataOut, path);
	outfile = fopen(dataOut, "wb");
	if (outfile == NULL) {
		fprintf(stderr, "outfile: NULL\n");
	}

	sizeOutput = nx * ny * sizeof( char );
	sprintf(pgmheadstr,"P5\n%d %d\n255\n",nx,ny);
	headLen=strlen(pgmheadstr);
	sizeOutput+=headLen;
	if ((quant = (unsigned char *) malloc((size_t) sizeOutput)) == NULL)
		error(" Unable to allocate memory for (in save_image)", "quant");
	strncpy((char*)quant,pgmheadstr,headLen);

	/*  fprintf(stderr, "quantizing\n");  */\
		i=nx*ny;
	index=0;
	while(i--){
		val = (image[index] - min_val) * scale;
		quant[index+headLen] = min(max(ROUND(val),0),255);
		index++;
	}

	/*  fprintf(stderr, "writing\n");  */
	fwrite((char *) quant, sizeOutput, 1, outfile);

	/*  fprintf(stderr, "done\n");  */
	free(quant);
	fclose(outfile);
}


void save_pgm_no_scale(char *path, float *image, int nx, int ny)
{
	char dataOut[MAXLEN];
	int index, i, sizeOutput;
	char pgmheadstr[2*MAXLEN];
	unsigned char *quant;
	FILE *outfile;
	int headLen;

	strcpy(dataOut, "");

	strcat(dataOut, path);
	outfile = fopen(dataOut, "wb");
	if (outfile == NULL) {
		fprintf(stderr, "outfile: NULL\n");
	}
	sizeOutput = nx * ny * sizeof( char );
	sprintf(pgmheadstr,"P5\n%d %d\n255\n",nx,ny);
	headLen=strlen(pgmheadstr);
	sizeOutput+=headLen;
	if ((quant = (unsigned char *) malloc((size_t) sizeOutput)) == NULL)
		error(" Unable to allocate memory for (in save_image)", "quant");
	strncpy((char*)quant,pgmheadstr,headLen);

	/*  fprintf(stderr, "quantizing\n");  */
	i=nx*ny;
	index=0;
	while(i--) {
		quant[index+headLen] = min(max(ROUND(image[index]),0),255);
		index++;
	}

	/*  fprintf(stderr, "writing\n");  */
	fwrite((char *) quant, sizeOutput, 1, outfile);

	/*  fprintf(stderr, "done\n");  */
	free(quant);
	fclose(outfile);
}

