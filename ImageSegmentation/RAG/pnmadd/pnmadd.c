#include "pnmadd.h"

ppm_t *pnm_ppm_create(void)
{
  return (pnm_ppm_init(NULL));
}

ppm_t *pnm_ppm_init(ppm_t *ppm)
{
  if (!ppm)
    ppm = (ppm_t *) malloc(sizeof(ppm_t));
  ppm->rows = 0;
  ppm->cols = 0;
  ppm->maxval = 0;
  ppm->pixels = NULL;
  return (ppm);
}

ppm_t *pnm_ppm_alloc(ppm_t *ppm, int cols, int rows)
{
  if (!ppm)
    ppm = pnm_ppm_create();
  ppm->rows = rows;
  ppm->cols = cols;
  ppm->pixels = ppm_allocarray(cols, rows);
  return (ppm);
}

void pnm_ppm_free(ppm_t *ppm)
{
  if (!ppm) return;
  if (ppm->pixels)
    ppm_freearray(ppm->pixels, ppm->rows);
  pnm_ppm_init(ppm);
}

void pnm_ppm_destroy(ppm_t *ppm)
{
  if (!ppm) return;
  pnm_ppm_free(ppm);
  free(ppm);
}

pixel *pnm_ppm_scalerow(pixel *copy, pixel *row, int cols2, int factor)
{
  int count,j,j2;
  for (count=factor, j2=0, j=0; j2 < cols2; j++) {
    pixel pix;
    if (count < 0) { count++; continue; }
    copy[j2++] = pix = row[j];
    while (count > 0) { copy[j2++] = pix; count--; }
    count = factor;
  }
  return (copy);
}

ppm_t *pnm_ppm_scale(ppm_t *copy, ppm_t *ppm, int factor)
{
  int i,i2,count;
  if (!ppm) return (copy);

  copy = pnm_ppm_alloc(copy, 
		       ((factor > 0) ? 
			ppm->cols * (factor+1) : 
			ppm->cols / (-factor+1)),
		       ((factor > 0) ?
			ppm->rows * (factor+1) :
			ppm->rows / (-factor+1)));
  for (count=factor, i2=0, i=0; i2 < copy->rows; i++) {
    pixel *row2, *row = ppm->pixels[i];
    if (count < 0) { count++; continue; }

    row2 = pnm_ppm_scalerow(copy->pixels[i2++], row, copy->cols, factor);
    while (count > 0) {
      memcpy(copy->pixels[i2++], row2, copy->cols * sizeof(pixel));
      count--;
    }
    count = factor;
  }
  return (copy);
}

ppm_t *pnm_ppm_clear(ppm_t *ppm, pixel value)
{
  unsigned int i;
  int cols, rows;
  pixel *row;
  if (!ppm || ppm->cols == 0 || ppm->rows == 0) return (ppm);
  
  cols = ppm->cols;
  rows = ppm->rows;

  /** first assign first row ... */
  row = ppm->pixels[0];
  /** ... assign first pixel ... */
  row[0] = value;
  /** ... double assigned pixels until doubling exceeds cols ... */
  for (i=1; (i<<1) <= cols; i<<=1)
    memcpy(row+i,row, i*sizeof(pixel));
  /** ... if cols is not potential of two, assign remaining pixels */
  if (i < cols)
    memcpy(row+i,row, (cols-i) * sizeof(pixel));

  /** assign remaining rows */
  for (i=1; i < rows; i++)
    memcpy(ppm->pixels[i],row,cols * sizeof(pixel));

  return (ppm);
}

pgm_t *pnm_pgm_create(void)
{
  return (pnm_pgm_init(NULL));
}

pgm_t *pnm_pgm_init(pgm_t *pgm)
{
  if (!pgm)
    pgm = (pgm_t *) malloc(sizeof(pgm_t));
  pgm->rows = 0;
  pgm->cols = 0;
  pgm->maxval = 0;
  pgm->pixels = NULL;
  return (pgm);
}

pgm_t *pnm_pgm_alloc(pgm_t *pgm, int cols, int rows)
{
  if (!pgm)
    pgm = pnm_pgm_create();
  pgm->rows = rows;
  pgm->cols = cols;
  pgm->pixels = pgm_allocarray(cols, rows);
  return (pgm);
}

void pnm_pgm_free(pgm_t *pgm)
{
  if (!pgm) return;
  if (pgm->pixels)
    pgm_freearray(pgm->pixels, pgm->rows);
  pnm_pgm_init(pgm);
}

void pnm_pgm_destroy(pgm_t *pgm)
{
  if (!pgm) return;
  pnm_pgm_free(pgm);
  free(pgm);
}

gray *pnm_pgm_scalerow(gray *copy, gray *row, int cols2, int factor)
{
  int count,j,j2;
  for (count=factor, j2=0, j=0; j2 < cols2; j++) {
    gray pix;
    if (count < 0) { count++; continue; }
    copy[j2++] = pix = row[j];
    while (count > 0) { copy[j2++] = pix; count--; }
    count = factor;
  }
  return (copy);
}

pgm_t *pnm_pgm_scale(pgm_t *copy, pgm_t *pgm, int factor)
{
  int i,i2,count;
  if (!pgm) return (copy);

  copy = pnm_pgm_alloc(copy, 
		       ((factor > 0) ? 
			pgm->cols * (factor+1) : 
			pgm->cols / (-factor+1)),
		       ((factor > 0) ?
			pgm->rows * (factor+1) :
			pgm->rows / (-factor+1)));
  for (count=factor, i2=0, i=0; i2 < copy->rows; i++) {
    gray *row2, *row = pgm->pixels[i];
    if (count < 0) { count++; continue; }

    row2 = pnm_pgm_scalerow(copy->pixels[i2++], row, copy->cols, factor);
    while (count > 0) {
      memcpy(copy->pixels[i2++], row2, copy->cols * sizeof(pixel));
      count--;
    }
    count = factor;
  }
  return (copy);
}

pgm_t *pnm_pgm_clear(pgm_t *pgm, gray value)
{
  unsigned int i;
  int cols, rows;
  gray *row;
  if (!pgm || pgm->cols == 0 || pgm->rows == 0) return (pgm);
  
  cols = pgm->cols;
  rows = pgm->rows;

  /** first assign first row ... */
  row = pgm->pixels[0];
  /** ... assign first pixel ... */
  row[0] = value;
  /** ... double assigned pixels until doubling exceeds cols ... */
  for (i=1; (i<<1) <= cols; i<<=1)
    memcpy(row+i,row, i*sizeof(gray));
  /** ... if cols is not potential of two, assign remaining pixels */
  if (i < cols)
    memcpy(row+i,row, (cols-i) * sizeof(gray));

  /** assign remaining rows */
  for (i=1; i < rows; i++)
    memcpy(pgm->pixels[i],row,cols * sizeof(gray));

  return (pgm);
}

int *pnm_int_scalerow(int *copy, int *row, int copy_cols, int factor)
{
  int count,j;
  int *copy_p = copy;
  for (count=factor, j=0; j < copy_cols; row++) {
    int pix;
    if (count < 0) { count++; continue; }
    *(copy_p++) = pix = *row; j++;
    while (count > 0) { *(copy_p++) = pix; j++; count--; }
    count = factor;
  }
  return (copy);
}

int *pnm_int_scale(int *copy, int *data, int cols, int rows, int factor)
{
  int i,count;
  int copy_cols, copy_rows;
  int *copy_row, *row;

  if (!data) return (copy);
  copy_cols = ((factor > 0) ? cols * (factor+1) : cols / (-factor+1));
  copy_rows = ((factor > 0) ? rows * (factor+1) : rows / (-factor+1));

  if (!copy)
    copy = (int *) malloc(copy_cols * copy_rows * sizeof(int));
  
  copy_row = copy;
  for (row=data, count=factor, i=0; i < copy_rows; row+=cols) {
    int *row2;
    if (count < 0) { count++; continue; }

    row2 = pnm_int_scalerow(copy_row, row, copy_cols, factor);
    copy_row += copy_cols; i++;

    while (count > 0) {
      memcpy(copy_row, row2, copy_cols * sizeof(int));
      count--; copy_row += copy_cols; i++;
    }
    count = factor;
  }
  return (copy);
}

unsigned char *pnm_uchar_scalerow(unsigned char *copy, unsigned char *row, 
				  int copy_cols, int factor)
{
  int count,j;
  unsigned char *copy_p = copy;
  for (count=factor, j=0; j < copy_cols; row++) {
    int pix;
    if (count < 0) { count++; continue; }
    *(copy_p++) = pix = *row; j++;
    while (count > 0) { *(copy_p++) = pix; j++; count--; }
    count = factor;
  }
  return (copy);
}

unsigned char *pnm_uchar_scale(unsigned char *copy, unsigned char *data, 
			       int cols, int rows, int factor)
{
  int i,count;
  int copy_cols, copy_rows;
  unsigned char *copy_row, *row;

  if (!data) return (copy);
  copy_cols = ((factor > 0) ? cols * (factor+1) : cols / (-factor+1));
  copy_rows = ((factor > 0) ? rows * (factor+1) : rows / (-factor+1));

  if (!copy)
    copy = (unsigned char *) malloc(copy_cols * copy_rows * sizeof(unsigned char));
  
  copy_row = copy;
  for (row=data, count=factor, i=0; i < copy_rows; row+=cols) {
    unsigned char *row2;
    if (count < 0) { count++; continue; }

    row2 = pnm_uchar_scalerow(copy_row, row, copy_cols, factor);
    copy_row += copy_cols; i++;

    while (count > 0) {
      memcpy(copy_row, row2, copy_cols * sizeof(unsigned char));
      count--; copy_row += copy_cols; i++;
    }
    count = factor;
  }
  return (copy);
}
    
pnm_image_t *pnm_image_create(void)
{
  return (pnm_image_init(NULL));
}

pnm_image_t *pnm_image_init(pnm_image_t *image)
{
  if (!image)
    image = (pnm_image_t *) malloc(sizeof(pnm_image_t));
  image->cols = 0;
  image->rows = 0;
  image->channels = 0;
  image->maxval = 0;
  image->pixels = NULL;
  
  return (image);
}

pnm_image_t *pnm_image_set(pnm_image_t *image,
			   int cols, int rows, int channels,
			   pnm_pixval maxval, pnm_pixval ***pixels)
{
  if (!image)
    image = (pnm_image_t *) malloc(sizeof(pnm_image_t));
  image->cols = cols;
  image->rows = rows;
  image->channels = channels;
  image->maxval = maxval;
  image->pixels = pixels;
  
  return (image);
}

pnm_image_t *pnm_image_alloc(pnm_image_t *image, 
			     int channels, int cols, int rows)
{
  int i,j,k;
  if (!image)
    image = pnm_image_create();

  /** channels */
  image->pixels = (pnm_pixval ***) 
    malloc((image->channels=channels) * sizeof(pnm_pixval **));
  
  /** rows */
  image->pixels[0] = (pnm_pixval **) 
    malloc((image->rows=rows) * channels * sizeof(pnm_pixval *));
  for (j=rows, i=1; i < channels; i++, j+=rows)
    image->pixels[i] = image->pixels[0]+j;
  
  /** columns */
  image->pixels[0][0] = (pnm_pixval *)
    malloc((image->cols=cols) * rows * channels * sizeof(pnm_pixval));
  for (k=0, i=0; i < channels; i++)
    for (j=0; j < rows; j++, k+=cols)
      image->pixels[i][j] = image->pixels[0][0]+k;

  image->maxval=0;

  return(image);
}

void pnm_image_free(pnm_image_t *image)
{
  if (!image) return;
  if (image->pixels) {
    if (image->pixels[0]) {
      if (image->pixels[0][0]) 
	free(image->pixels[0][0]);
      free(image->pixels[0]);
    }
    free(image->pixels);
  }
  pnm_image_init(image);
}

void pnm_image_destroy(pnm_image_t *image)
{
  if (!image) return;
  pnm_image_free(image);
  free(image);
}

int pnm_image_pgmWrite(FILE *fp, pnm_image_t *image)
{
  int i,j;
  gray *row;

  if (!image || !fp) return (-1);

  row  = pgm_allocrow(image->cols);
  for (i=0; i < image->channels; i++) {
    pgm_writepgminit(fp, image->cols, image->rows, image->maxval, 
		     1 /* plain PGM format file, no raw */);
    for (j=0; j < image->rows; j++) {
      int k;
      for (k=0; k < image->cols; k++)
	row[k] = (gray) image->pixels[i][j][k];
      pgm_writepgmrow(fp, row, image->cols, image->maxval, 
		      1 /* plain PGM format file, no raw */);
    }
  }
  pgm_freerow(row);

  return (0);
}

pnm_image_t *pnm_image_pgmRead(pnm_image_t *image, FILE *fp)
{
  gray **pixels;
  int  cols, rows;
  gray maxval;
  int i,j;

  pixels = pgm_readpgm(fp, &cols, &rows, &maxval);

  image = pnm_image_alloc(image, 1, cols, rows);
  image->maxval = maxval;
  for (i=0; i < rows; i++) 
    for (j=0; j < cols; j++) 
      image->pixels[0][i][j] = (pnm_pixval) pixels[i][j];
  
  return (image);
}

pnm_image_t *pnm_ppm2image(pnm_image_t *image, ppm_t *ppm)
{
  int i,j,k;
  if (!ppm) return (image);
  if (!image)
    image = pnm_image_alloc(NULL, 3, ppm->cols, ppm->rows);

  for (j=0; j < image->rows; j++)
    for (k=0; k < image->cols; k++) {
      image->pixels[0][j][k] = (pnm_pixval) PPM_GETR(ppm->pixels[j][k]);
      image->pixels[1][j][k] = (pnm_pixval) PPM_GETG(ppm->pixels[j][k]);
      image->pixels[2][j][k] = (pnm_pixval) PPM_GETB(ppm->pixels[j][k]);  
    }
  image->maxval = ppm->maxval;
  return (image);
}

ppm_t *pnm_image2ppm(ppm_t *ppm, pnm_image_t *image)
{
  int i,j,k;
  if (!image) return (ppm);
  if (!ppm) {
    ppm = malloc(sizeof(ppm_t));
    ppm_allocarray(ppm->cols=image->cols,
		   ppm->rows=image->rows);
  }

  for (j=0; j < ppm->rows; j++)
    for (k=0; k < ppm->cols; k++)
      PPM_ASSIGN(ppm->pixels[j][k], 
		 (gray) image->pixels[0][j][k],
		 (gray) image->pixels[1][j][k],
		 (gray) image->pixels[2][j][k]);
  ppm->maxval = (gray) image->maxval;
  return (ppm);
}
