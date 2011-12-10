/*
 * pnmadd/png.c
 *
 * reading and writing ppm_t as png
 *
 * Sven Wachsmuth, 16.7.2003
 */
#include <stdio.h>
#include <stdlib.h>

#include <png.h>

#include "png.h"

int pnm_png_verbose = 0;

int pnm_ppm_readImage(ppm_t *ppm, FILE *fp, const char *prefix)
{
    int status;
    status = pnm_ppm_readpng(ppm, fp);
    if (status != PNM_PNG_OK) {
	switch (status) {
	    case PNM_PNG_NOPNG:
		if (pnm_png_verbose > 0) {
		    fprintf(stderr,"%s: file is no png-file.\n",prefix);
		    fprintf(stderr,"%s: read file as ppm-file.\n",prefix);
		}
		ppm->pixels = ppm_readppm(fp, &ppm->cols, &ppm->rows,
					  &ppm->maxval);
		break;
	    case PNM_PNG_NOMEM:
		fprintf(stderr,"%s: no ppm-mem.\n",prefix);
		exit(1);
	    default:
		fprintf(stderr,"%s: error reading file.\n",prefix);
		exit(1);
	}
    }
    return (status);
}

int pnm_pgm_readImage(pgm_t *pgm, FILE *fp, const char *prefix)
{
    int status;
    status = pnm_pgm_readpng(pgm, fp);
    if (status != PNM_PNG_OK) {
	switch (status) {
	    case PNM_PNG_NOPNG:
		if (pnm_png_verbose > 0) {
		    fprintf(stderr,"%s: file is no png-file.\n",prefix);
		    fprintf(stderr,"%s: read file as pgm-file.\n",prefix);
		}
		pgm->pixels = pgm_readpgm(fp, &pgm->cols, &pgm->rows, 
					  &pgm->maxval);
		break;
	    case PNM_PNG_NOMEM:
		fprintf(stderr,"%s: no ppm-mem.\n",prefix);
		exit(1);
	    default:
		fprintf(stderr,"%s: error reading file.\n",prefix);
		exit(1);
	}
    }
    return (status);
}

int pnm_ppm_readpng(ppm_t *ppm, FILE *fp)
{
  char header[16];
  png_structp png_ptr;
  png_infop   info_ptr, end_info;

  if (!fp) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: no file pointer.\n");
    return (PNM_PNG_ERROR);
  }
  if (!ppm) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: no ppm pointer.\n");
    return (PNM_PNG_NOMEM);
  }
  pnm_ppm_free(ppm);

  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: no png-file.\n");
    fseek(fp, 0, SEEK_SET);
    return (PNM_PNG_NOPNG);
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: cannot initialize png_struct.\n");
    return (PNM_PNG_ERROR);
  }
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: cannot initialize png_info.\n");
    return (PNM_PNG_ERROR);
  }    
  
  end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: cannot initialize end_info.\n");
    return (PNM_PNG_ERROR);
  }    

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: ERROR while reading png!\n");
    return (PNM_PNG_ERROR);
  }
  
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  {
    png_bytep *row_pointers;
    unsigned long int cols, rows, rowbytes;
    int bit_depth, color_type;
    int i, i_row, i_col, i_byte;

    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &cols, &rows, &bit_depth, &color_type,
		 NULL, NULL, NULL);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: convert %d bit depth to 8.\n",
		bit_depth);
      png_set_gray_1_2_4_to_8(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: convert palette to RGB.\n");
      png_set_palette_to_rgb(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY ||
	color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: convert gray to RGB.\n");
      png_set_gray_to_rgb(png_ptr);
    }
    if (bit_depth == 16) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: strip 16bit to 8bit.\n");      
      png_set_strip_16(png_ptr);
    }
    if (color_type & PNG_COLOR_MASK_ALPHA) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: strip alpha channel.\n");      
      png_set_strip_alpha(png_ptr);
    }
    png_read_update_info(png_ptr, info_ptr);

    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    row_pointers = (png_bytep *) malloc(rows * sizeof(png_bytep));
    row_pointers[0] = (png_bytep) malloc(rows * rowbytes * sizeof(png_byte));
    for (i=1; i < rows; i++)
      row_pointers[i] = row_pointers[0] + i * rowbytes;

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, end_info);

    pnm_ppm_alloc(ppm, cols, rows);
    
    for (i_row=0; i_row < rows; i_row++) {
      png_bytep row_ptr = row_pointers[i_row];
      pixel *p = ppm->pixels[i_row];

      for (i_col=0; i_col < cols; i_col++) {
	int red, green, blue;
	
	red = *(row_ptr++);
	green = *(row_ptr++);
	blue = *(row_ptr++);
	/*fprintf(stderr,"pnm_ppm_readpng: set (%d,%d,%d)\n",
	 *	red,green,blue);
	 */
	PPM_ASSIGN(*(p), red, green, blue);
	p++;
      }
    }
    free(row_pointers[0]);
    free(row_pointers);
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  return (PNM_PNG_OK);
}

int pnm_pgm_readpng(pgm_t *pgm, FILE *fp)
{
  char header[16];
  png_structp png_ptr;
  png_infop   info_ptr, end_info;

  if (!fp) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: no file pointer.\n");
    return (PNM_PNG_ERROR);
  }
  if (!pgm) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_pgm_readpng: no pgm pointer.\n");
    return (PNM_PNG_NOMEM);
  }
  pnm_pgm_free(pgm);

  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: no png-file.\n");
    fseek(fp, 0, SEEK_SET);
    return (PNM_PNG_NOPNG);
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: cannot initialize png_struct.\n");
    return (PNM_PNG_ERROR);
  }
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: cannot initialize png_info.\n");
    return (PNM_PNG_ERROR);
  }    
  
  end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: cannot initialize end_info.\n");
    return (PNM_PNG_ERROR);
  }    

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    if (pnm_png_verbose > 0)
      fprintf(stderr, "pnm_ppm_readpng: ERROR while reading png!\n");
    return (PNM_PNG_ERROR);
  }
  
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  {
    png_bytep *row_pointers;
    unsigned long int cols, rows, rowbytes;
    int bit_depth, color_type;
    int i, i_row, i_col, i_byte;

    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &cols, &rows, &bit_depth, &color_type,
		 NULL, NULL, NULL);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: convert %d bit depth to 8.\n",
		bit_depth);
      png_set_gray_1_2_4_to_8(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: convert palette to RGB.\n");
      png_set_palette_to_rgb(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY ||
	color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: convert gray to RGB.\n");
      png_set_gray_to_rgb(png_ptr);
    }
    if (bit_depth == 16) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: strip 16bit to 8bit.\n");      
      png_set_strip_16(png_ptr);
    }
    if (color_type & PNG_COLOR_MASK_ALPHA) {
      if (pnm_png_verbose > 1)
	fprintf(stderr,"pnm_ppm_readpng: strip alpha channel.\n");      
      png_set_strip_alpha(png_ptr);
    }
    png_read_update_info(png_ptr, info_ptr);

    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    row_pointers = (png_bytep *) malloc(rows * sizeof(png_bytep));
    row_pointers[0] = (png_bytep) malloc(rows * rowbytes * sizeof(png_byte));
    for (i=1; i < rows; i++)
      row_pointers[i] = row_pointers[0] + i * rowbytes;

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, end_info);

    pnm_pgm_alloc(pgm, cols, rows);
    
    for (i_row=0; i_row < rows; i_row++) {
      png_bytep row_ptr = row_pointers[i_row];
      gray *p = pgm->pixels[i_row];

      for (i_col=0; i_col < cols; i_col++) {
	int red, green, blue;
	
	red = *(row_ptr++);
	green = *(row_ptr++);
	blue = *(row_ptr++);
	/*fprintf(stderr,"pnm_ppm_readpng: set (%d,%d,%d)\n",
	 *	red,green,blue);
	 */
	*(p) = (red + green + blue) / 3;
	p++;
      }
    }
    free(row_pointers[0]);
    free(row_pointers);
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  return (PNM_PNG_OK);
}

int pnm_ppm_writepng(FILE *fp, ppm_t *ppm)
{
  png_infop info_ptr;
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return (PNM_PNG_ERROR);
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr,
			     (png_infopp)NULL);
    return (PNM_PNG_ERROR);
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return (PNM_PNG_ERROR);
  }
  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, ppm->cols, ppm->rows,
	       /*bit_depth*/ 8, 
	       /*color_type*/ PNG_COLOR_TYPE_RGB, 
	       /*interlace_type*/ PNG_INTERLACE_NONE,
	       /*compression_type*/ PNG_COMPRESSION_TYPE_DEFAULT, 
	       /*filter_method*/ PNG_FILTER_TYPE_DEFAULT);

  /* png_set_PLTE(png_ptr, info_ptr, palette, num_palette); */
  /*palette        - the palette for the file
   *                 (array of png_color)
   *num_palette    - number of entries in the palette
   */

  /*png_set_hIST(png_ptr, info_ptr, hist);
   *(PNG_INFO_hIST)
   * hist           - histogram of palette (array of
   *                         png_uint_16)
   */
  
  png_write_info(png_ptr, info_ptr);
  /*png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);*/
  png_set_packing(png_ptr);
  /*{
    png_color_8 sig_bit;
    int true_bit_depth = 10;
    png_byte color_type = PNG_COLOR_TYPE_RGB;

    // Set the true bit depth of the image data 
    if (color_type & PNG_COLOR_MASK_COLOR)
      {
	sig_bit.red = true_bit_depth;
	sig_bit.green = true_bit_depth;
	sig_bit.blue = true_bit_depth;
      }
    else
      {
	sig_bit.gray = true_bit_depth;
      }
    if (color_type & PNG_COLOR_MASK_ALPHA)
      {
	sig_bit.alpha = true_bit_depth;
      }
    //png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_set_shift(png_ptr, &sig_bit);
  }*/
  /*png_set_invert_mono(png_ptr);*/
  {
    int i,j,k;
    png_byte **row_pointers  = malloc(ppm->rows * sizeof(png_byte *));
    row_pointers[0] = malloc(ppm->cols*3 * ppm->rows * sizeof(png_byte));
    for (k=0,i=0; i < ppm->rows; i++) {
      row_pointers[i] = &row_pointers[0][k];
      for (j=0; j < ppm->cols; j++) {
	pixval r = PPM_GETR(ppm->pixels[i][j]);
	pixval g = PPM_GETG(ppm->pixels[i][j]);
	pixval b = PPM_GETB(ppm->pixels[i][j]);
	
	row_pointers[0][k++] = r;
	row_pointers[0][k++] = g;
	row_pointers[0][k++] = b;
      }
    }
    png_write_image(png_ptr, row_pointers);
    
    free(row_pointers[0]);
    free(row_pointers);
  }
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  return (0);
}

int pnm_pgm_writepng(FILE *fp, pgm_t *pgm)
{
  png_infop info_ptr;
  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return (PNM_PNG_ERROR);
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr,
			     (png_infopp)NULL);
    return (PNM_PNG_ERROR);
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return (PNM_PNG_ERROR);
  }
  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, pgm->cols, pgm->rows,
	       /*bit_depth*/ 8, 
	       /*color_type*/ PNG_COLOR_TYPE_GRAY, 
	       /*interlace_type*/ PNG_INTERLACE_NONE,
	       /*compression_type*/ PNG_COMPRESSION_TYPE_DEFAULT, 
	       /*filter_method*/ PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr, info_ptr);
  png_set_packing(png_ptr);
  {
    int i,j,k;
    png_byte **row_pointers  = malloc(pgm->rows * sizeof(png_byte *));
    row_pointers[0] = malloc(pgm->cols * pgm->rows * sizeof(png_byte));
    for (k=0,i=0; i < pgm->rows; i++) {
      row_pointers[i] = &row_pointers[0][k];
      for (j=0; j < pgm->cols; j++) {
	pixval g = pgm->pixels[i][j];
	row_pointers[0][k++] = g;
      }
    }
    png_write_image(png_ptr, row_pointers);
    
    free(row_pointers[0]);
    free(row_pointers);
  }
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  return (0);
}

