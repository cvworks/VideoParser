#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "png.h"

int main(int argc, char *argv[])
{
  ppm_t ppm;
  pgm_t pgm;
  FILE *fp;
  pnm_ppm_init(&ppm);
  pnm_pgm_init(&pgm);

  fp = fopen(argv[1],"r");
  if (!fp) {
    fprintf(stderr, "%s: cannot open png-file '%s'.\n",argv[0],argv[1]);
    exit(1);
  }
  pnm_png_verbose = 2;
  {
    int status;
    status = pnm_ppm_readpng(&ppm, fp);
    if (status != PNM_PNG_OK) {
      switch (status) {
      case PNM_PNG_NOPNG:
	fprintf(stderr,"%s: '%s' is no png-file.\n",argv[0],argv[1]);
	fprintf(stderr,"%s: read '%s' as pgm-file.\n",argv[0],argv[1]);
	pgm.pixels = pgm_readpgm(fp, &pgm.cols, &pgm.rows, &pgm.maxval);
	break;
      case PNM_PNG_NOMEM:
	fprintf(stderr,"%s: no ppm-mem.\n",argv[0]);
	exit(1);
      default:
	fprintf(stderr,"%s: error reading '%s'.\n",argv[0],argv[1]);
	exit(1);
      }
    }
  }
  /*ppm_writeppm(stdout, ppm.pixels, ppm.cols, ppm.rows, 255, 1);*/
  if (pgm.pixels)
    pnm_pgm_writepng(stdout, &pgm);
  else
    pnm_ppm_writepng(stdout, &ppm);

  exit(0);
}
